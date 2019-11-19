/**

  @file    iopy_root.c
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
#include "extensions/iocompython/iocompython.h"


static void Root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocHandle *mblk_handle,
    iocRootCallbackEvent event,
    void *context);


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
        /* Dispose of the callback function.
         */
        if (self->root_callback)
        {
            Py_XDECREF(self->root_callback);
            self->root_callback = NULL;
        }

        /* Free the root data structure and everything that belong to it.
         */
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

  @brief Set python function as root callback.

  X...

  The Root_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *Root_set_callback(
    Root *self,
    PyObject *args)
{
    PyObject *temp;

    if (self->root == OS_NULL)
    {
        PyErr_SetString(iocomError, "no IOCOM root object");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "O:set_callback", &temp))
    {
        PyErr_SetString(PyExc_TypeError, "parsing argument failed");
        return NULL;
    }

    if (!PyCallable_Check(temp))
    {
        PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        return NULL;
    }

    /* Add a reference to new callback.
     */
    Py_XINCREF(temp);

    /* Dispose of the previous callback.
     */
    if (self->root_callback)
    {
        Py_XDECREF(self->root_callback);
        self->root_callback = NULL;
    }

    /* Remember the new callback.
     */
    self->root_callback = temp;
    ioc_set_root_callback(self->root, Root_callback, self);
    PySys_WriteStdout("Root.set_callback()\n");

    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/**
****************************************************************************************************

  @brief Call Python when new memory block is created, etc.

  The Root_callback function gets called by iocom library on event related to whole root,
  like when new memory block is created.

  @param   root Root object.
  @param   con Connection.
  @param   handle Memory block handle.
  @param   event Why the callback?
  @param   context ?
  @return  None.

****************************************************************************************************
*/
static void Root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocHandle *mblk_handle,
    iocRootCallbackEvent event,
    void *context)
{
    os_char text[128], mblk_name[IOC_NAME_SZ];

    int arg;
    PyObject *arglist;
    PyObject *result;

    Root *pyroot;
    pyroot = (Root*)context;

    /* If we have no callback function, then do nothing.
     */
    if (pyroot->root_callback == OS_NULL) return;

    switch (event)
    {
        /* Process "new dynamic memory block" callback.
         */
        case IOC_NEW_DYNAMIC_MBLK:
            ioc_memory_block_get_string_param(mblk_handle, IOC_MBLK_NAME, mblk_name, sizeof(mblk_name));

            os_strncpy(text, "Memory block ", sizeof(text));
            os_strncat(text, mblk_name, sizeof(text));
            os_strncat(text, " dynamically allocated\n", sizeof(text));
            osal_console_write(text);

            /* if (!os_strcmp(mblk_name, "info"))
            {
                ioc_add_callback(handle, info_callback, OS_NULL);
                ioc_memory_block_set_int_param(handle, IOC_MBLK_AUTO_SYNC_FLAG, OS_TRUE);
            } */

            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();

            /* Time to call the callback.
             */
            arglist = Py_BuildValue("(s)", text);
            result = PyObject_CallObject(pyroot->root_callback, arglist);
            Py_DECREF(arglist);

            /* What is that callback function reported an error?
             */
            if (result == NULL)
            {
                PyErr_Clear();
                PyGILState_Release(gstate);
                return;
                /* Pass error back ?? */
            }

            /* ...use result ... here we don't ...
             */

            Py_DECREF(result);

            /* Release the thread. No Python API allowed beyond this point.
             */
            PyGILState_Release(gstate);
            break;

        /* Ignore unknown callbacks. More callback events may be introduced in future.
         */
        default:
            break;
    }
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
    {"set_callback", (PyCFunction)Root_set_callback, METH_VARARGS, "Set IOCOM root callback function"},
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
