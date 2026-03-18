/**

  @file    iopy_end_point.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  An end point listens for incoming connections.

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

  The EndPoint_new function starts running the end point. Running end point will keep
  on running until the end point is deleted. It will attempt repeatedly to connect socket,
  etc transport to other IOCOM device and will move the data when transport is there.

  Note: Application must not delete and create new end point to reestablish the transport.
  This is handled by the running end point object.

  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *EndPoint_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    EndPoint *self;
    iocEndPointParams epprm;
    iocConnectionParams cprm;
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

    self = (EndPoint *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }
    self->con = OS_NULL;
    self->epoint = OS_NULL;

    os_memclear(&epprm, sizeof(epprm));
    os_memclear(&cprm, sizeof(cprm));

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

    cprm.parameters = parameters;
    epprm.parameters = parameters;

    if (flags == OS_NULL)
    {
        PyErr_SetString(iocomError, "No flags");
        goto failed;
    }

    if (os_strstr(flags, "dynamic", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        epprm.flags |= IOC_DYNAMIC_MBLKS;
        cprm.flags |= IOC_DYNAMIC_MBLKS;
    }

    if (os_strstr(flags, "tls", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_TLS_SUPPORT
        epprm.flags |= IOC_SOCKET|IOC_CREATE_THREAD;
        epprm.iface = OSAL_TLS_IFACE;
#else
        PyErr_SetString(iocomError, "TLS support if not included in eosal build");
        goto failed;
#endif
    }

    else if (os_strstr(flags, "socket", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_SOCKET_SUPPORT
        epprm.flags |= IOC_SOCKET|IOC_CREATE_THREAD;
        epprm.iface = OSAL_SOCKET_IFACE;
#else
        PyErr_SetString(iocomError, "Socket support if not included in eosal build");
        goto failed;
#endif
    }

    else if (os_strstr(flags, "bluetooth", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_BLUETOOTH_SUPPORT
        cprm.flags |= IOC_SERIAL|IOC_CREATE_THREAD|IOC_LISTENER;
        cprm.iface = OSAL_BLUETOOTH_IFACE;
#else
        PyErr_SetString(iocomError, "Bluetooth support if not included in eosal build");
        goto failed;
#endif
    }

    else if (os_strstr(flags, "serial", OSAL_STRING_SEARCH_ITEM_NAME))
    {
#ifdef OSAL_SERIAL_SUPPORT
        cprm.flags |= IOC_SERIAL|IOC_CREATE_THREAD|IOC_LISTENER;
        cprm.iface = OSAL_SERIAL_IFACE;
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

    /* if (os_strstr(flags, "listen", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        prm.flags |= 0;
    } */

    if (epprm.flags & IOC_SOCKET)
    {
        self->epoint = ioc_initialize_end_point(OS_NULL, iocroot);
        self->status = (int)ioc_listen(self->epoint, &epprm);
    }
    else
    {
        self->con = ioc_initialize_connection(OS_NULL, iocroot);
        self->status = (int)ioc_connect(self->con, &cprm);
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("EndPoint.new(%s, %s)\n", parameters, flags);
#endif

    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}


/**
****************************************************************************************************

  @brief Destructor.

  The EndPoint_dealloc function releases the associated Python object. It doesn't release
  the actual IOCOM end point. That will be released by when root object is deleted or
  explicitely by calling epoint.delete().

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static void EndPoint_dealloc(
    EndPoint *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("EndPoint.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Delete an IOCOM end point.

  The EndPoint_delete function closes the end point and releases any ressources for it.
  The end point must be explisitly closed by calling .delete() function, or by calling
  .delete() on the root object. But not both.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static PyObject *EndPoint_delete(
    EndPoint *self)
{
    if (self->epoint)
    {
        ioc_release_end_point(self->epoint);
        self->epoint = OS_NULL;
    }

    if (self->con)
    {
        ioc_release_connection(self->con);
        self->con = OS_NULL;
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("EndPoint.delete()\n");
#endif
    /* Return "None".
     */
    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef EndPoint_members[] = {
    {(char*)"status", T_INT, offsetof(EndPoint, status), 0, (char*)"constructor status"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef EndPoint_methods[] = {
    {"delete", (PyCFunction)EndPoint_delete, METH_NOARGS, "Deletes IOCOM end point"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject EndPointType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".EndPoint",  /* tp_name */
    sizeof(EndPoint),                         /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)EndPoint_dealloc,             /* tp_dealloc */
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
    "EndPoint objects",                       /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    EndPoint_methods,                         /* tp_methods */
    EndPoint_members,                         /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    EndPoint_new,                             /* tp_new */
};
