/**

  @file    iopy_root.c
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

  The Root_new function generates a new root object.
  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *Root_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    Root *self;

    const char
        *device_name = NULL,
        *network_name = NULL;

    int
        device_nr = 0;

    static char *kwlist[] = {
        "device_name",
        "device_nr",
        "network_name",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sis",
         kwlist, &device_name, &device_nr, &network_name))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    self = (Root *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    iocom_python_initialize();
    self->root = (iocRoot*)os_malloc(sizeof(iocRoot), OS_NULL);
    ioc_initialize_root(self->root);

    os_strncpy(self->network_name, network_name, IOC_NETWORK_NAME_SZ);
    os_strncpy(self->device_name, device_name, IOC_NAME_SZ);
    self->device_nr = device_nr;

    self->number = 1;

#if IOPYTHON_TRACE
    PySys_WriteStdout("Root.new(%s%d.%s)\n",
        self->device_name, self->device_nr, self->network_name);
#endif

    return (PyObject *)self;
}


/**
****************************************************************************************************

  @brief Destructor.

  The Root_dealloc function releases the associated Python object. It doesn't do anything
  for the actual IOCOM connection.

  @param   self Pointer to the Python object.
  @return  None.

****************************************************************************************************
*/
static void Root_dealloc(
    Root *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("Root.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Initialize.

  I do not think this is needed

  The Root_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static int Root_init(
    Root *self,
    PyObject *args,
    PyObject *kwds)
{
#if IOPYTHON_TRACE
    PySys_WriteStdout("Root.init()\n");
#endif
    return 0;
}


/**
****************************************************************************************************

  @brief Delete IOCOM root object.

  The Root_delete() function...
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *Root_delete(
    Root *self)
{
    osalStatus s;

    if (self->root)
    {
        ioc_release_root(self->root);
        self->root = OS_NULL;
        self->number = 0;
        PySys_WriteStdout("Root.delete()\n");
        s = OSAL_SUCCESS;
        iocom_python_release();
    }
    else
    {
        PySys_WriteStdout("Root.delete() called, IOCOM root is NULL\n");
        s = OSAL_STATUS_FAILED;
    }

    return PyLong_FromLong(s);
}


/**
****************************************************************************************************

  @brief Initialize.

  X...

  The Root_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *Root_set_callback(
    Root *self)
{
    osalStatus s;

    if (self->root)
    {
        ioc_set_root_callback(self->root, OS_NULL /* ioc_root_callback func*/, OS_NULL /* context */);
        s = OSAL_SUCCESS;
        PySys_WriteStdout("Root.set_callback\n");
    }
    else
    {
        s = OSAL_STATUS_FAILED;
        PySys_WriteStdout("Root.set_callback() called, IOCOM root is NULL\n");
    }

    return PyLong_FromLong(s);
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef Root_members[] = {
    {(char*)"number", T_INT, offsetof(Root, number), 0, (char*)"classy number"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef Root_methods[] = {
    {"delete", (PyCFunction)Root_delete, METH_NOARGS, "Delete IOCOM root object"},
    {"set_callback", (PyCFunction)Root_set_callback, METH_NOARGS, "Set IOCOM root callback function"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject RootType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".Root",  /* tp_name */
    sizeof(Root),                             /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)Root_dealloc,                 /* tp_dealloc */
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
    "Root objects",                           /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Root_methods,                             /* tp_methods */
    Root_members,                             /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)Root_init,                      /* tp_init */
    0,                                        /* tp_alloc */
    Root_new,                                 /* tp_new */
};
