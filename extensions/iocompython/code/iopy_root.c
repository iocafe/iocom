/**

  @file    iopy_root.c
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


typedef enum
{
    IOC_SEND_MBLK,
    IOC_RECEIVE_MBLK,
}
iocSendReceiveOp;


static void Root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,    
    void *context);

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
        *network_name = NULL,
        *security = NULL,
        *password = NULL;

    int
        device_nr = IOC_AUTO_DEVICE_NR;

    static char *kwlist[] = {
        "device_name",
        "device_nr",
        "network_name",
        "security",
        "password",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sisss",
         kwlist, &device_name, &device_nr, &network_name, &security, &password))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    self = (Root *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    iocom_python_initialize(security);

    /* Allocate and initialize communication root and dymanic structure data root objects.
     * This demo uses dynamic signal configuration.
     */
    self->root = (iocRoot*)os_malloc(sizeof(iocRoot), OS_NULL);
    ioc_initialize_root(self->root);
    ioc_set_iodevice_id(self->root, device_name, device_nr, password, network_name);
    ioc_initialize_dynamic_root(self->root);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(self->root, Root_callback, self);

    /* Save network and device.
     */
    os_strncpy(self->network_name, network_name, IOC_NETWORK_NAME_SZ);
    os_strncpy(self->device_name, device_name, IOC_NAME_SZ);
    self->device_nr = device_nr;

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
        /* Free the root data structure and everything that belong to it.
         */
        ioc_release_root(self->root);
        self->root = OS_NULL;

        /* Delete communication wait event. This MUST be deleted after root.
         */
        if (self->queue_event)
        {
            osal_event_delete(self->queue_event);
            self->queue_event = OS_NULL;
        }

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
  List IO device networks.
****************************************************************************************************
*/
static PyObject *Root_list_networks(
    Root *self,
    PyObject *args)
{
    iocRoot *root;
    iocDynamicRoot *droot;
    iocDynamicNetwork *dnetwork;
    PyObject *pynetname, *rval;
    const char *reserved = OS_NULL;
    os_int i;

    root = self->root;
    if (root == OS_NULL)
    {
        PyErr_SetString(iocomError, "no IOCOM root object");
        return NULL;
    }
    droot = root->droot;
    if (droot == OS_NULL)
    {
        PyErr_SetString(iocomError, "no dynamic objects");
        Py_RETURN_NONE;
    }

    if (!PyArg_ParseTuple(args, "|s", &reserved))
    {
        PyErr_SetString(iocomError, "xxx");
        return NULL;
    }

    ioc_lock(root);

    rval = PyList_New(0);

    for (i = 0; i < IOC_DROOT_HASH_TAB_SZ; i++)
    {
        for (dnetwork = droot->hash[i];
             dnetwork;
             dnetwork = dnetwork->next)
        {
            pynetname = PyUnicode_FromString(dnetwork->network_name);
            PyList_Append(rval, pynetname);
            Py_DECREF(pynetname);
        }
    }

    ioc_unlock(root);
    return rval;
}

/**
****************************************************************************************************
  List devices in specific network.
****************************************************************************************************
*/
static PyObject *Root_list_devices(
    Root *self,
    PyObject *args)
{
    iocRoot *root;
    iocDynamicRoot *droot;
    iocDynamicNetwork *dnetwork;
    iocMblkShortcut *shortcut;
    iocMemoryBlock *mblk;
    PyObject *pydevname, *rval;
    const char *network_name = OS_NULL;
    os_char device_name[IOC_NAME_SZ + 8]; /* +8 for device number */
    os_char nbuf[OSAL_NBUF_SZ];

    root = self->root;
    if (root == OS_NULL)
    {
        PyErr_SetString(iocomError, "no IOCOM root object");
        return NULL;
    }
    droot = root->droot;
    if (droot == OS_NULL)
    {
        PyErr_SetString(iocomError, "no dynamic objects");
        Py_RETURN_NONE;
    }

    if (!PyArg_ParseTuple(args, "s", &network_name))
    {
        PyErr_SetString(iocomError, "Network name is needed as an argument");
        return NULL;
    }

    ioc_lock(root);
    dnetwork = ioc_find_dynamic_network(droot, network_name);
    if (dnetwork == OS_NULL)
    {
        ioc_unlock(root);
        Py_RETURN_NONE;
    }

    rval = PyList_New(0);
    for (shortcut = dnetwork->mlist_first;
         shortcut;
         shortcut = shortcut->next)
    {
        mblk = shortcut->mblk_handle.mblk;
        if (mblk == OS_NULL) continue;
        if (os_strcmp(mblk->mblk_name, "info")) continue;

        os_strncpy(device_name, mblk->device_name, sizeof(device_name));
        osal_int_to_str(nbuf, sizeof(nbuf), mblk->device_nr);
        os_strncat(device_name, nbuf, sizeof(device_name));

        pydevname = PyUnicode_FromString(device_name);
        PyList_Append(rval, pydevname);
        Py_DECREF(pydevname);
    }

    ioc_unlock(root);
    return rval;
}


/**
****************************************************************************************************

  @brief Start queueing IO network, device and memory block connects and disconnects.

  The Root_initialize_event_queue function sets up queue for connect/disconnect, etc. events.
  This keeps the Python application informed about the events.

****************************************************************************************************
*/
static PyObject *Root_initialize_event_queue(
    Root *self,
    PyObject *args,
    PyObject *kwds)
{
    iocRoot *root;
    osalEvent event;
    char *py_flags = OS_NULL;
    int py_max_events = 0;
    os_int flags;

    static char *kwlist[] = {
        "flags",
        "n",
        NULL
    };

    root = self->root;
    if (root == OS_NULL)
    {
        PyErr_SetString(iocomError, "no IOCOM root object");
        return NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|si",
         kwlist, &py_flags, &py_max_events))
    {
        PyErr_SetString(PyExc_TypeError, "parsing argument failed");
        return NULL;
    }

    event = osal_event_create();
    self->queue_event = event;

    flags = 0;
    if (py_flags)
    {
        if (os_strstr(py_flags, "new_mblk", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_NEW_MBLK_EVENTS;

        if (os_strstr(py_flags, "connect_mblk", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_MBLK_CONNECT_EVENTS;

        if (os_strstr(py_flags, "all_mblk", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_ALL_MBLK_EVENTS;

        if (os_strstr(py_flags, "device", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_DEVICE_EVENTS;

        if (os_strstr(py_flags, "network", OSAL_STRING_SEARCH_ITEM_NAME))
            flags |= IOC_NETWORK_EVENTS;
    }

    /* Default to receiving network and device evnts
     */
    if (flags == 0)
    {
        flags = IOC_DEVICE_EVENTS|IOC_NETWORK_EVENTS|IOC_NEW_MBLK_EVENTS;
    }

    ioc_initialize_event_queue(root, event, py_max_events, flags);

    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}

/**
****************************************************************************************************

  @brief Interrupt IOCOM communication queue "wait for event".

  The Root_trig_event() function...
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *Root_interrupt_wait(
    Root *self)
{
    if (self->queue_event)
    {
        osal_event_set(self->queue_event);
    }

    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Wait for network, device and memory block connect/disconnects, etc event.

  The Root_wait_for_com_event function waits for communication event for given amount of time
  If no event, the function returns None.

****************************************************************************************************
*/
static PyObject *Root_wait_for_com_event(
    Root *self,
    PyObject *args)
{
    iocRoot *root;
    PyObject *rval;
    int timeout_ms;
    iocQueuedEvent *e;
    os_char device_name[IOC_NAME_SZ+8];
    os_char nbuf[OSAL_NBUF_SZ], *event_name;

    root = self->root;
    if (root == OS_NULL)
    {
        PyErr_SetString(iocomError, "no IOCOM root object");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "|i", &timeout_ms))
    {
        PyErr_SetString(PyExc_TypeError, "parsing argument failed");
        return NULL;
    }

    if (self->queue_event == OS_NULL)
    {
        PyErr_SetString(PyExc_TypeError, "Communication events are not queues, call queue_events()");
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    osal_event_wait(self->queue_event, timeout_ms);
    Py_END_ALLOW_THREADS

    e = ioc_get_event(root);

    if (e)
    {
        rval = PyList_New(4);
        switch (e->event)
        {
            case IOC_NEW_MEMORY_BLOCK:
                event_name = "new_mblk";
                break;

            case IOC_MBLK_CONNECTED_AS_SOURCE:
                event_name = "mblk_as_source";
                break;

            case IOC_MBLK_CONNECTED_AS_TARGET:
                event_name = "mblk_as_target";
                break;

            case IOC_MEMORY_BLOCK_DELETED:
                event_name = "mblk_deleted";
                break;

            case IOC_NEW_NETWORK:
                event_name = "new_network";
                break;

            case IOC_NETWORK_DISCONNECTED:
                event_name = "network_disconnected";
                break;

            case IOC_NEW_DEVICE:
                event_name = "new_device";
                break;

            case IOC_DEVICE_DISCONNECTED:
                event_name = "device_disconnected";
                break;

            default:
                event_name = "unknown";
                break;
        }

        PyList_SetItem(rval, 0, Py_BuildValue("s", (char *)event_name));
        PyList_SetItem(rval, 1, Py_BuildValue("s", (char *)e->network_name));

        osal_int_to_str(nbuf, sizeof(nbuf), e->device_nr);
        os_strncpy(device_name, e->device_name, sizeof(device_name));
        os_strncat(device_name, nbuf, sizeof(device_name));
        PyList_SetItem(rval, 2, Py_BuildValue("s", (char *)device_name));
        PyList_SetItem(rval, 3, Py_BuildValue("s", (char *)e->mblk_name));

        ioc_pop_event(root);
        return rval;
    }

    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/**
****************************************************************************************************

  @brief Callback when dynamic IO network, device, etc has been connected or disconnected.

  The info_Root_callback() function is called when memory block, io device network or io device is
  added or removed.

  @param   root Pointer to the root object.
  @param   event Either IOC_NEW_NETWORK, IOC_NEW_DEVICE or IOC_NETWORK_DISCONNECTED.
  @param   dnetwork Pointer to dynamic network object which has just been connected or is
           about to be removed.
  @param   mblk Pointer to memory block structure, OS_NULL if not available for the event.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void Root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,    
    void *context)
{
    os_char *mblk_name; 
    iocHandle handle;

    switch (event)
    {
        /* Process "new dynamic memory block" callback.
         */
        case IOC_NEW_MEMORY_BLOCK:
            mblk_name = mblk->mblk_name;

            if (!os_strcmp(mblk_name, "info"))
            {
                ioc_setup_handle(&handle, root, mblk);
                ioc_add_callback(&handle, Root_info_callback, OS_NULL);
                ioc_release_handle(&handle);
            }
            break;

        default:
            break;
    }
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
        ioc_add_dynamic_info(handle, OS_FALSE);
    }
}


/**
****************************************************************************************************

  @brief Set memory block parameter value.
  @anchor MemoryBlock_set_param

  The MemoryBlock.set_param() function gets value of memory block's parameter.

  param_name Currently only "auto" can be set. Controlles wether to use automatic (value 1)
  or synchronous sending/receiving (value 0).

****************************************************************************************************
*/
static PyObject *Root_set_mblk_param(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
    const char *mblk_path = NULL;
    const char *param_name = OS_NULL;
    int param_value = 0;
    iocMemoryBlockParamIx param_ix;

    Root *root;

    iocRoot *iocroot;
    iocIdentifiers identifiers;
    iocDynamicNetwork *dnetwork;
    iocHandle *handle;

    static char *kwlist[] = {
        "io_path",
        "param",
        "value",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ssi",
         kwlist, &mblk_path, &param_name, &param_value))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    root = (Root*)self;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        return NULL;
    }

    if (!os_strcmp(param_name, "auto"))
    {
        param_ix = IOC_MBLK_AUTO_SYNC_FLAG;
    }
    else
    {
        PyErr_SetString(iocomError, "Unknown parameter");
        return NULL;
    }

    ioc_lock(iocroot);

    ioc_iopath_to_identifiers(iocroot, &identifiers, mblk_path, IOC_EXPECT_MEMORY_BLOCK);
    dnetwork = ioc_find_dynamic_network(iocroot->droot, identifiers.network_name);
    handle = ioc_find_mblk_shortcut(dnetwork, identifiers.mblk_name,
        identifiers.device_name, identifiers.device_nr);

    if (handle == OS_NULL)
    {

        osal_trace("Warning: Memory block was not found for parameter setting (may be just deleted)");
    }
    else
    {
        ioc_memory_block_set_int_param(handle, param_ix, param_value);
    }
    ioc_unlock(iocroot);

    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Synchronized data transfers, sen and receive.

****************************************************************************************************
*/
static PyObject *Root_send_receive(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds,
    iocSendReceiveOp op)
{
    const char *mblk_path = NULL;
    Root *root;

    iocRoot *iocroot;
    iocIdentifiers identifiers;
    iocDynamicNetwork *dnetwork;
    iocMblkShortcut *item, *next_item;
    iocMemoryBlock *mblk;

    static char *kwlist[] = {
        "io_path",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s",
         kwlist, &mblk_path))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    root = (Root*)self;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        return NULL;
    }

    ioc_lock(iocroot);

    ioc_iopath_to_identifiers(iocroot, &identifiers, mblk_path, IOC_EXPECT_DEVICE);
    dnetwork = ioc_find_dynamic_network(iocroot->droot, identifiers.network_name);

    if (dnetwork == OS_NULL)
    {
        osal_trace("Warning, send/receive: Network was not found");
    }
    else
    {
        for (item = dnetwork->mlist_first;
             item;
             item = next_item)
        {
            next_item = item->next;

            /* Clean up memory while searching
             */
            mblk = item->mblk_handle.mblk;
            if (mblk == OS_NULL)
            {
                ioc_release_mblk_shortcut(dnetwork, item);
            }
            else
            {
                if (identifiers.device_nr == mblk->device_nr)
                {
                    if (!os_strcmp(identifiers.device_name, mblk->device_name))
                    {
                        switch (op)
                        {
                            case IOC_SEND_MBLK:
                                ioc_send(&item->mblk_handle);
                                break;

                            default:
                            case IOC_RECEIVE_MBLK:
                                ioc_receive(&item->mblk_handle);
                                break;
                        }
                    }
                }
            }
        }
    }

    ioc_unlock(iocroot);

    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Call synchronized send.

****************************************************************************************************
*/
static PyObject *Root_send(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
    return Root_send_receive(self, args, kwds, IOC_SEND_MBLK);
}

/**
****************************************************************************************************

  @brief Call synchronized receive.

****************************************************************************************************
*/
static PyObject *Root_receive(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
    return Root_send_receive(self, args, kwds, IOC_RECEIVE_MBLK);
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
    osal_stream_close(stream, OSAL_STREAM_DEFAULT);
    return rval;
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef Root_members[] = {
    {(char*)"status", T_INT, offsetof(Root, status), 0, (char*)"Status code"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef Root_methods[] = {
    {"delete", (PyCFunction)Root_delete, METH_NOARGS, "Delete IOCOM root object"},
    {"queue_events", (PyCFunction)Root_initialize_event_queue, METH_VARARGS|METH_KEYWORDS,
        "Start queueing connect/disconnect, etc. events"},
    {"wait_com_event", (PyCFunction)Root_wait_for_com_event, METH_VARARGS, "Wait for a communication event"},
    {"interrupt_wait", (PyCFunction)Root_interrupt_wait, METH_NOARGS, "Interrupt \'wait for communication event\'"},
    {"list_networks", (PyCFunction)Root_list_networks, METH_VARARGS, "List IO device networks"},
    {"list_devices", (PyCFunction)Root_list_devices, METH_VARARGS, "List devices in spefified network"},
    {"set_mblk_param", (PyCFunction)Root_set_mblk_param, METH_VARARGS|METH_KEYWORDS, "Set memory block parameter"},
    {"send", (PyCFunction)Root_send, METH_VARARGS|METH_KEYWORDS, "Send data synchronously"},
    {"receive", (PyCFunction)Root_receive, METH_VARARGS|METH_KEYWORDS, "Receive data synchronously"},
    {"print", (PyCFunction)Root_print, METH_VARARGS, "Print internal state of IOCOM"},

    {"getconf", (PyCFunction)iocom_stream_getconf, METH_VARARGS|METH_KEYWORDS, "Read configuration."},
    {"setconf", (PyCFunction)iocom_stream_setconf, METH_VARARGS|METH_KEYWORDS, "Write configuration."},
    {"initconf", (PyCFunction)iocom_initconf, METH_VARARGS|METH_KEYWORDS, "Initialize conf stream signals."},

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
