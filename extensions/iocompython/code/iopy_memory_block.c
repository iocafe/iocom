/**

  @file    iopy_memory_block.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iocompython/iocompython.h"


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

    const char
        *mblk_name = NULL,
        *device_name = NULL,
        *network_name = NULL,
        *flags = NULL;

    int
        mblk_nr = 0,
        nbytes = 128,
        device_nr = 0;

    static char *kwlist[] = {
        "root",
        "flags",
        "mblk_name",
        "mblk_nr",
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

    os_memclear(&prm, sizeof(prm));

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ssisisi",
         kwlist, &pyroot, &flags, &mblk_name, &mblk_nr, &device_name, &device_nr, &network_name, &nbytes))
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
        PyErr_SetString(iocomError, "The root argument is not an instance of the Root class");
        goto failed;
    }

    root = (Root*)pyroot;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "The root object has been internally deleted");
        goto failed;
    }

    if (os_strstr(flags, "source", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_SOURCE;
    }
    if (os_strstr(flags, "target", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_TARGET;
    }
    if (prm.flags == 0)
    {
        PyErr_SetString(iocomError, "Memory block must have either target or source flag");
        goto failed;
    }
    if (os_strstr(flags, "auto_sync", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_AUTO_SYNC;
    }
    if (os_strstr(flags, "allow_resize", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_ALLOW_RESIZE;
    }
    if (os_strstr(flags, "static", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_STATIC;
    }

    prm.mblk_name = mblk_name;
    prm.mblk_nr = mblk_nr;

    prm.device_name = root->device_name;
    prm.device_nr = root->device_nr;
    prm.network_name = root->network_name;

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

    if (nbytes < 24) nbytes = 24;
    prm.nbytes = nbytes;

    ioc_initialize_memory_block(&self->mblk_handle, OS_NULL, iocroot, &prm);
    self->number = 1;

#if IOPYTHON_TRACE
    PySys_WriteStdout("MemoryBlock.new(%s%d.%s%d.%s)\n",
        prm.mblk_name, prm.mblk_nr,
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

  The MemoryBlock_dealloc function releases the associated Python object. It doesn't do anything
  for the actual IOCOM memory block.

  @param   self Pointer to the Python MemoryBlock object.
  @return  None.

****************************************************************************************************
*/
static void MemoryBlock_dealloc(
    MemoryBlock *self)
{
    ioc_release_handle(&self->mblk_handle);

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
    ioc_release_memory_block(&self->mblk_handle);
    self->number = 0;

#if IOPYTHON_TRACE
    PySys_WriteStdout("MemoryBlock.delete()\n");
#endif
    return PyLong_FromLong((long)self->number);
}


/**
****************************************************************************************************

  @brief Get memory block parameter value.
  @anchor MemoryBlock_get_param

  The MemoryBlock.get_param() function gets value of memory block's parameter.

  @param   self Pointer to the Python MemoryBlock object.
  @param   param_name Which parameter to get, one of "network_name", "device_name", "device_nr",
           "mblk_name" or "mblk_nr".
  @return  Parameter value as string.

****************************************************************************************************
*/
static PyObject *MemoryBlock_get_param(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwargs)
{
    const char
        *param_name = NULL;

    char
        buf[128];

    iocMemoryBlockParamIx
        param_ix;

    static char *kwlist[] = {
        "param_name",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s",
         kwlist, &param_name))
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
    }
    else if (!os_strcmp(param_name, "mblk_name"))
    {
        param_ix = IOC_MBLK_NAME;
    }
    else if (!os_strcmp(param_name, "mblk_nr"))
    {
        param_ix = IOC_MBLK_NR;
    }
    else
    {
        PyErr_SetString(iocomError, "Unknown parameter name");
        return NULL;
    }

    ioc_memory_block_get_string_param(&self->mblk_handle, param_ix, buf, sizeof(buf));

#if IOPYTHON_TRACE
    PySys_WriteStdout("MemoryBlock.get_param()\n");
#endif

    return PyUnicode_FromString(buf);
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
    {"get_param", (PyCFunction)MemoryBlock_get_param, METH_VARARGS|METH_KEYWORDS, "Get memory block parameter"},
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
