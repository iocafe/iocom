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

/* Used to store data parsed from arguments.
 */
#define IOPY_FIXBUF_SZ 64
typedef struct
{
    iocSignal *signal;

    osalTypeId type_id;
    os_boolean is_array;
    os_boolean is_string;

    os_int n_values;
    os_int max_values;

    os_char *buf;

    union
    {
        os_char fixbuf[IOPY_FIXBUF_SZ];
        iocValue vv;
    }
    storage;
}
SignalSetParseState;


/* Used to store data parsed from arguments.
 */
typedef struct
{
    iocSignal *signal;
    osalTypeId type_id;
    os_int max_values;
}
SignalGetState;



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

    self->status = OSAL_SUCCESS;

#if IOPYTHON_TRACE
    PySys_WriteStdout("Signal.new(%s, %s)\n", io_path, network_name);
#endif

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

    if (self->pyroot)
    {
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

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

    if (self->pyroot)
    {
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("Signal.delete()\n");
#endif
    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/**
****************************************************************************************************

  @brief Store long integer parsed from Python arguments in format expected for the signal.
  @anchor Signal_store_double

  @return  None.

****************************************************************************************************
*/
static void Signal_store_long(
    os_long x,
    SignalSetParseState *state)
{
    os_char *p;
    os_int i;

    if (state->n_values >= state->max_values)
    {
        return;
    }

    if (state->is_array)
    {
//        PySys_WriteStdout("TYPE: %d\n", (int)state->type_id); // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        p = state->buf;
        i = state->n_values;
        switch (state->type_id)
        {
            case OS_BOOLEAN:
            case OS_CHAR:
            case OS_UCHAR:
                p[i] = (os_char)x;
                break;

            case OS_SHORT:
            case OS_USHORT:
                ((os_short*)p)[i] = (os_short)x;
                break;

            case OS_INT:
            case OS_UINT:
                ((os_int*)p)[i] = (os_int)x;
                break;

            case OS_INT64:
            case OS_LONG:
                ((os_long*)p)[i] = x;
                break;

            case OS_FLOAT:
                ((os_float*)p)[i] = (os_float)x;
                break;

            case OS_DOUBLE:
                ((os_double*)p)[i] = (os_double)x;
                break;

            default:
                return;
        }
    }
    else
    {
//        PySys_WriteStdout("TYPE II: %d\n", (int)state->type_id); // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        switch (state->type_id)
        {
            case OS_BOOLEAN:
            case OS_CHAR:
            case OS_UCHAR:
            case OS_SHORT:
            case OS_USHORT:
            case OS_INT:
            case OS_UINT:
                state->storage.vv.value.i = (os_int)x;
                break;

            case OS_INT64:
            case OS_LONG:
                state->storage.vv.value.l = x;
                break;

            case OS_FLOAT:
                state->storage.vv.value.f = (os_float)x;
                break;

            case OS_DOUBLE:
                state->storage.vv.value.d = (os_double)x;
                break;

            default:
                return;
        }
    }

    state->n_values++;
}


/**
****************************************************************************************************

  @brief Store double parsed from Python arguments in format expected for the signal.
  @anchor Signal_store_double

  @return  None.

****************************************************************************************************
*/
static void Signal_store_double(
    os_double x,
    SignalSetParseState *state)
{
    os_char *p;
    os_int i;

    if (state->n_values >= state->max_values) return;

    if (state->is_array)
    {
        p = state->buf;
        i = state->n_values;
        switch (state->type_id)
        {
            case OS_BOOLEAN:
            case OS_CHAR:
            case OS_UCHAR:
                p[i] = (os_char)os_round_short(x);
                break;

            case OS_SHORT:
            case OS_USHORT:
                ((os_short*)p)[i] = os_round_short(x);
                break;

            case OS_INT:
            case OS_UINT:
                ((os_int*)p)[i] = os_round_int(x);
                break;

            case OS_INT64:
            case OS_LONG:
                ((os_long*)p)[i] = os_round_long(x);
                break;

            case OS_FLOAT:
                ((os_float*)p)[i] = (os_float)x;
                break;

            case OS_DOUBLE:
                ((os_double*)p)[i] = x;
                break;

            default:
                return;
        }
    }

    else
    {
//        PySys_WriteStdout("TYPE II: %d\n", (int)state->type_id); // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        switch (state->type_id)
        {
            case OS_BOOLEAN:
            case OS_CHAR:
            case OS_UCHAR:
            case OS_SHORT:
            case OS_USHORT:
            case OS_INT:
            case OS_UINT:
                state->storage.vv.value.i = os_round_int(x);
                break;

            case OS_INT64:
            case OS_LONG:
                state->storage.vv.value.l = os_round_long(x);
                break;

            case OS_FLOAT:
                state->storage.vv.value.f = (os_float)x;
                break;

            case OS_DOUBLE:
                state->storage.vv.value.d = x;
                break;

            default:
                return;
        }
    }

    state->n_values++;
}




/**
****************************************************************************************************

  @brief Parse python arguments.
  @anchor Signal_set_sequence

  Idea here is to parse python arguments, how ever structured in python call, to format needed
  for storing value(s) into register map.

  - String signal, as one string argument.
  - Array, as array of numbers
  - Single value

  @return  None.

****************************************************************************************************
*/
static void Signal_set_sequence(
    PyObject *args,
    SignalSetParseState *state)
{
    PyObject *a, *py_repr, *py_str;
    os_char value_str[128];

    long i, length, l;
    const os_char *str;

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

            os_strncpy(value_str, str, sizeof(value_str));
//            PySys_WriteStdout("String: %s\n", str);
            Py_XDECREF(py_repr);
            Py_XDECREF(py_str);
        }

        else if (PyLong_Check(a)) {
            l = PyLong_AsLong(a);
            Signal_store_long(l, state);
//            PySys_WriteStdout("Long %d\n", (int)l);
        }

        else if (PyFloat_Check(a)) {
            double d = PyFloat_AsDouble(a);
            Signal_store_double(d, state);
//            PySys_WriteStdout("Float %f\n", d);
        }

        else if (PySequence_Check(a))
        {
//            PySys_WriteStdout("Sequence\n");

            Signal_set_sequence(a, state);

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
    }
}


/**
****************************************************************************************************

  @brief Set simple signal containing one numeric value.
  @anchor Signal_set_one_value

  @return  None.

****************************************************************************************************
*/
static void Signal_set_one_value(
    PyObject *args,
    SignalSetParseState *state)
{
    state->max_values = 1;

    /* Process the Python arguments into format we need.
     */
    Signal_set_sequence(args, state);

    if (state->n_values)
    {
        state->storage.vv.state_bits = OSAL_STATE_CONNECTED;
        ioc_movex_signals(state->signal, &state->storage.vv, 1,
            IOC_SIGNAL_WRITE|IOC_SIGNAL_NO_THREAD_SYNC);
//        PySys_WriteStdout("WRITING : %d\n", state->storage.vv.value.i); // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    }
}



/**
****************************************************************************************************

  @brief Set signal containing array of values.
  @anchor Signal_set_array

  @return  None.

****************************************************************************************************
*/
static void Signal_set_array(
    PyObject *args,
    SignalSetParseState *state)
{
    os_memsz buf_sz, type_sz;

    /* If we need more space than we have in small fixed buffer, allocate.
     */
    state->buf = state->storage.fixbuf;
    type_sz = osal_typeid_size(state->type_id);
    buf_sz = state->max_values * type_sz;
    if (buf_sz > IOPY_FIXBUF_SZ)
    {
        state->buf = os_malloc(buf_sz, OS_NULL);
        os_memclear(state->buf, buf_sz);
    }

    /* Process the Python arguments into format we need.
     */
    Signal_set_sequence(args, state);

    /* Write always all values in array, even if caller would provides fewer. Rest will be zeros.
     */
    ioc_movex_array_signal(state->signal, state->buf, state->max_values,
        OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|IOC_SIGNAL_NO_THREAD_SYNC);

    /* If we allocated extra buffer space, free it.
     */
    if (buf_sz > IOPY_FIXBUF_SZ)
    {
        os_free(state->buf, buf_sz);
    }
}


/**
****************************************************************************************************

  @brief Store signal value into memory block.
  @anchor Signal_set

  The Signal.set() function finds mathing dynamic information for this signal, and according
  to the information stores signal value: string, array or one numerical value into the memory
  block containing the signal.

****************************************************************************************************
*/
static PyObject *Signal_set(
    Signal *self,
    PyObject *args)
{
    iocRoot *iocroot;
    SignalSetParseState state;

    /* Get IOC root pointer. Check not to crash on exceptional situations.
     */
    if (self->pyroot == OS_NULL) goto getout;
    iocroot = self->pyroot->root;
    if (iocroot == OS_NULL) goto getout;
    ioc_lock(iocroot);

    /* Find dynamic information for this signal.
     */
    os_memclear(&state, sizeof(state));
    state.signal = &self->signal;
    ioc_setup_signal_by_identifiers(iocroot, &self->identifiers, state.signal);
    if (state.signal->handle->mblk == OS_NULL)
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    /* If the signal is string.
     */
    state.type_id = (state.signal->flags & OSAL_TYPEID_MASK);
    state.max_values = state.signal->n;
    if (state.type_id == OS_STR)
    {
        state.is_string = OS_TRUE;
    }

    /* If this signal is an array
     */
    if (state.max_values > 1)
    {
        state.is_array = OS_TRUE;
        Signal_set_array(args, &state);
    }

    /* Otherwise this is single value signal.
     */
    else
    {
        Signal_set_one_value(args, &state);
    }

    ioc_unlock(iocroot);

getout:
    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Get simple signal value.
  @anchor Signal_get_one_value

  @return  None.

****************************************************************************************************
*/
static PyObject *Signal_get_one_value(
    PyObject *args,
    SignalGetState *state)
{
    iocValue vv;
    PyObject *rval;

    ioc_movex_signals(state->signal, &vv, 1, IOC_SIGNAL_NO_THREAD_SYNC);

    rval = PyList_New(2);
    PyList_SetItem(rval, 0, Py_BuildValue("i", (int)vv.state_bits));

    switch (state->type_id)
    {
        case OS_BOOLEAN:
        case OS_CHAR:
        case OS_UCHAR:
        case OS_SHORT:
        case OS_USHORT:
        case OS_INT:
        case OS_UINT:
            PyList_SetItem(rval, 1, Py_BuildValue("i", (int)vv.value.i));
            break;

        case OS_INT64:
        case OS_LONG:
            PyList_SetItem(rval, 1, Py_BuildValue("l", (long long)vv.value.l));
            break;

        case OS_FLOAT:
            PyList_SetItem(rval, 1, Py_BuildValue("d", (double)vv.value.f));
            break;

        case OS_DOUBLE:
            PyList_SetItem(rval, 1, Py_BuildValue("d", (double)vv.value.d));
            break;

        default:
            break;
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get signal containing array of values.
  @anchor Signal_get_array

  @return  None.

****************************************************************************************************
*/
static PyObject *Signal_get_array(
    PyObject *args,
    SignalGetState *state)
{
    os_memsz buf_sz, type_sz;
    PyObject *rval, *list;
    os_int i;
    os_char state_bits, *buf;
    os_char fixbuf[64];

    /* If we need more space than we have in small fixed buffer, allocate.
     */
    buf = fixbuf;
    type_sz = osal_typeid_size(state->type_id);
    buf_sz = state->max_values * type_sz;
    if (buf_sz > sizeof(fixbuf))
    {
        buf = os_malloc(buf_sz, OS_NULL);
    }
    os_memclear(buf, buf_sz);

    /* Read values.
     */
    state_bits = ioc_movex_array_signal(state->signal, buf, state->max_values,
        OSAL_STATE_CONNECTED, IOC_SIGNAL_NO_THREAD_SYNC);

    list = PyList_New(state->max_values);
    for(i = 0; i < state->max_values; i++)
    {
        switch (state->type_id)
        {
            case OS_BOOLEAN:
            case OS_CHAR:
                PyList_SetItem(list, i, Py_BuildValue("i", (int)((os_char*)buf)[i]));
                break;

            case OS_UCHAR:
                PyList_SetItem(list, i, Py_BuildValue("i", (unsigned int)((os_uchar*)buf)[i]));
                break;

            case OS_SHORT:
                PyList_SetItem(list, i, Py_BuildValue("i", (int)((os_short*)buf)[i]));
                break;

            case OS_USHORT:
                PyList_SetItem(list, i, Py_BuildValue("i", (int)((os_ushort*)buf)[i]));
                break;

            case OS_INT:
                PyList_SetItem(list, i, Py_BuildValue("i", (int)((os_int*)buf)[i]));
                break;

            case OS_UINT:
                PyList_SetItem(list, i, Py_BuildValue("l", (unsigned long)((os_uint*)buf)[i]));
                break;

            case OS_INT64:
            case OS_LONG:
                PyList_SetItem(list, i, Py_BuildValue("l", (long long)((os_long*)buf)[i]));
                break;

            case OS_FLOAT:
                PyList_SetItem(list, i, Py_BuildValue("d", (double)((os_float*)buf)[i]));
                break;

            case OS_DOUBLE:
                PyList_SetItem(list, i, Py_BuildValue("d", (double)((os_double*)buf)[i]));
                break;

            default:
                PyList_SetItem(list, i, Py_BuildValue("i", (int)0));
                break;
        }
    }

    rval = PyList_New(2);
    PyList_SetItem(rval, 0, Py_BuildValue("i", (int)state_bits));
    PyList_SetItem(rval, 1, list);

    /* If we allocated extra buffer space, free it.
     */
    if (buf_sz > sizeof(fixbuf))
    {
        os_free(buf, buf_sz);
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get signal value from memory block.
  @anchor Signal_get

  The Signal.get() function finds mathing dynamic information for this signal, and according
  to the information reads signal value: string, array or one numerical value from the memory
  block and returns it.

****************************************************************************************************
*/
static PyObject *Signal_get(
    Signal *self,
    PyObject *args)
{
    iocRoot *iocroot;
    SignalGetState state;
    PyObject *rval = OS_NULL;

    /* Get IOC root pointer. Check not to crash on exceptional situations.
     */
    if (self->pyroot == OS_NULL) goto getout;
    iocroot = self->pyroot->root;
    if (iocroot == OS_NULL) goto getout;
    ioc_lock(iocroot);

    /* Find dynamic information for this signal.
     */
    os_memclear(&state, sizeof(state));
    state.signal = &self->signal;
    ioc_setup_signal_by_identifiers(iocroot, &self->identifiers, state.signal);
    if (state.signal->handle->mblk == OS_NULL)
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    /* If the signal is string.
     */
    state.type_id = (state.signal->flags & OSAL_TYPEID_MASK);
    state.max_values = state.signal->n;
    if (state.type_id == OS_STR)
    {
    }

    /* If this signal is an array
     */
    if (state.max_values > 1)
    {
        rval = Signal_get_array(args, &state);
    }

    /* Otherwise this is single value signal.
     */
    else
    {
        rval = Signal_get_one_value(args, &state);
    }

    ioc_unlock(iocroot);

    if (rval) return rval;

getout:
    rval = PyList_New(2);
    PyList_SetItem(rval, 0, Py_BuildValue("i", (int)0));
    PyList_SetItem(rval, 1, Py_BuildValue("i", (int)0));
    return rval;
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
    {"set", (PyCFunction)Signal_set, METH_VARARGS, "Store signal data"},
    {"get", (PyCFunction)Signal_get, METH_VARARGS, "Get signal data"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject SignalType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".Signal",  /* tp_name */
    sizeof(Signal),                           /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)Signal_dealloc,               /* tp_dealloc */
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
    "Signal objects",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    Signal_methods,                           /* tp_methods */
    Signal_members,                           /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    Signal_new,                               /* tp_new */
};
