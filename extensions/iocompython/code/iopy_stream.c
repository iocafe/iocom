/**

  @file    iopy_stream.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.12.2019

  Streaming data trough memory block.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocompython.h"


static void Stream_init_signals(
    iocStreamerSignals *ptrs,
    iocStreamSignalsStruct *signal_struct,
    iocHandle *exp_handle,
    iocHandle *imp_handle,
    os_boolean is_frd);

static iocSignal *Stream_clear_signal(
    iocSignal *signal,
    iocHandle *handle);

static osalStatus Stream_setup_signals(
    Stream *self,
    iocStreamSignalsStruct *sigs,
    os_boolean is_frd,
    iocRoot *iocroot);

static osalStatus Stream_setup_one(
    iocSignal *signal,
    char *signal_name_prefix,
    char *signal_name_end,
    iocIdentifiers *identifiers,
    iocRoot *iocroot);


/**
****************************************************************************************************

  @brief Constructor.

  The Stream_new function starts running the signal. Running signal will keep
  on running until the signal is deleted. It will attempt repeatedly to connect socket,
  etc transport to other IOCOM device and will move the data when transport is there.

  Note: Application must not delete and create new signal to reestablish the transport.
  This is handled by the running signal object.

  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *Stream_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    Stream *self;
    PyObject *pyroot = NULL;
    Root *root;
    iocStreamerParams *prm;
    iocRoot *iocroot;
    os_char *p;

    const char
        *read_buf_name = NULL,
        *write_buf_name = NULL,
        *exp_mblk_path = NULL,
        *imp_mblk_path = NULL,
        *select = NULL;

    static char *kwlist[] = {
        "root",
        "read",
        "write",
        "exp",
        "imp",
        "select",
        NULL
    };

    self = (Stream *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    prm = &self->prm;
    os_memclear(prm, sizeof(iocStreamerParams));
    os_memclear(&self->exp_handle, sizeof(iocHandle));
    os_memclear(&self->imp_handle, sizeof(iocHandle));

    Stream_init_signals(&prm->frd, &self->frd, &self->exp_handle, &self->imp_handle, OS_TRUE);
    Stream_init_signals(&prm->tod, &self->tod, &self->exp_handle, &self->imp_handle, OS_FALSE);
    prm->tod.to_device = OS_TRUE;
    prm->is_device = OS_FALSE;
    prm->static_memory_allocation = OS_TRUE;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|sss",
         kwlist, &pyroot,  &read_buf_name, &write_buf_name,
        &exp_mblk_path, &imp_mblk_path, &select))
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

    if (exp_mblk_path == OS_NULL || imp_mblk_path == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        goto failed;
    }

    os_strncpy(self->frd_signal_name_prefix, read_buf_name, IOC_SIGNAL_NAME_SZ);
    p = os_strchr(self->frd_signal_name_prefix, '_');
    if (p) p[1] = '\0';
    os_strncpy(self->tod_signal_name_prefix, write_buf_name, IOC_SIGNAL_NAME_SZ);
    p = os_strchr(self->tod_signal_name_prefix, '_');
    if (p) p[1] = '\0';

    ioc_iopath_to_identifiers(iocroot, &self->exp_identifiers, exp_mblk_path, IOC_EXPECT_MEMORY_BLOCK);
    ioc_iopath_to_identifiers(iocroot, &self->imp_identifiers, imp_mblk_path, IOC_EXPECT_MEMORY_BLOCK);

    self->status = OSAL_SUCCESS;

    /* Save root pointer and increment reference count.
     */
    self->pyroot = root;
    Py_INCREF(root);

    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}

static void Stream_init_signals(
    iocStreamerSignals *ptrs,
    iocStreamSignalsStruct *signal_struct,
    iocHandle *exp_handle,
    iocHandle *imp_handle,
    os_boolean is_frd)
{
    ptrs->cmd = Stream_clear_signal(&signal_struct->cmd, imp_handle);
    ptrs->select = Stream_clear_signal(&signal_struct->select, imp_handle);
    ptrs->buf = Stream_clear_signal(&signal_struct->buf, is_frd ? exp_handle : imp_handle);
    ptrs->head = Stream_clear_signal(&signal_struct->head, is_frd ? exp_handle : imp_handle);
    ptrs->tail = Stream_clear_signal(&signal_struct->tail, is_frd ? imp_handle : exp_handle);
    ptrs->state = Stream_clear_signal(&signal_struct->state, exp_handle);
}

static iocSignal *Stream_clear_signal(
    iocSignal *signal,
    iocHandle *handle)
{
    os_memclear(signal, sizeof(iocSignal));
    signal->handle = handle;
    return signal;
}


/**
****************************************************************************************************

  @brief Destructor.

  The Stream_dealloc function releases the associated Python object.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static void Stream_dealloc(
    Stream *self)
{
    ioc_release_handle(&self->exp_handle);
    ioc_release_handle(&self->imp_handle);

    if (self->pyroot)
    {
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("Stream.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Delete an IOCOM stream.

  The Stream_delete function closes the signal and releases any ressources for it.
  The signal must be explisitly closed by calling .delete() function, or by calling
  .delete() on the root object. But not both.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static PyObject *Stream_delete(
    Stream *self)
{
    ioc_release_handle(&self->exp_handle);
    ioc_release_handle(&self->imp_handle);

    if (self->pyroot)
    {
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("Stream.delete()\n");
#endif
    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/* Lock must be on
 * */
static osalStatus Stream_try_setup(
    Stream *self,
    iocRoot *iocroot)
{
    /* If we have all set up already ?
     */
    if (self->exp_handle.mblk && self->imp_handle.mblk)
    {
        return OSAL_SUCCESS;
    }

    if (self->frd_signal_name_prefix[0] != '\0')
    {
        if (Stream_setup_signals(self, &self->frd, OS_TRUE, iocroot)) goto failed;
    }
    if (self->tod_signal_name_prefix[0] != '\0')
    {
        if (Stream_setup_signals(self, &self->tod, OS_FALSE, iocroot)) goto failed;
    }

    return OSAL_SUCCESS;

failed:
    ioc_release_handle(&self->exp_handle);
    ioc_release_handle(&self->imp_handle);
    return OSAL_STATUS_FAILED;
}


static osalStatus Stream_setup_signals(
    Stream *self,
    iocStreamSignalsStruct *sigs,
    os_boolean is_frd,
    iocRoot *iocroot)
{
    os_char *prefix;
    iocIdentifiers *ei, *ii;

    ei = &self->exp_identifiers;
    ii = &self->imp_identifiers;

    prefix = is_frd ? self->frd_signal_name_prefix : self->tod_signal_name_prefix;

    if (Stream_setup_one(&sigs->cmd, prefix, "cmd", ii, iocroot)) return OSAL_STATUS_FAILED;
    if (Stream_setup_one(&sigs->select, prefix, "select", ii, iocroot)) return OSAL_STATUS_FAILED;
    if (Stream_setup_one(&sigs->buf, prefix, "select", is_frd ? ei : ii, iocroot)) return OSAL_STATUS_FAILED;
    if (Stream_setup_one(&sigs->head, prefix, "head", is_frd ? ei : ii, iocroot)) return OSAL_STATUS_FAILED;
    if (Stream_setup_one(&sigs->tail, prefix, "tail", is_frd ? ii : ei, iocroot)) return OSAL_STATUS_FAILED;
    if (Stream_setup_one(&sigs->state, prefix, "state", ei, iocroot)) return OSAL_STATUS_FAILED;

    return OSAL_SUCCESS;
}


static osalStatus Stream_setup_one(
    iocSignal *signal,
    char *signal_name_prefix,
    char *signal_name_end,
    iocIdentifiers *identifiers,
    iocRoot *iocroot)
{
    iocDynamicSignal *dsignal;

    os_strncpy(identifiers->signal_name, signal_name_prefix, IOC_SIGNAL_NAME_SZ);
    os_strncat(identifiers->signal_name, signal_name_end, IOC_SIGNAL_NAME_SZ);

    dsignal = ioc_setup_signal_by_identifiers(iocroot, identifiers, signal);
    return dsignal ? OSAL_SUCCESS : OSAL_STATUS_FAILED;
}


static PyObject *Stream_read_or_write(
    Stream *self,
    PyObject *args,
    PyObject *kwds,
    os_int flags)
{
    int nro_bytes = 0;
    Root *root;
    iocRoot *iocroot;

    static char *kwlist[] = {
        "n",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i",
         kwlist, &nro_bytes))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    root = (Root*)self;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        Py_RETURN_NONE;
    }

    if (Stream_try_setup(self,  iocroot))
    {
        osal_trace2("Unable to setup stream transfer, maybe not ready yet");
        Py_RETURN_NONE;
    }
#if 0
    if (never opened)
    {
        // status->select = ??

        osalStream ioc_streamer_open(
            const os_char *parameters,
            void *option,
            osalStatus *status,
            os_int flags);
    }


/* Close streamer.
 */
void ioc_streamer_close(
    osalStream stream);

#endif

    Py_RETURN_NONE;
}


static PyObject *Stream_read(
    Stream *self,
    PyObject *args,
    PyObject *kwds)
{
    return Stream_read_or_write(self, args, kwds, OSAL_STREAM_READ);
}


static PyObject *Stream_write(
    Stream *self,
    PyObject *args,
    PyObject *kwds)
{
    return Stream_read_or_write(self, args, kwds, OSAL_STREAM_WRITE);
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef Stream_members[] = {
    {(char*)"status", T_INT, offsetof(Stream, status), 0, (char*)"constructor status"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef Stream_methods[] = {
    {"delete", (PyCFunction)Stream_delete, METH_NOARGS, "Deletes IOCOM signal"},
    {"read", (PyCFunction)Stream_read, METH_VARARGS|METH_KEYWORDS, "Read data from stream"},
    {"write", (PyCFunction)Stream_write, METH_VARARGS|METH_KEYWORDS, "Write data to stream"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject StreamType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".Stream",  /* tp_name */
    sizeof(Stream),                           /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)Stream_dealloc,               /* tp_dealloc */
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
    "Stream objects",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Stream_methods,                           /* tp_methods */
    Stream_members,                           /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    Stream_new,                               /* tp_new */
};
