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

/* Working state structure used to store data parsed from arguments.
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


/* Working state structure used to read data.
 */
typedef struct
{
    iocSignal *signal;
    osalTypeId type_id;
    os_int max_values, nro_values;
    os_boolean no_state_bits;
}
SignalGetState;


/* Forward referred static functions.
 */
static osalStatus Signal_try_setup(
    Signal *self,
    iocRoot *iocroot);


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
        *io_path = NULL;

    static char *kwlist[] = {
        "root",
        "io_path",
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
    signal->handle = handle;
    self->ncolumns = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Os",
         kwlist, &pyroot, &io_path))
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

    ioc_iopath_to_identifiers(iocroot, identifiers, io_path, IOC_EXPECT_SIGNAL);

    self->status = OSAL_SUCCESS;

#if IOPYTHON_TRACE
    PySys_WriteStdout("Signal.new(%s)\n", io_path);
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

  The Signal_dealloc function releases the associated Python object.

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
        switch (state->type_id)
        {
            case OS_BOOLEAN:
            case OS_CHAR:
            case OS_UCHAR:
            case OS_SHORT:
            case OS_USHORT:
            case OS_INT:
            case OS_UINT:
            case OS_INT64:
            case OS_LONG:
                state->storage.vv.value.l = x;
                break;

            case OS_FLOAT:
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
        switch (state->type_id)
        {
            case OS_BOOLEAN:
            case OS_CHAR:
            case OS_UCHAR:
            case OS_SHORT:
            case OS_USHORT:
            case OS_INT:
            case OS_UINT:
            case OS_INT64:
            case OS_LONG:
                state->storage.vv.value.l = os_round_long(x);
                break;

            case OS_FLOAT:
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
            Py_XDECREF(py_repr);
            Py_XDECREF(py_str);
        }

        else if (PyLong_Check(a)) {
            l = PyLong_AsLong(a);
            Signal_store_long(l, state);
        }

        else if (PyFloat_Check(a)) {
            double d = PyFloat_AsDouble(a);
            Signal_store_double(d, state);
        }

        else if (PySequence_Check(a))
        {
            Signal_set_sequence(a, state);
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
    os_int offset = 0;

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
    ioc_moves_array(state->signal, offset, state->buf, state->max_values,
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
    if (Signal_try_setup(self, iocroot))
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

/* Lock must be on
 * */
static osalStatus Signal_try_setup(
    Signal *self,
    iocRoot *iocroot)
{
    iocDynamicSignal *dsignal;

    /* If setup is already good.
     */
    if (self->signal.handle->mblk)
    {
        return OSAL_SUCCESS;
    }

    dsignal = ioc_setup_signal_by_identifiers(iocroot, &self->identifiers, &self->signal);
    if (self->signal.handle->mblk == OS_NULL)
    {
        return OSAL_STATUS_FAILED;
    }

    if (dsignal)
    {
        self->ncolumns = dsignal->ncolumns;
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get simple signal value.
  @anchor Signal_get_one_value

  @return  None.

****************************************************************************************************
*/
static PyObject *Signal_get_one_value(
    SignalGetState *state)
{
    iocValue vv;
    PyObject *rval, *value;

    ioc_movex_signals(state->signal, &vv, 1, IOC_SIGNAL_NO_THREAD_SYNC);

    if ((vv.state_bits & OSAL_STATE_CONNECTED) == 0 && state->no_state_bits)
    {
        return Py_BuildValue("i", (int)0);
    }

    switch (state->type_id)
    {
        case OS_BOOLEAN:
        case OS_CHAR:
        case OS_UCHAR:
        case OS_SHORT:
        case OS_USHORT:
        case OS_INT:
        case OS_UINT:
            value = Py_BuildValue("i", (int)vv.value.l);
            break;

        case OS_INT64:
        case OS_LONG:
            value = Py_BuildValue("l", (long long)vv.value.l);
            break;

        case OS_FLOAT:
        case OS_DOUBLE:
            value = Py_BuildValue("d", (double)vv.value.d);
            break;

        default:
            value = Py_BuildValue("i", (int)0);
            break;
    }

    if (state->no_state_bits)
    {
        return value;
    }

    rval = PyList_New(2);
    PyList_SetItem(rval, 0, Py_BuildValue("i", (int)vv.state_bits));
    PyList_SetItem(rval, 1, value);

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
    SignalGetState *state)
{
    os_memsz buf_sz, type_sz;
    PyObject *rval, *list, *value;
    os_int i;
    os_char state_bits, *buf;
    os_char fixbuf[64];
    os_int offset = 0;

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
    state_bits = ioc_moves_array(state->signal, offset, buf, state->max_values,
        OSAL_STATE_CONNECTED, IOC_SIGNAL_NO_THREAD_SYNC);

    if (!state->nro_values)
        state->nro_values = state->max_values;

    if ((state_bits & OSAL_STATE_CONNECTED) == 0 && state->no_state_bits)
    {
        state->max_values = 0;
    }

    list = PyList_New(state->nro_values);
    for(i = 0; i < state->nro_values; i++)
    {
        if (i < state->max_values)
        {
            switch (state->type_id)
            {
                case OS_BOOLEAN:
                case OS_CHAR:
                    value = Py_BuildValue("i", (int)((os_char*)buf)[i]);
                    break;

                case OS_UCHAR:
                    value = Py_BuildValue("i", (unsigned int)((os_uchar*)buf)[i]);
                    break;

                case OS_SHORT:
                    value = Py_BuildValue("i", (int)((os_short*)buf)[i]);
                    break;

                case OS_USHORT:
                    value = Py_BuildValue("i", (int)((os_ushort*)buf)[i]);
                    break;

                case OS_INT:
                    value = Py_BuildValue("i", (int)((os_int*)buf)[i]);
                    break;

                case OS_UINT:
                    value = Py_BuildValue("l", (unsigned long)((os_uint*)buf)[i]);
                    break;

                case OS_INT64:
                case OS_LONG:
                    value = Py_BuildValue("l", (long long)((os_long*)buf)[i]);
                    break;

                case OS_FLOAT:
                    value = Py_BuildValue("d", (double)((os_float*)buf)[i]);
                    break;

                case OS_DOUBLE:
                    value = Py_BuildValue("d", (double)((os_double*)buf)[i]);
                    break;

                default:
                    value = Py_BuildValue("i", (int)0);
                    break;
            }
        }
        else
        {
            value = Py_BuildValue("i", (int)0);
        }

        PyList_SetItem(list, i, value);
    }

    if (state->no_state_bits)
    {
        rval = list;
    }
    else
    {
        rval = PyList_New(2);
        PyList_SetItem(rval, 0, Py_BuildValue("i", (int)state_bits));
        PyList_SetItem(rval, 1, list);
    }

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
static PyObject *Signal_get_internal(
    Signal *self,
    SignalGetState *state)
{
    iocRoot *iocroot;
    PyObject *rval = OS_NULL;

    /* Get IOC root pointer. Check not to crash on exceptional situations.
     */
    if (self->pyroot == OS_NULL) goto getout;
    iocroot = self->pyroot->root;
    if (iocroot == OS_NULL) goto getout;
    ioc_lock(iocroot);

    /* Find dynamic information for this signal.
     */
    state->signal = &self->signal;
    if (Signal_try_setup(self, iocroot))
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    /* If the signal is string.
     */
    state->type_id = (state->signal->flags & OSAL_TYPEID_MASK);
    if (!state->max_values || state->signal->n < state->max_values)
    {
        state->max_values = state->signal->n;
    }
    if (state->type_id == OS_STR)
    {
    }

    /* If this signal is an array
     */
    if (state->signal->n > 1)
    {
        rval = Signal_get_array(state);
    }

    /* Otherwise this is single value signal.
     */
    else
    {
        rval = Signal_get_one_value(state);
    }

    ioc_unlock(iocroot);

    if (rval) return rval;

getout:
    if (state->no_state_bits)
    {
        return Py_BuildValue("i", (int)0);
    }
    rval = PyList_New(2);
    PyList_SetItem(rval, 0, Py_BuildValue("i", (int)0));
    PyList_SetItem(rval, 1, Py_BuildValue("i", (int)0));
    return rval;
}


/**
****************************************************************************************************

  @brief Get signal value from memory block.

****************************************************************************************************
*/
static PyObject *Signal_get(
    Signal *self,
    PyObject *args,
    PyObject *kwds)
{
    SignalGetState state;
    int nro_values = 0, max_values = 0;

    static char *kwlist[] = {
        "nro_values",
        "max_values",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ii",
         kwlist, &nro_values, &max_values))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    os_memclear(&state, sizeof(state));
    state.max_values = max_values;
    state.nro_values = nro_values;

    return Signal_get_internal(self, &state);
}


/**
****************************************************************************************************

  @brief Get signal value from memory block without state bits.

****************************************************************************************************
*/
static PyObject *Signal_get0(
    Signal *self,
    PyObject *args,
    PyObject *kwds)
{
    SignalGetState state;
    int nro_values = 0, max_values = 0;

    static char *kwlist[] = {
        "nro_values",
        "max_values",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ii",
         kwlist, &nro_values, &max_values))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    os_memclear(&state, sizeof(state));
    state.max_values = max_values;
    state.nro_values = nro_values;
    state.no_state_bits = OS_TRUE;

    return Signal_get_internal(self, &state);
}


/**
****************************************************************************************************

  @brief Get signal value from memory block without state bits.

****************************************************************************************************
*/
static PyObject *Signal_get_attribute(
    Signal *self,
    PyObject *args,
    PyObject *kwds)
{
    char *attrib_name = OS_NULL;
    PyObject *rval;
    iocRoot *iocroot;
    int w;

    static char *kwlist[] = {
        "name",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s",
         kwlist, &attrib_name))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    /* Get IOC root pointer. Check not to crash on exceptional situations.
     */
    if (self->pyroot == OS_NULL) goto getout;
    iocroot = self->pyroot->root;
    if (iocroot == OS_NULL) goto getout;
    ioc_lock(iocroot);

    /* Find dynamic information for this signal.
     */
    if (Signal_try_setup(self, iocroot))
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    if (!os_strcmp(attrib_name, "n"))
    {
        rval = Py_BuildValue("i", (int)self->signal.n);
    }
    else if (!os_strcmp(attrib_name, "ncolumns"))
    {
        w = self->ncolumns;
        rval = Py_BuildValue("i", (int)(w >= 1 ? w : 1));
    }
    else if (!os_strcmp(attrib_name, "type"))
    {
        rval = Py_BuildValue("s", osal_typeid_to_name(self->signal.flags & OSAL_TYPEID_MASK));
    }
    else
    {
        Py_INCREF(Py_None);
        rval = Py_None;
    }

    ioc_unlock(iocroot);
    return rval;

getout:
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
    {"set", (PyCFunction)Signal_set, METH_VARARGS, "Store signal data"},
    {"get", (PyCFunction)Signal_get, METH_VARARGS|METH_KEYWORDS, "Get signal data"},
    {"get0", (PyCFunction)Signal_get0, METH_VARARGS|METH_KEYWORDS, "Get signal data without state bits"},
    {"get_attribute", (PyCFunction)Signal_get_attribute, METH_VARARGS|METH_KEYWORDS, "Get signal attribute"},
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
