/**

  @file    iopy_signal.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  An signal listens for incoming connections.

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

  The Signal_new function starts running the signal. Running signal will keep
  on running until the signal is deleted. It will attempt repeatedly to connect socket,
  etc transport to other IOCOM device and will move the data when transport is there.

  Note: Application must not delete and create new signal to reestablish the transport.
  This is handled by the running signal object.

  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *Signal_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    Signal *self;
    PyObject *pyroot = NULL;
    Root *root;

    iocRoot *iocroot;
    iocSignal *signal;
    iocHandle *handle;
    iocIdentifiers *identifiers;

    const char
        *io_path = NULL,
        *network_name = NULL;

    const os_char
        *topnet,
        *req_topnet;

    static char *kwlist[] = {
        "root",
        "io_path",
        "network_name",
        NULL
    };

    self = (Signal *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }
    signal = &self->signal;
    handle = &self->handle;
    identifiers = &self->identifiers;

    os_memclear(signal, sizeof(iocSignal));
    os_memclear(handle, sizeof(iocHandle));
    os_memclear(identifiers, sizeof(iocIdentifiers));
    signal->handle = handle;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ss",
         kwlist, &pyroot, &io_path, &network_name))
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

    if (io_path == OS_NULL)
    {
        PyErr_SetString(iocomError, "No IO path");
        goto failed;
    }

    ioc_iopath_to_identifiers(identifiers, io_path, IOC_EXPECT_SIGNAL);

    /* We do allow access between device networks, as long as these are subnets of the same
       top level network. This is useful to allow subnets in large IO networks. Care must be
       taken because here this could become a security vunerability.
     */
    if (network_name)
    {
        topnet = os_strchr((os_char*)network_name, '.');
        topnet = topnet ? topnet + 1 : network_name;
        req_topnet = os_strchr(identifiers->network_name, '.');
        req_topnet = req_topnet ? req_topnet + 1 : req_topnet;
        if (os_strcmp(topnet, req_topnet))
        {
            os_strncpy(identifiers->network_name, network_name, IOC_NETWORK_NAME_SZ);
        }
    }

    ioc_setup_signal_by_identifiers(iocroot, identifiers, signal);
    self->status = OSAL_SUCCESS;

#if IOPYTHON_TRACE
    PySys_WriteStdout("Signal.new(%s, %s)\n", io_path, network_name);
#endif

    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}


/**
****************************************************************************************************

  @brief Destructor.

  The Signal_dealloc function releases the associated Python object. It doesn't do anything
  for the actual IOCOM signal.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static void Signal_dealloc(
    Signal *self)
{
    ioc_release_handle(&self->handle);

    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("Signal.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Delete an IOCOM signal.

  The Signal_delete function closes the signal and releases any ressources for it.
  The signal must be explisitly closed by calling .delete() function, or by calling
  .delete() on the root object. But not both.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static PyObject *Signal_delete(
    Signal *self)
{
    ioc_release_handle(&self->handle);

#if IOPYTHON_TRACE
    PySys_WriteStdout("Signal.delete()\n");
#endif
    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


static osalStatus Signal_set_sequence(
    Signal *self,
    PyObject *args)
{
    PyObject *a, *py_repr, *py_str;
    osalTypeId fix_type, tag_name_or_addr_type, value_type;
    os_char tag_name_or_addr_str[128], value_str[128];

    long i, length;
    os_boolean expect_value;
    osalStatus s = OSAL_STATUS_FAILED;
    const os_char *str;

    /* Start with untyped data.
     */
    fix_type = OS_UNDEFINED_TYPE;
    expect_value = OS_FALSE;

    length = PySequence_Length(args);
    for(i = 0; i < length; i++)
    {
        a = PySequence_GetItem(args, i);

        /* If string
         */
        if (PyUnicode_Check(a))
        {
            py_repr = PyObject_Repr(a);
            py_str = PyUnicode_AsEncodedString(py_repr, "utf-8", "ignore"); // "~E~");
            str = PyBytes_AS_STRING(py_str);

            if (!expect_value)
            {
                os_strncpy(tag_name_or_addr_str, str, sizeof(tag_name_or_addr_str));
                tag_name_or_addr_type = OS_STR;
            }
            else
            {
                os_strncpy(value_str, str, sizeof(value_str));
                value_type = OS_STR;
            }
            PySys_WriteStdout("String: %s\n", str);
            Py_XDECREF(py_repr);
            Py_XDECREF(py_str);
        }

        else if (PyLong_Check(a)) {
            int  elem = PyLong_AsLong(a);
            PySys_WriteStdout("Long %d\n", elem);
        }

        else if (PyFloat_Check(a)) {
            double d = PyFloat_AsDouble(a);
            PySys_WriteStdout("Float %f\n", d);
        }

        else if (PySequence_Check(a))
        {
            PySys_WriteStdout("Sequence\n");

            s = Signal_set_sequence(self, a);


            /* long bn = PySequence_Length(a);
            for (int j = 0; j<bn; j++)
            {
                PyObject *b = PySequence_GetItem(a, j);
                if (PyLong_Check(b)) {
                    int  elem = PyLong_AsLong(b);
                    PySys_WriteStdout("LL %d\n", elem);
                }

                if (PyFloat_Check(b)) {
                    double d = PyFloat_AsDouble(b);
                    PySys_WriteStdout("FF %f\n", d);
                }
                Py_DECREF(b);
            } */
        }

        Py_DECREF(a);
        expect_value = !expect_value;
    }

    return s;
}


/**
****************************************************************************************************

  @brief Store value or arrya into memory block.
  @anchor Signal_get_param

  The Signal.get_param() function gets value of memory block's parameter.

  @param   self Pointer to the Python Signal object.
  @param   param_name Which parameter to get, one of "network_name", "device_name", "device_nr",
           or "mblk_name".
  @return  Parameter value as string.

****************************************************************************************************
*/
static PyObject *Signal_set(
    Signal *self,
    PyObject *args)
{
    self->status = Signal_set_sequence(self, args);
    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef Signal_members[] = {
    {(char*)"status", T_INT, offsetof(Signal, status), 0, (char*)"constructor status"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef Signal_methods[] = {
    {"delete", (PyCFunction)Signal_delete, METH_NOARGS, "Deletes IOCOM signal"},
    {"set", (PyCFunction)Signal_set, METH_VARARGS, "Store signal data for sending"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject SignalType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".Signal",  /* tp_name */
    sizeof(Signal),                         /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)Signal_dealloc,             /* tp_dealloc */
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
    "Signal objects",                       /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Signal_methods,                         /* tp_methods */
    Signal_members,                         /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    Signal_new,                             /* tp_new */
};
