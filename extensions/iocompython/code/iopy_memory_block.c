/**

  @file    iopy_memory_block.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocompython.h"


/**
****************************************************************************************************

  @brief Constructor.

  The MemoryBlock_new function generates a new root object.
  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *MemoryBlock_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    MemoryBlock *self;
    iocMemoryBlockParams prm;
    PyObject *pyroot = NULL;
    Root *root;
    iocRoot *iocroot;
    iocIdentifiers identifiers;

    const char
        *mblk_name = NULL,
        *device_name = NULL,
        *network_name = NULL,
        *flags = NULL;

    int
        nbytes = IOC_MIN_MBLK_SZ,
        device_nr = 0;

    static char *kwlist[] = {
        "root",
        "flags",
        "mblk_name",
        "device_name",
        "device_nr",
        "network_name",
        "nbytes",
        NULL
    };

    self = (MemoryBlock *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }
    self->mblk_created = OS_FALSE;
    os_memclear(&self->mblk_handle, sizeof(iocHandle));
    os_memclear(&prm, sizeof(prm));

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|sssisi",
         kwlist, &pyroot, &flags, &mblk_name, &device_name, &device_nr, &network_name, &nbytes))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        goto failed;
    }

    if (pyroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "A root object is not given as argument");
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

    if (os_strstr(flags, "down", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_MBLK_DOWN;
    }
    if (os_strstr(flags, "up", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_MBLK_UP;
    }
    if (os_strstr(flags, "allow_resize", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_ALLOW_RESIZE;
    }
    if (os_strstr(flags, "static", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_STATIC;
    }

    ioc_iopath_to_identifiers(iocroot, &identifiers, mblk_name, IOC_EXPECT_MEMORY_BLOCK);
    prm.mblk_name = identifiers.mblk_name;
    prm.device_name = identifiers.device_name[0] ? identifiers.device_name : root->device_name;
    prm.device_nr = identifiers.device_nr ? identifiers.device_nr : root->device_nr;
    prm.network_name = identifiers.network_name[0] ? identifiers.network_name : root->network_name;

    if (device_name)
    {
        prm.device_name = device_name;
    }
    if (device_nr)
    {
        prm.device_nr = device_nr;
    }
    if (network_name)
    {
        prm.network_name = network_name;
    }

    if (nbytes < IOC_MIN_MBLK_SZ) nbytes = IOC_MIN_MBLK_SZ;
    prm.nbytes = nbytes;

    /* Try to find memory block.
     */
    if (ioc_find_mblk(iocroot, &self->mblk_handle, prm.mblk_name,
        prm.device_name, prm.device_nr, prm.network_name) != OSAL_SUCCESS)
    {
        /* If we have no up nor down flag, we searched for memory block which was
           not found.
         */
        if ((prm.flags & (IOC_MBLK_UP|IOC_MBLK_DOWN)) == 0)
        {
            ioc_setup_handle(&self->mblk_handle, iocroot, OS_NULL);
            self->number = OSAL_STATUS_FAILED;
            return (PyObject *)self;
        }

        /* Not found, create new one.
         */
        ioc_initialize_memory_block(&self->mblk_handle, OS_NULL, iocroot, &prm);
        self->mblk_created = OS_TRUE;
    }

    self->number = OSAL_SUCCESS;

#if IOPYTHON_TRACE
    PySys_WriteStdout("MemoryBlock.new(%s.%s%d.%s)\n",
        prm.mblk_name,
        prm.device_name, prm.device_nr, prm.network_name);
#endif
    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}


/**
****************************************************************************************************

  @brief Destructor.

  The MemoryBlock_dealloc function releases the associated Python object.

  It doesn't do release actual IOCOM memory block. The IOCOM memory block will be deleted once
  the root is deleted or explicetely by calling mblk.delete()

  @param   self Pointer to the Python MemoryBlock object.
  @return  None.

****************************************************************************************************
*/
static void MemoryBlock_dealloc(
    MemoryBlock *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("MemoryBlock.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Initialize.

  I do not think this is needed

  The MemoryBlock_init function initializes an object.
  @param   self Pointer to the Python MemoryBlock object.
  @return  ?.

****************************************************************************************************
*/
static int MemoryBlock_init(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
#if IOPYTHON_TRACE
    PySys_WriteStdout("MemoryBlock.init()\n");
#endif
    return 0;
}


/**
****************************************************************************************************

  @brief Initialize.

  X...

  The MemoryBlock_init function initializes an object.
  @param   self Pointer to the Python MemoryBlock object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *MemoryBlock_delete(
    MemoryBlock *self)
{
    if (self->mblk_created)
    {
        ioc_release_memory_block(&self->mblk_handle);
    }
    else
    {
        ioc_release_handle(&self->mblk_handle);
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("MemoryBlock.delete()\n");
#endif
    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Get memory block parameter value.
  @anchor MemoryBlock_get_param

  The MemoryBlock.get_param() function gets value of memory block's parameter.

  @param   self Pointer to the Python MemoryBlock object.
  @param   param_name Which parameter to get, one of "network_name", "device_name", "device_nr",
           or "mblk_name".
  @return  Parameter value as string.

****************************************************************************************************
*/
static PyObject *MemoryBlock_get_param(
    MemoryBlock *self,
    PyObject *args)
{
    const char *param_name = OS_NULL;
    os_char buf[128];
    iocMemoryBlockParamIx param_ix;
    PyObject *rval;
    os_boolean is_int = OS_FALSE;

    if (!PyArg_ParseTuple(args, "s", param_name))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    if (!os_strcmp(param_name, "network_name"))
    {
        param_ix = IOC_NETWORK_NAME;
    }
    else if (!os_strcmp(param_name, "device_name"))
    {
        param_ix = IOC_DEVICE_NAME;
    }
    else if (!os_strcmp(param_name, "device_nr"))
    {
        param_ix = IOC_DEVICE_NR;
        is_int = OS_TRUE;
    }
    else if (!os_strcmp(param_name, "mblk_name"))
    {
        param_ix = IOC_MBLK_NAME;
    }
    else if (!os_strcmp(param_name, "mblk_sz"))
    {
        param_ix = IOC_MBLK_SZ;
        is_int = OS_TRUE;
    }
    else
    {
        PyErr_SetString(iocomError, "Unknown parameter name");
        return NULL;
    }

    if (is_int)
    {
        rval = Py_BuildValue("i", (int)ioc_memory_block_get_int_param(&self->mblk_handle, param_ix));
    }
    else
    {
        ioc_memory_block_get_string_param(&self->mblk_handle, param_ix, buf, sizeof(buf));
        rval = Py_BuildValue("s", buf);
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Set memory block parameter value.
  @anchor MemoryBlock_set_param

  The MemoryBlock.set_param() function gets value of memory block's parameter.

  THIS FUNCTION IS NOT NEEDED, WAS USED FOR AUTO SYNC

  param_name Currently only "auto" can be set. Controlles wether to use automatic (value 1)
  or synchronous sending/receiving (value 0).

****************************************************************************************************
*/
static PyObject *MemoryBlock_set_param(
    MemoryBlock *self,
    PyObject *args)
{
    const char *param_name = OS_NULL;
    int param_value = 0;
    iocMemoryBlockParamIx param_ix;

    if (!PyArg_ParseTuple(args, "si", &param_name, &param_value))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    /* if (!os_strcmp(param_name, "auto"))
    {
        param_ix = IOC_MBLK_AUTO_SYNC_FLAG;
    }
    else
    { */
        PyErr_SetString(iocomError, "Unknown parameter");
        return NULL;
    /* } */

    // ioc_memory_block_set_int_param(&self->mblk_handle, param_ix, param_value);

    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  Read data from memory block.
****************************************************************************************************
*/
static PyObject *MemoryBlock_read(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
    PyObject *rval;
    os_char *data;
    int pyaddr = 0, pynbytes = -1;
    os_int mblk_sz, n;

    static char *kwlist[] = {
        "addr",
        "n",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ii",
         kwlist, &pyaddr, &pynbytes))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    mblk_sz = ioc_memory_block_get_int_param(&self->mblk_handle, IOC_MBLK_SZ);
    if (mblk_sz <= 0) goto getout;

    n = pynbytes >= 0 ? pynbytes : mblk_sz;
    if (pyaddr + n > mblk_sz) n = mblk_sz - pyaddr;
    if (n <= 0) goto getout;

    data = os_malloc(n, OS_NULL);
    ioc_read(&self->mblk_handle, pyaddr, data, n, 0);
    rval = PyBytes_FromStringAndSize(data, n);

    os_free(data, n);

    return rval;

getout:
    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  Write binary data to memory block.
****************************************************************************************************
*/
static PyObject *MemoryBlock_write(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
    PyObject *pydata = NULL;
    int pyaddr = 0;

    char *buffer;
    Py_ssize_t length;

    static char *kwlist[] = {
        "data",
        "addr",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i",
         kwlist, &pydata, &pyaddr))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    PyBytes_AsStringAndSize(pydata, &buffer, &length);
    ioc_write(&self->mblk_handle, pyaddr, buffer, (os_int)length, 0);

    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  Publish memory block content as dynamic IO network information.
  The Python example below sets signal configuration for an IO device. This function should not
  be called on server side.

  \verbatim
  signal_conf = ('{'
    '"mblk": ['
    '{'
      '"name": "exp",'
      '"groups": ['
         '{'
           '"name": "control",'
           '"signals": ['
             '{"name": "ver", "type": "short"},'
             '{"name": "hor"}'
           ']'
         '}'
      ']'
    '}'
    ']'
  '}')

  data = json2bin(signal_conf)
  info = MemoryBlock(root, 'source,auto', 'info', nbytes=len(data))
  info.publish(data)
  \endverbatim
****************************************************************************************************
*/
static PyObject *MemoryBlock_publish(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
    PyObject *pydata = NULL;
    iocRoot *root;
    int pyaddr = 0;

    char *buffer;
    Py_ssize_t length;

    static char *kwlist[] = {
        "data",
        "addr",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i",
         kwlist, &pydata, &pyaddr))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    /* If we got data as argument, set it first.
     */
    if (pydata)
    {
        PyBytes_AsStringAndSize(pydata, &buffer, &length);
        ioc_write(&self->mblk_handle, pyaddr, buffer, (os_int)length, 0);
    }

    /* Publish block content as dynamic structure. Resize memory block (make bigger only).
     */
    root = self->mblk_handle.root;
    if (root) if (root->droot) {
        ioc_add_dynamic_info(root->droot, &self->mblk_handle, OS_TRUE);
    }

    Py_RETURN_NONE;
}


/* Send data synchronously.
 */
static PyObject *MemoryBlock_send(
    MemoryBlock *self)
{
    ioc_send(&self->mblk_handle);

    Py_RETURN_NONE;
}


/* Receive data synchronously.
 */
static PyObject *MemoryBlock_receive(
    MemoryBlock *self)
{
    ioc_receive(&self->mblk_handle);

    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef MemoryBlock_members[] = {
    {"number", T_INT, offsetof(MemoryBlock, number), 0, "classy number"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef MemoryBlock_methods[] = {
    {"delete", (PyCFunction)MemoryBlock_delete, METH_NOARGS, "Deletes IOCOM memory block"},
    {"get_param", (PyCFunction)MemoryBlock_get_param, METH_VARARGS, "Get memory block parameter"},
    {"set_param", (PyCFunction)MemoryBlock_set_param, METH_VARARGS, "Set memory block parameter"},
    {"read", (PyCFunction)MemoryBlock_read, METH_VARARGS|METH_KEYWORDS, "Read data from memory block"},
    {"write", (PyCFunction)MemoryBlock_write, METH_VARARGS|METH_KEYWORDS, "Write data to memory block"},
    {"publish", (PyCFunction)MemoryBlock_publish, METH_VARARGS|METH_KEYWORDS, "Publish as dynamic IO info"},
    {"send", (PyCFunction)MemoryBlock_send, METH_NOARGS, "Send data synchronously"},
    {"receive", (PyCFunction)MemoryBlock_receive, METH_NOARGS, "Receive data synchronously"},

    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject MemoryBlockType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".MemoryBlock",  /* tp_name */
    sizeof(MemoryBlock),                      /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)MemoryBlock_dealloc,          /* tp_dealloc */
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
    "MemoryBlock objects",                    /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    MemoryBlock_methods,                      /* tp_methods */
    MemoryBlock_members,                      /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)MemoryBlock_init,               /* tp_init */
    0,                                        /* tp_alloc */
    MemoryBlock_new,                          /* tp_new */
};
