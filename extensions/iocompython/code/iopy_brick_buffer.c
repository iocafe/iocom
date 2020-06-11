/**

  @file    iopy_brick_buffer.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    16.4.2020


  Example to get video data from remote camera:

    # Module: receive-camera-data.py
    # Pull data from IO device's (gina1) camera trough a server (frank, etc)
    # This example logs into "iocafenet" device network run by server in local computer.
    # User name "ispy" and password "pass" identify the client to server.
    # The camtest must be accepted as valid at server (this can be done with i-spy)
    # Client verifies validity of the server by acceptable certificate bundle 'myhome-bundle.crt'.

    from iocompython import Root, Connection, MemoryBlock, BrickBuffer
    import ioterminal
    import time

    # 9000 = select device number automatically
    my_device_nr = 9000

    def main():
        root = Root('camtest', device_nr=my_device_nr, security='certchainfile=myhome-bundle.crt')
        ioterminal.start(root)

        Connection(root, "127.0.0.1", "tls,down,dynamic", user='ispy.iocafenet', password='pass')
        camera_buffer = BrickBuffer(root, "exp.gina1.iocafenet", "imp.gina1.iocafenet", "rec_",timeout=-1)
        camera_buffer.set_receive(True);

        while (ioterminal.run(root)):
            data = camera_buffer.get()
            if data != None:
                print(data)

            time.sleep(0.01)

        root.delete()


    if (__name__ == '__main__'):
        main()


  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocompython.h"

#if IOC_USE_JPEG_COMPRESSION
#include "eosal_jpeg.h"
#endif

/* Working state structure used to read data.
 */
typedef struct
{
    iocBrickBuffer *brick_buffer;
    osalTypeId type_id;
    os_int max_values, nro_values;
    os_boolean no_state_bits;
}
BrickBufferGetState;


/* Forward referred static functions.
 */
static void bb_init_signal(
    iocSignal *sig,
    iocHandle *handle);


/**
****************************************************************************************************

  @brief Constructor.

  The BrickBuffer_new function starts running the brick_buffer. Running brick_buffer will keep
  on running until the brick_buffer is deleted. It will attempt repeatedly to connect socket,
  etc transport to other IOCOM device and will move the data when transport is there.

  Note: Application must not delete and create new brick_buffer to reestablish the transport.
  This is handled by the running brick_buffer object.

  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *BrickBuffer_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    BrickBuffer *self;
    PyObject *pyroot = NULL;
    Root *root;

    iocRoot *iocroot;
    iocStreamerSignals sig;

    const char
        *exp = NULL,
        *imp = NULL,
        *prefix = "rec_",
        *flags = osal_str_empty;

    int
        timeout_ms = 0; /* 0 = use default timeout */

    static char *kwlist[] = {
        "root",
        "exp",
        "imp",
        "prefix",
        "timeout",
        "flags",
        NULL
    };

    self = (BrickBuffer *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    os_memclear(&self->brick_buffer, sizeof(iocBrickBuffer));
    os_memclear(&self->h_exp, sizeof(iocHandle));
    os_memclear(&self->h_imp, sizeof(iocHandle));

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oss|sis",
         kwlist, &pyroot, &exp, &imp, &prefix, &timeout_ms, &flags))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        goto failed;
    }

    if (!PyObject_IsInstance(pyroot, (PyObject *)&RootType))
    {
        PyErr_SetString(iocomError, "Root argument is invalid");
        goto failed;
    }

    root = (Root*)pyroot;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        goto failed;
    }

    self->is_device = OS_FALSE;
    if (os_strstr(flags, "device", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        self->is_device = OS_TRUE;
    }
    self->from_device = OS_TRUE;
    if (os_strstr(flags, "tod", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        self->from_device = OS_FALSE;
    }

    self->flat_buffer = OS_TRUE;
    if (os_strstr(flags, "ring", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        self->flat_buffer = OS_FALSE;
    }

    bb_init_signal(&self->sig_cmd, &self->h_imp);
    bb_init_signal(&self->sig_select, &self->h_imp);
    bb_init_signal(&self->sig_state, &self->h_exp);

    if (self->from_device) {
        bb_init_signal(&self->sig_buf, &self->h_exp);
        bb_init_signal(&self->sig_head, &self->h_exp);
        bb_init_signal(&self->sig_tail, &self->h_imp);
    }
    else {
        bb_init_signal(&self->sig_buf, &self->h_imp);
        bb_init_signal(&self->sig_head, &self->h_imp);
        bb_init_signal(&self->sig_tail, &self->h_exp);
    }

    ioc_iopath_to_identifiers(iocroot, &self->exp_ids, exp, IOC_EXPECT_MEMORY_BLOCK);
    ioc_iopath_to_identifiers(iocroot, &self->imp_ids, imp, IOC_EXPECT_MEMORY_BLOCK);

    os_strncpy(self->prefix, prefix, IOPY_BB_PREFIX_SZ);

    /* Initialize brick buffer (does not allocate any memory yet)
    */
    os_memclear(&sig, sizeof(iocStreamerSignals));
    sig.to_device = !self->from_device;
    sig.flat_buffer = self->flat_buffer;
    sig.cmd = &self->sig_cmd;
    sig.select = &self->sig_select;
    sig.state = &self->sig_state;
    sig.buf = &self->sig_buf;
    sig.head = &self->sig_head;
    sig.tail = &self->sig_tail;

    ioc_initialize_brick_buffer(&self->brick_buffer, &sig, iocroot, timeout_ms,
        self->is_device ? IOC_BRICK_DEVICE : IOC_BRICK_CONTROLLER);

    self->status = OSAL_SUCCESS;

#if IOPYTHON_TRACE
    PySys_WriteStdout("BrickBuffer.new(%s,%s)\n", exp, imp);
#endif

    /* Save root pointer and increment reference count.
     */
    self->pyroot = root;
    Py_INCREF(root);
    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}


/**
****************************************************************************************************

  @brief Destructor.

  The BrickBuffer_dealloc function releases the associated Python object and IOCOM brick_buffer.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_dealloc(
    BrickBuffer *self)
{
    if (self->pyroot)
    {
        ioc_release_brick_buffer(&self->brick_buffer);

        if (self->h_exp.mblk) {
            ioc_release_handle(&self->h_exp);
        }

        if (self->h_imp.mblk) {
            ioc_release_handle(&self->h_imp);
        }
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("BrickBuffer.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Delete an IOCOM brick_buffer.

  The BrickBuffer_delete function closes the brick_buffer and releases any ressources for it.
  The brick_buffer must be explisitly closed by calling .delete() function, or by calling
  .delete() on the root object. But not both.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static PyObject *BrickBuffer_delete(
    BrickBuffer *self)
{
    if (self->pyroot)
    {
        ioc_release_brick_buffer(&self->brick_buffer);

        if (self->h_exp.mblk) {
            ioc_release_handle(&self->h_exp);
        }

        if (self->h_imp.mblk) {
            ioc_release_handle(&self->h_imp);
        }
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("BrickBuffer.delete()\n");
#endif
    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/**
****************************************************************************************************

  Initialize unused signal structure.

  @param   sig Pointer to signal structure to initialize.
  @param   handle Pointer to memory block handle structure. Handle doesn't have to be initialized,
           just pointer is saved within the signal structure.
  @return  None.

****************************************************************************************************
*/
static void bb_init_signal(
    iocSignal *sig,
    iocHandle *handle)
{
    os_memclear(sig, sizeof(iocSignal));
    sig->handle = handle;
}


/**
****************************************************************************************************

  Set memory block handle for a signal.

  Lock must be on

  @param   sig Pointer to signal structure to set up.
  @param   handle Pointer to memory block handle structure. Handle doesn't have to be initialized,
           just pointer is saved within the signal structure.
  @return  None.

****************************************************************************************************
*/
static osalStatus bb_try_signal_setup(
    iocSignal *sig,
    os_char *name,
    os_char *prefix,
    iocIdentifiers *mblk_identifiers,
    iocRoot *iocroot)
{
    iocIdentifiers identifiers;

    os_memcpy(&identifiers, mblk_identifiers, sizeof(iocIdentifiers));
    os_strncpy(identifiers.signal_name, prefix, IOC_SIGNAL_NAME_SZ);
    os_strncat(identifiers.signal_name, name, IOC_SIGNAL_NAME_SZ);

    return ioc_setup_signal_by_identifiers(iocroot, &identifiers, sig)
        ? OSAL_SUCCESS : OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  Initialize all signal structires needed.

  Lock must be on

  @param   sig Pointer to signal structure to initialize.
  @param   sig Pointer to memory block handle structure. Handle doesn't have to be initialized,
           just pointer is saved within the signal structure.
  @return  None.

****************************************************************************************************
*/
static osalStatus bb_try_setup(
    BrickBuffer *self,
    iocRoot *iocroot)
{
    /* If setup is already good. We check head and cmd because they are in different
       memory blocks and last to be set up.
     */
    if (self->sig_head.handle->mblk && self->sig_head.flags &&
        self->sig_cmd.handle->mblk &&(self->sig_cmd.flags))
    {
        return OSAL_SUCCESS;
    }
    self->sig_head.flags = 0;
    self->sig_tail.flags = 0;

    if (!self->flat_buffer) {
        if (bb_try_signal_setup(&self->sig_select, "select", self->prefix, &self->imp_ids, iocroot))
            goto getout;
    }
    if (bb_try_signal_setup(&self->sig_state, "state", self->prefix, &self->exp_ids, iocroot)) goto getout;

    if (self->from_device) {
        if (bb_try_signal_setup(&self->sig_buf, "buf", self->prefix, &self->exp_ids, iocroot)) goto getout;
        if (bb_try_signal_setup(&self->sig_head, "head", self->prefix, &self->exp_ids, iocroot)) goto getout;
        if (!self->flat_buffer) {
            if (bb_try_signal_setup(&self->sig_tail, "tail", self->prefix, &self->imp_ids, iocroot))
                goto getout;
        }
    }
    else {
        if (bb_try_signal_setup(&self->sig_buf, "buf", self->prefix, &self->imp_ids, iocroot)) goto getout;
        if (bb_try_signal_setup(&self->sig_head, "head", self->prefix, &self->imp_ids, iocroot)) goto getout;
        if (!self->flat_buffer) {
            if (bb_try_signal_setup(&self->sig_tail, "tail", self->prefix, &self->exp_ids, iocroot))
                goto getout;
        }
    }

    if (bb_try_signal_setup(&self->sig_cmd, "cmd", self->prefix, &self->imp_ids, iocroot)) goto getout;

    return OSAL_COMPLETED;

getout:
    return OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Enable/disable reciving data.

****************************************************************************************************
*/
static PyObject *BrickBuffer_set_receive(
    BrickBuffer *self,
    PyObject *args,
    PyObject *kwds)
{
    int enable = OS_TRUE;
    static char *kwlist[] = {
        "enable",
        NULL
    };

    /* Get iocom root object pointer.
     */
    if (self->pyroot == OS_NULL) {
        PyErr_SetString(iocomError, "Root has been deleted");
        return NULL;
    }

    /* Parse Python argumenrs.
     */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &enable)) {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    ioc_brick_set_receive(&self->brick_buffer, enable);
    self->status = OSAL_SUCCESS;
    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Get brick from this buffer.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get(
    BrickBuffer *self,
    PyObject *args,
    PyObject *kwds)
{
    iocRoot *iocroot;
    PyObject *brick_data, *rval;
    iocBrickHdr *hdr;
    os_uchar *buf, *data;
    os_memsz buf_sz, data_sz;
    osalBitmapFormat format;
    os_uchar compression;
    os_int width, height;
    osalStatus s;

#if IOC_USE_JPEG_COMPRESSION
    osalJpegMallocContext alloc_context;
#endif

    int reserved = 0;
    static char *kwlist[] = {
        "reserved",
        NULL
    };

    /* Get iocom root object pointer.
     */
    if (self->pyroot == OS_NULL) {
        PyErr_SetString(iocomError, "Root has been deleted");
        return NULL;
    }
    iocroot = self->pyroot->root;

    /* Parse Python argumenrs.
     */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &reserved)) {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    /* Synchronize and setup all signals, if we have not done that already.
     */
    ioc_lock(iocroot);
    s = bb_try_setup(self, iocroot);
    if (OSAL_IS_ERROR(s))
    {
        ioc_unlock(iocroot);
        self->status = s;
        Py_RETURN_NONE;
    }

    /* Receive data, return None if we got no data.
     */
    s = ioc_run_brick_receive(&self->brick_buffer);
    buf_sz = self->brick_buffer.buf_sz;
    if (s != OSAL_COMPLETED || buf_sz <= (os_memsz)sizeof(iocBrickHdr))
    {
        ioc_unlock(iocroot);
        self->status = s;
        Py_RETURN_NONE;
    }

    buf = self->brick_buffer.buf;
    data = buf + sizeof(iocBrickHdr);
    data_sz = buf_sz - sizeof(iocBrickHdr);
    hdr = (iocBrickHdr*)buf;

    format = (osalBitmapFormat)hdr->format;
    compression = hdr->compression;
    width = (os_int)ioc_get_brick_hdr_int(hdr->width, IOC_BRICK_DIM_SZ);
    height = (os_int)ioc_get_brick_hdr_int(hdr->height, IOC_BRICK_DIM_SZ);

    if (compression == IOC_UNCOMPRESSED)
    {
        brick_data = PyBytes_FromStringAndSize((const char*)data, data_sz);
    }
#if IOC_USE_JPEG_COMPRESSION
    else if (compression & IOC_JPEG)
    {
        os_memclear(&alloc_context, sizeof(alloc_context));

        s = os_uncompress_JPEG(data, data_sz, OS_NULL, &alloc_context, OSAL_JPEG_DEFAULT);
        if (s) {
            os_free(alloc_context.buf, alloc_context.buf_sz);
            ioc_unlock(iocroot);
            self->status = OSAL_STATUS_FAILED;
            Py_RETURN_NONE;
        }
        else {
            brick_data = PyBytes_FromStringAndSize((const char*)alloc_context.buf, alloc_context.nbytes);
        }

        os_free(alloc_context.buf, alloc_context.buf_sz);
    }
#endif
    else
    {
        osal_debug_error_int("unsupported brick compression = ", compression);
        ioc_unlock(iocroot);
        self->status = OSAL_STATUS_NOT_SUPPORTED;
        Py_RETURN_NONE;
    }

    rval = PyList_New(5);
    PyList_SetItem(rval, 0, brick_data);
    PyList_SetItem(rval, 1, Py_BuildValue("i", (int)format));
    PyList_SetItem(rval, 2, Py_BuildValue("i", (int)width));
    PyList_SetItem(rval, 3, Py_BuildValue("i", (int)height));
    PyList_SetItem(rval, 4, Py_BuildValue("L", (long long)ioc_get_brick_hdr_int(hdr->tstamp, IOC_BRICK_TSTAMP_SZ)));

    /* End synchronization and return data.
     */
    ioc_unlock(iocroot);
    self->status = OSAL_SUCCESS;
    return rval;
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef BrickBuffer_members[] = {
    {(char*)"status", T_INT, offsetof(BrickBuffer, status), 0, (char*)"constructor status"},
    {NULL, 0, 0, 0, NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef BrickBuffer_methods[] = {
    {"delete", (PyCFunction)BrickBuffer_delete, METH_NOARGS, "Deletes IOCOM brick_buffer"},
    {"set_receive", (PyCFunction)BrickBuffer_set_receive, METH_VARARGS|METH_KEYWORDS, "Enable/disable reciving data"},
    {"get", (PyCFunction)BrickBuffer_get, METH_VARARGS|METH_KEYWORDS, "Get buffered data brick"},
//    {"set", (PyCFunction)BrickBuffer_set, METH_VARARGS|METH_KEYWORDS, "Store brick_buffer data"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject BrickBufferType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".BrickBuffer",  /* tp_name */
    sizeof(BrickBuffer),                      /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)BrickBuffer_dealloc,          /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_reserved */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "BrickBuffer objects",                    /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    BrickBuffer_methods,                      /* tp_methods */
    BrickBuffer_members,                      /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    BrickBuffer_new,                          /* tp_new */
};
