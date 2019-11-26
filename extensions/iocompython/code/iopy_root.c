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
#include "iocompython.h"


static void Root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocHandle *mblk_handle,
    iocRootCallbackEvent event,
    void *context);

static void Root_network_callback(
    struct iocRoot *root,
    struct iocDynamicNetwork *dnetwork,
    iocDynamicNetworkEvent event,
    void *context);

static void Root_do_callback(
    Root *pyroot,
    os_char *text);

static void Root_info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
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

    /* Allocate and initialize communication root and dymanic structure data root objects.
     * This demo uses dynamic signal configuration.
     */
    self->root = (iocRoot*)os_malloc(sizeof(iocRoot), OS_NULL);
    ioc_initialize_root(self->root);
    ioc_initialize_dynamic_root(self->root);

    /* Set callback function to receive information about created or removed dynamic IO networks.
     */
    ioc_set_dnetwork_callback(self->root, Root_network_callback, self);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(self->root, Root_callback, self);

    /* Save network and device.
     */
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

#if IOPYTHON_TRACE
        PySys_WriteStdout("Root.delete()\n");
#endif
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

  The Root_set_callback function sets function to call when IOCOM root object has information
  to pass to the Python application.

  @param   self Pointer to the python object.
  @param   args Callback function (Python callable).
  @return  None.

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
    Root *pyroot;
    pyroot = (Root*)context;

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

            if (!os_strcmp(mblk_name, "info"))
            {
                ioc_add_callback(mblk_handle, Root_info_callback, OS_NULL);
            }

            Root_do_callback(pyroot, text);
            break;

        /* Ignore unknown callbacks. More callback events may be introduced in future.
         */
        default:
            break;
    }
}


/**
****************************************************************************************************

  @brief Callback when dynamic IO network has been connected or disconnected.

  The info_callback() function is called when device information data is received from connection
  or when connection status changes.

  @param   root Pointer to the root object.
  @param   dnetwork Pointer to dynamic network object which has just been connected or is
           about to be removed.
  @param   event Either IOC_NEW_DYNAMIC_NETWORK or IOC_DYNAMIC_NETWORK_REMOVED.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void Root_network_callback(
    struct iocRoot *root,
    struct iocDynamicNetwork *dnetwork,
    iocDynamicNetworkEvent event,
    void *context)
{
    Root *pyroot;
    pyroot = (Root*)context;

    switch (event)
    {
        case IOC_NEW_DYNAMIC_NETWORK:
            osal_trace2("IOC_NEW_DYNAMIC_NETWORK");

            Root_do_callback(pyroot, "new_network");
            break;

        case IOC_DYNAMIC_NETWORK_REMOVED:
            osal_trace2("IOC_DYNAMIC_NETWORK_REMOVED");
            break;
    }
}


/**
****************************************************************************************************

  @brief Do call the Python callback function.

  The Root_do_callback() function is called when device information data is received from connection
  or when connection status changes.
  @return  None.

****************************************************************************************************
*/
static void Root_do_callback(
    Root *pyroot,
    os_char *text)
{
    PyObject *arglist, *result;
    PyGILState_STATE gstate;

    /* If we have no python application callback function, then do nothing more.
     */
    if (pyroot->root_callback == OS_NULL) return;

    gstate = PyGILState_Ensure();

    /* Call the callback.
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
}


/**
****************************************************************************************************

  @brief Callback function to add dynamic device information.

  The info_callback() function is called when device information data is received from connection
  or when connection status changes.

  @param   mblk Pointer to the memory block object.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void Root_info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    iocRoot *root;
    root = handle->root;

    /* If actual data received (not connection status change).
     */
    if (end_addr >= 0 && root)
    {
        ioc_add_dynamic_info(handle);
    }
}


/**
****************************************************************************************************

  @brief Return IOCOM internal state printout.

  The Root_print function initializes an object.
  Example: print(root.print('memory_blocks'))

  @param   self Pointer to the python object.
  @param   args Which list to get.
  @return  Requested printout as JSON.

****************************************************************************************************
*/
static PyObject *Root_print(
    Root *self,
    PyObject *args)
{
    PyObject *rval;
    const char *param1, *param2 = OS_NULL, *param3 = OS_NULL;
    osalStream stream;
    os_char *p;
    os_memsz n;
    os_short flags;

    if (self->root == OS_NULL)
    {
        PyErr_SetString(iocomError, "no IOCOM root object");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s|ss", &param1, &param2, &param3))
    {
        PyErr_SetString(iocomError, "errornous function arguments");
        return NULL;
    }

    stream = osal_stream_buffer_open(OS_NULL, 0, OS_NULL, 0);

    if (!os_strcmp(param1, "connections"))
    {
        devicedir_connections(self->root, stream, 0);
    }
    else if (!os_strcmp(param1, "end_points"))
    {
        devicedir_end_points(self->root, stream, 0);
    }
    else if (!os_strcmp(param1, "memory_blocks"))
    {
        flags = IOC_DEVDIR_DEFAULT;
        if (os_strstr(param2, "data", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_DEVDIR_DATA;
        if (os_strstr(param2, "buffers", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_DEVDIR_BUFFERS;

        if (flags & (IOC_DEVDIR_DATA|IOC_DEVDIR_BUFFERS))
        {
            param2 = "";
        }
        if (os_strstr(param3, "data", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_DEVDIR_DATA;
        if (os_strstr(param3, "buffers", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_DEVDIR_BUFFERS;

        devicedir_memory_blocks(self->root, stream, param2, flags);
    }
    else if (!os_strcmp(param1, "signals"))
    {
        devicedir_dynamic_signals(self->root, stream, param2, 0);
    }

    osal_stream_write(stream, "\0", 1, &n, OSAL_STREAM_DEFAULT);
    p = osal_stream_buffer_content(stream, &n);
    rval = PyUnicode_FromString(p);
    osal_stream_close(stream);
    return rval;
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
    {"print", (PyCFunction)Root_print, METH_VARARGS, "Print internal state of IOCOM"},
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
