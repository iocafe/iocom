/**

  @file    iopy_stream.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Streaming data trough memory block.

  ** SIMPLE BLOCKING CALL
  from iocompython import Root
  print(root.getconf(device_name + "." + network_name))
  print(root.setconf(device_name + "." + network_name, str.encode("Dummy config data")))

  ** USING STREAM OBJECT
  from iocompython import Root, EndPoint, Signal, Stream, json2bin
  import ioterminal
  import time

  def get_network_conf(device_name, network_name):
    global root, callback_queue

    exp_mblk_path = 'conf_exp.' + device_name + '.' + network_name
    imp_mblk_path = 'conf_imp.' + device_name + '.' + network_name

    stream = Stream(root, frd = "frd_buf", tod = "tod_buf", exp = exp_mblk_path, imp = imp_mblk_path, select = 2)
    stream.start_read()

    while True:
        s = stream.run()
        if s != None:
            break
        time.sleep(0.01)

    if s == 'completed':
        data = stream.get_data();
        print(data)

    else:
        print(s)

    stream.delete()


  def set_network_conf(device_name, network_name):
    global root, callback_queue

    exp_mblk_path = 'conf_exp.' + device_name + '.' + network_name
    imp_mblk_path = 'conf_imp.' + device_name + '.' + network_name

    stream = Stream(root, frd = "frd_buf", tod = "tod_buf", exp = exp_mblk_path, imp = imp_mblk_path, select = 2)

    my_conf_bytes = str.encode("My dummy network configuration string")
    stream.start_write(my_conf_bytes)

    while True:
        s = stream.run()
        if s != None:
            break
        time.sleep(0.01)

    if s == 'completed':
        print("success")

    else:
        print(s)

    stream.delete()

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocompython.h"


/**
****************************************************************************************************
  Constructor creates a new stream object.
****************************************************************************************************
*/
static PyObject *Stream_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    Stream *self;
    PyObject *pyroot = NULL;
    Root *root;
    iocRoot *iocroot;

    const char
        *frd_buf_name = "frd_buf",
        *tod_buf_name = "tod_buf",
        *exp_mblk_path = NULL,
        *imp_mblk_path = NULL,
        *flags = osal_str_empty;

    int
        select = 0;

    static char *kwlist[] = {
        "root",
        "frd",
        "tod",
        "exp",
        "imp",
        "select",
        "flags",
        NULL
    };

    self = (Stream *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }
    self->stream = OS_NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ssssis",
         kwlist, &pyroot, &frd_buf_name, &tod_buf_name,
        &exp_mblk_path, &imp_mblk_path, &select, &flags))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
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

    if (exp_mblk_path == OS_NULL || imp_mblk_path == OS_NULL)
    {
        PyErr_SetString(iocomError, "no imp or exp memory block path");
        goto failed;
    }

    self->stream = ioc_open_stream(
        iocroot, select, frd_buf_name, tod_buf_name, exp_mblk_path, imp_mblk_path,
        OS_NULL, 0, OS_NULL,
        os_strstr(flags, "device", OSAL_STRING_SEARCH_ITEM_NAME)
        ? IOC_IS_DEVICE : IOC_IS_CONTROLLER);

    self->status = OSAL_SUCCESS;

    /* Save root pointer and increment reference count.
     */
    self->pyroot = root;
    Py_INCREF(root);

    return (PyObject *)self;

failed:
    Py_TYPE(self)->tp_free((PyObject *)self);
    return NULL;
}


/**
****************************************************************************************************
  Internal cleanup function called by Stream_delete and destructor.
****************************************************************************************************
*/
static void Stream_close_stremer(
    Stream *self)
{
    if (self->stream)
    {
        ioc_release_stream(self->stream);
        self->stream = OS_NULL;
    }
}


/**
****************************************************************************************************
  DestructorStream_dealloc function releases Python object and IOCOM stream associated to it.
****************************************************************************************************
*/
static void Stream_dealloc(
    Stream *self)
{
    if (self->pyroot)
    {
        Stream_close_stremer(self);
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("Stream.dealloc()\n");
#endif
}


/**
****************************************************************************************************
  The "delete" function closes the stream and releases any ressources for it.
****************************************************************************************************
*/
static PyObject *Stream_delete(
    Stream *self)
{
    if (self->pyroot)
    {
        Stream_close_stremer(self);
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("Stream.delete()\n");
#endif
    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/**
****************************************************************************************************
  The "start_write" function prepares to start writing data to stream.
****************************************************************************************************
*/
static PyObject *Stream_start_write(
    Stream *self,
    PyObject *args,
    PyObject *kwds)
{
    PyObject *pydata = NULL;
    char *buffer;
    Py_ssize_t length;
    int count = -1;
    int pos = 0;

    static char *kwlist[] = {
        "data",
        "pos",
        "n",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ii",
         kwlist, &pydata, &pos, &count))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    PyBytes_AsStringAndSize(pydata, &buffer, &length);
    if (count < 0) count = (int)length;
    if (pos + count > length) count = (int)(length - pos);
    if (count < 0) count = 0;
    ioc_start_stream_write(self->stream, buffer + pos, count, OS_TRUE);

    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  The "start_read" function prepares to start reading data from stream.
****************************************************************************************************
*/
static PyObject *Stream_start_read(
    Stream *self)
{
    ioc_start_stream_read(self->stream);
    Py_RETURN_NONE;
}


/**
****************************************************************************************************
  The "run" function does actual data transfer. Call until function returns something but None.
****************************************************************************************************
*/
static PyObject *Stream_run(
    Stream *self)
{
    osalStatus s;
    s = ioc_run_stream(self->stream, IOC_CALL_SYNC);

    switch (s)
    {
        case OSAL_SUCCESS:
            Py_RETURN_NONE;

        case OSAL_COMPLETED:
            return Py_BuildValue("s", "completed");

        default:
            return Py_BuildValue("s", "failed");
    }
}


/**
****************************************************************************************************
  The "get_data" function returns received data as bytes object.
****************************************************************************************************
*/
static PyObject *Stream_get_data(
    Stream *self)
{
    const os_char *data;
    os_memsz sz;

    data = ioc_get_stream_data(self->stream, &sz, 0);
    if (data == NULL) data = osal_str_empty;
    return PyBytes_FromStringAndSize(data, sz);
}


/**
****************************************************************************************************
  The "bytes_moved" function returns how much data has been moved trough stream, this is
  useful for scroll bars, etc. Returns -1 if stream is not open.
****************************************************************************************************
*/
static PyObject *Stream_bytes_moved(
    Stream *self)
{
    os_memsz bytes;

    if (self->stream) {
        bytes = ioc_stream_nro_bytes_moved(self->stream);
    }
    else {
        bytes = -1;
    }

    return Py_BuildValue("i", (int)bytes);
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef Stream_members[] = {
    {(char*)"status", T_INT, offsetof(Stream, status), 0, (char*)"constructor status"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef Stream_methods[] = {
    {"delete", (PyCFunction)Stream_delete, METH_NOARGS, "Deletes streamer"},
    {"start_write", (PyCFunction)Stream_start_write, METH_VARARGS|METH_KEYWORDS, "Start write"},
    {"start_read", (PyCFunction)Stream_start_read, METH_NOARGS, "Start read"},
    {"run", (PyCFunction)Stream_run, METH_NOARGS, "Transfer the data"},
    {"get_data", (PyCFunction)Stream_get_data, METH_NOARGS, "Get received data"},
    {"bytes_moved", (PyCFunction)Stream_bytes_moved, METH_NOARGS, "Get number of transferred bytes"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject StreamType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".Stream",  /* tp_name */
    sizeof(Stream),                           /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)Stream_dealloc,               /* tp_dealloc */
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
    "Stream objects",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Stream_methods,                           /* tp_methods */
    Stream_members,                           /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    Stream_new,                               /* tp_new */
};


/**
****************************************************************************************************
  Global blocking function, get configuration.

  device_path = 'gina3.iocafenet'
  file_content = ioc_root.getconf(device_path, select=7)
  if file_content == None:
    print('download fails')

****************************************************************************************************
*/
PyObject *iocom_stream_getconf(
    PyObject *self,
    PyObject *args,
    PyObject *kwds)
{
    const char
        *frd_buf_name = "frd_buf",
        *tod_buf_name = "tod_buf",
        *device_path = OS_NULL,
        *flags = osal_str_empty;

    int
        select = OS_PBNR_CONFIG;

    os_char
        exp_mblk_path[IOC_MBLK_PATH_SZ],
        imp_mblk_path[IOC_MBLK_PATH_SZ];

    const os_char
        *data;

    os_memsz sz;
    osalStatus s;
    PyObject *rval;
    iocStream *stream;
    Root *root;
    iocRoot *iocroot;

    static char *kwlist[] = {
        "path",
        "select",
        "flags",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|is",
         kwlist, &device_path, &select, &flags))
    {
        PyErr_SetString(iocomError, "Device path mydevice.mynetwork is expected as argument.");
        return NULL;
    }

    root = (Root*)self;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        return NULL;
    }

    os_strncpy(exp_mblk_path, "conf_exp.", sizeof(exp_mblk_path));
    os_strncat(exp_mblk_path, device_path, sizeof(exp_mblk_path));
    os_strncpy(imp_mblk_path, "conf_imp.", sizeof(imp_mblk_path));
    os_strncat(imp_mblk_path, device_path, sizeof(imp_mblk_path));

    stream = ioc_open_stream(
        iocroot, select, frd_buf_name, tod_buf_name, exp_mblk_path, imp_mblk_path,
        OS_NULL, 0, OS_NULL, os_strstr(flags, "device",
        OSAL_STRING_SEARCH_ITEM_NAME) ?  IOC_IS_DEVICE : IOC_IS_CONTROLLER);

    ioc_start_stream_read(stream);

    while ((s = ioc_run_stream(stream, IOC_CALL_SYNC)) == OSAL_SUCCESS && osal_go())
    {
        Py_BEGIN_ALLOW_THREADS
        os_timeslice();
        Py_END_ALLOW_THREADS
    }

    if (s == OSAL_COMPLETED)
    {
        data = ioc_get_stream_data(stream, &sz, 0);
        if (data == NULL) data = osal_str_empty;
        rval = PyBytes_FromStringAndSize(data, sz);
    }
    else
    {
        Py_INCREF(Py_None);
        rval = Py_None;
    }

    ioc_release_stream(stream);

    return rval;
}


/**
****************************************************************************************************
  Global blocking function, set configuration.
****************************************************************************************************
*/
PyObject *iocom_stream_setconf(
    PyObject *self,
    PyObject *args,
    PyObject *kwds)
{
    const char
        *frd_buf_name = "frd_buf",
        *tod_buf_name = "tod_buf",
        *device_path = OS_NULL,
        *flags = osal_str_empty;

    int
        select = OS_PBNR_CONFIG;

    os_char
        exp_mblk_path[IOC_MBLK_PATH_SZ],
        imp_mblk_path[IOC_MBLK_PATH_SZ];

    osalStatus s;
    iocStream *stream;
    Root *root;
    iocRoot *iocroot;
    PyObject *pydata = NULL;
    char *buffer;
    Py_ssize_t length;
    int count = -1;
    int pos = 0;

    static char *kwlist[] = {
        "path",
        "data",
        "pos",
        "n",
        "select",
        "flags",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO|iiis",
         kwlist, &device_path, &pydata, &pos, &count, &select, &flags))
    {
        PyErr_SetString(iocomError, "Device path mydevice.mynetwork and byte data to send are expected as arguments.");
        return NULL;
    }

    root = (Root*)self;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        return NULL;
    }

    os_strncpy(exp_mblk_path, "conf_exp.", sizeof(exp_mblk_path));
    os_strncat(exp_mblk_path, device_path, sizeof(exp_mblk_path));
    os_strncpy(imp_mblk_path, "conf_imp.", sizeof(imp_mblk_path));
    os_strncat(imp_mblk_path, device_path, sizeof(imp_mblk_path));

    stream = ioc_open_stream(
        iocroot, select, frd_buf_name, tod_buf_name, exp_mblk_path, imp_mblk_path,
        OS_NULL, 0, OS_NULL, os_strstr(flags, "device",
        OSAL_STRING_SEARCH_ITEM_NAME) ?  IOC_IS_DEVICE : IOC_IS_CONTROLLER);

    PyBytes_AsStringAndSize(pydata, &buffer, &length);
    if (count < 0) count = (int)length;
    if (pos + count > length) count = (int)length - pos;
    if (count < 0) count = 0;
    ioc_start_stream_write(stream, buffer + pos, count, OS_TRUE);

    while ((s = ioc_run_stream(stream, IOC_CALL_SYNC)) == OSAL_SUCCESS && osal_go())
    {
        Py_BEGIN_ALLOW_THREADS
        os_timeslice();
        Py_END_ALLOW_THREADS
    }

    ioc_release_stream(stream);
    return Py_BuildValue("s", s == OSAL_COMPLETED ? "completed" : "failed");
}


/**
****************************************************************************************************
  Initialize configuration stream signals.
****************************************************************************************************
*/
PyObject *iocom_initconf(
    PyObject *self,
    PyObject *args,
    PyObject *kwds)
{
    const char
        *frd_buf_name = "frd_buf",
        *tod_buf_name = "tod_buf",
        *device_path = OS_NULL,
        *flags = osal_str_empty;

    os_char
        exp_mblk_path[IOC_MBLK_PATH_SZ],
        imp_mblk_path[IOC_MBLK_PATH_SZ];

    iocStream *stream;
    Root *root;
    iocRoot *iocroot;

    static char *kwlist[] = {
        "path",
        "flags",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss",
         kwlist, &device_path, &flags))
    {
        PyErr_SetString(iocomError, "Device path and flags are expected as arguments.");
        return NULL;
    }

    root = (Root*)self;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        return NULL;
    }

    os_strncpy(exp_mblk_path, "conf_exp.", sizeof(exp_mblk_path));
    os_strncat(exp_mblk_path, device_path, sizeof(exp_mblk_path));
    os_strncpy(imp_mblk_path, "conf_imp.", sizeof(imp_mblk_path));
    os_strncat(imp_mblk_path, device_path, sizeof(imp_mblk_path));

    stream = ioc_open_stream(
        iocroot, 0, frd_buf_name, tod_buf_name, exp_mblk_path, imp_mblk_path,
        OS_NULL, 0, OS_NULL, os_strstr(flags, "device",
        OSAL_STRING_SEARCH_ITEM_NAME) ?  IOC_IS_DEVICE : IOC_IS_CONTROLLER);

    ioc_stream_initconf(stream, IOC_CALL_SYNC);
    ioc_release_stream(stream);

    Py_RETURN_NONE;
}
