/**

  @file    iopy_connection.c
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

  The Connection_new function generates a new connection.
  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *Connection_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    Connection *self;
    iocConnectionParams prm;
    PyObject *pyroot = NULL;
    Root *root;
    iocRoot *iocroot;

    const char
        *parameters = NULL,
        *flags = NULL;

    static char *kwlist[] = {
        "root",
        "parameters",
        "flags",
        NULL
    };

    self = (Connection *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    os_memclear(&prm, sizeof(prm));

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ss",
         kwlist, &pyroot, &parameters, &flags))
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

    self->con = ioc_initialize_connection(OS_NULL, iocroot);
    self->number = 1;

//    prm.flags = ??
// prm.iface

    PySys_WriteStdout("new\n");

    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}


/**
****************************************************************************************************

  @brief Destructor.

  The Connection_dealloc function releases the associated Python object. It doesn't do anything
  for the actual IOCOM connection.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static void Connection_dealloc(
    Connection *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("Connection.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Initialize.

  I do not think this is needed

  The Connection_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static int Connection_init(
    Connection *self,
    PyObject *args,
    PyObject *kwds)
{
#if IOPYTHON_TRACE
    PySys_WriteStdout("Connection.init()\n");
#endif
    return 0;
}


/**
****************************************************************************************************

  @brief Delete an IOCOM connection.

  The Connection_delete function closes the connection and releases any ressources for it.
  The connection must be explisitly closed by calling .delete() function, or by calling
  .delete() on the root object. But not both.

  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *Connection_delete(
    Connection *self)
{
    if (self->con)
    {
        ioc_release_connection(self->con);
        self->con = OS_NULL;
    }
    self->number = 0;

#if IOPYTHON_TRACE
    PySys_WriteStdout("Connection.delete()\n");
#endif
    return PyLong_FromLong((long)self->number);
}


/**
****************************************************************************************************

  @brief Initialize.

  X...

  The Connection_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *Connection_connect(
    Connection *self)
{
    osalStatus s;
    iocConnectionParams prm;

    if (self->con)
    {
        os_memclear(&prm, sizeof(prm));
        s = ioc_connect(self->con, &prm);
        self->number = 2;
    }
    else
    {
        s = OSAL_STATUS_FAILED;
    }

    PySys_WriteStdout("Connection.delete()\n");
    return PyLong_FromLong(s);
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef Connection_members[] = {
    {(char*)"number", T_INT, offsetof(Connection, number), 0, (char*)"classy number"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef Connection_methods[] = {
    {"delete", (PyCFunction)Connection_delete, METH_NOARGS, "Deletes IOCOM connection"},
    {"connect", (PyCFunction)Connection_connect, METH_NOARGS, "Initiate IOCOM connection"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject ConnectionType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".Connection",  /* tp_name */
    sizeof(Connection),                       /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)Connection_dealloc,           /* tp_dealloc */
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
    "Connection objects",                     /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Connection_methods,                       /* tp_methods */
    Connection_members,                       /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)Connection_init,                /* tp_init */
    0,                                        /* tp_alloc */
    Connection_new,                           /* tp_new */
};
