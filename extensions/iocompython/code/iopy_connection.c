/**

  @file    iopy_connection.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

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

  The Connection_new function starts running the connection. Running connection will keep
  on running until the connection is deleted. It will attempt repeatedly to connect socket,
  etc transport to other IOCOM device and will move the data when transport is there.

  Note: Application must not delete and create new connection to reestablish the transport.
  This is handled by the running connection objects.

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

    if (parameters == OS_NULL)
    {
        PyErr_SetString(iocomError, "No communication parameters");
        goto failed;
    }
    prm.parameters = parameters;

    if (flags == OS_NULL)
    {
        PyErr_SetString(iocomError, "No flags");
        goto failed;
    }

    if (os_strstr(flags, "dynamic", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_DYNAMIC_MBLKS;
    }

    if (os_strstr(flags, "tls", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_TLS_SUPPORT
        prm.flags |= IOC_SOCKET|IOC_CREATE_THREAD;
        prm.iface = OSAL_TLS_IFACE;
#else
        PyErr_SetString(iocomError, "TLS support if not included in eosal build");
        goto failed;
#endif
    }

    else if (os_strstr(flags, "socket", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_SOCKET_SUPPORT
        prm.flags |= IOC_SOCKET|IOC_CREATE_THREAD;
        prm.iface = OSAL_SOCKET_IFACE;
#else
        PyErr_SetString(iocomError, "Socket support if not included in eosal build");
        goto failed;
#endif
    }

    else if (os_strstr(flags, "bluetooth", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_BLUETOOTH_SUPPORT
        prm.flags |= IOC_SERIAL|IOC_CREATE_THREAD;
        prm.iface = OSAL_BLUETOOTH_IFACE;
#else
        PyErr_SetString(iocomError, "Bluetooth support if not included in eosal build");
        goto failed;
#endif
    }

    else if (os_strstr(flags, "serial", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_SERIAL_SUPPORT
        prm.flags |= IOC_SERIAL|IOC_CREATE_THREAD;
        prm.iface = OSAL_SERIAL_IFACE;
#else
        PyErr_SetString(iocomError, "Serial port support if not included in eosal build");
        goto failed;
#endif
    }
    else
    {
        PyErr_SetString(iocomError, "Transport (tls, socket, bluetooth or serial) must be specified in flags");
        goto failed;
    }

    if (os_strstr(flags, "up", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= IOC_CONNECT_UP;
    }
    else if (os_strstr(flags, "down", OSAL_STRING_SEARCH_ITEM_NAME) == OS_NULL)
    {
        PyErr_SetString(iocomError, "Either down or up flag must be given");
        goto failed;
    }

    self->con = ioc_initialize_connection(OS_NULL, iocroot);
    self->status = (int)ioc_connect(self->con, &prm);


#if IOPYTHON_TRACE
    PySys_WriteStdout("Connection.new(%s, %s)\n", prm.parameters, flags);
#endif

    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}


/**
****************************************************************************************************

  @brief Destructor.

  The Connection_dealloc function releases the associated Python object. The function doesn't
  release actual IOCOM connection. That will be deteted when root is deleted or explisitely
  by calling conn.delete().

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
/* static int Connection_init(
    Connection *self,
    PyObject *args,
    PyObject *kwds)
{
#if IOPYTHON_TRACE
    PySys_WriteStdout("Connection.init()\n");
#endif
    return 0;
}
*/


/**
****************************************************************************************************

  @brief Delete an IOCOM connection.

  The Connection_delete function closes the connection and releases any ressources for it.
  The connection must be explisitly closed by calling .delete() function, or by calling
  .delete() on the root object. But not both.

  @param   self Pointer to the python object.
  @return  None.

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

#if IOPYTHON_TRACE
    PySys_WriteStdout("Connection.delete()\n");
#endif
    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef Connection_members[] = {
    {(char*)"status", T_INT, offsetof(Connection, status), 0, (char*)"constructor status"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef Connection_methods[] = {
    {"delete", (PyCFunction)Connection_delete, METH_NOARGS, "Deletes IOCOM connection"},
    // {"connect", (PyCFunction)Connection_connect, METH_NOARGS, "Initiate IOCOM connection"},
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
  0, //  (initproc)Connection_init,                /* tp_init */
    0,                                        /* tp_alloc */
    Connection_new,                           /* tp_new */
};
