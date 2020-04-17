/**

  @file    iopy_brick_buffer.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    16.4.2020


  Example to get video data from remote camera:

    # Module: recive-camera-data.py
    # Pull data from IO device's (gina1) camera trough a server (frank, etc)
    # This example logs into "iocafenet" device network run by server in local computer.
    # User name "ispy" and password "pass" identify the client to server.
    # The camtest must be accepted as valid at server (this can be done with i-spy)
    # Client verifies validity of the server by acceptable certificate bundle 'myhome-bundle.crt'.

    from iocompython import Root, Connection, MemoryBlock, BrickBuffer
    import ioterminal
    import time

    # 9000 = select device number automatically
    my_device_nr = 9000

    def main():
        root = Root('camtest', device_nr=my_device_nr, security='certchainfile=myhome-bundle.crt')
        ioterminal.start(root)

        Connection(root, "127.0.0.1", "tls,down,dynamic", user='ispy.iocafenet', password='pass')
        camera_buffer = BrickBuffer(root, "exp.gina1.iocafenet", "imp.gina1.iocafenet", "rec_",timeout=-1)
        camera_buffer.set_recive(True);

        while (ioterminal.run(root)):
            data = camera_buffer.get()
            if data != None:
                print(data)

            time.sleep(0.01)

        root.delete()

    if (__name__ == '__main__'):
        main()


  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocompython.h"

/* Working state structure used to read data.
 */
typedef struct
{
    iocBrickBuffer *brick_buffer;
    osalTypeId type_id;
    os_int max_values, nro_values;
    os_boolean no_state_bits;
}
BrickBufferGetState;


/* Forward referred static functions.
 */
static void bb_init_signal(
    iocSignal *sig,
    iocHandle *handle);


/**
****************************************************************************************************

  @brief Constructor.

  The BrickBuffer_new function starts running the brick_buffer. Running brick_buffer will keep
  on running until the brick_buffer is deleted. It will attempt repeatedly to connect socket,
  etc transport to other IOCOM device and will move the data when transport is there.

  Note: Application must not delete and create new brick_buffer to reestablish the transport.
  This is handled by the running brick_buffer object.

  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *BrickBuffer_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    BrickBuffer *self;
    PyObject *pyroot = NULL;
    Root *root;

    iocRoot *iocroot;
    iocStreamerSignals sig;

    const char
        *exp = NULL,
        *imp = NULL,
        *prefix = "rec_",
        *flags = "";

    int
        timeout_ms = 0; /* 0 = use default timeout */

    static char *kwlist[] = {
        "root",
        "exp",
        "imp",
        "prefix",
        "timeout",
        "flags",
        NULL
    };

    self = (BrickBuffer *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
        return PyErr_NoMemory();
    }

    os_memclear(&self->brick_buffer, sizeof(iocBrickBuffer));
    os_memclear(&self->h_exp, sizeof(iocHandle));
    os_memclear(&self->h_imp, sizeof(iocHandle));

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oss|sis",
         kwlist, &pyroot, &exp, &imp, &prefix, &timeout_ms, &flags))
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

    self->is_device = OS_FALSE;
    if (os_strstr(flags, "device", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        self->is_device = OS_TRUE;
    }
    self->from_device = OS_TRUE;
    if (os_strstr(flags, "tod", OSAL_STRING_SEARCH_ITEM_NAME))
    {
        self->from_device = OS_FALSE;
    }

    bb_init_signal(&self->sig_cmd, &self->h_imp);
    bb_init_signal(&self->sig_select, &self->h_imp);
    bb_init_signal(&self->sig_state, &self->h_exp);

    if (self->from_device) {
        bb_init_signal(&self->sig_buf, &self->h_exp);
        bb_init_signal(&self->sig_head, &self->h_exp);
        bb_init_signal(&self->sig_tail, &self->h_imp);
    }
    else {
        bb_init_signal(&self->sig_buf, &self->h_imp);
        bb_init_signal(&self->sig_head, &self->h_imp);
        bb_init_signal(&self->sig_tail, &self->h_exp);
    }

    ioc_iopath_to_identifiers(iocroot, &self->exp_ids, exp, IOC_EXPECT_MEMORY_BLOCK);
    ioc_iopath_to_identifiers(iocroot, &self->imp_ids, imp, IOC_EXPECT_MEMORY_BLOCK);

    os_strncpy(self->prefix, prefix, IOPY_BB_PREFIX_SZ);

    /* Initialize brick buffer (does not allocate any memory yet)
    */
    os_memclear(&sig, sizeof(iocStreamerSignals));
    sig.to_device = !self->from_device;
    sig.cmd = &self->sig_cmd;
    sig.select = &self->sig_select;
    sig.state = &self->sig_state;
    sig.buf = &self->sig_buf;
    sig.head = &self->sig_head;
    sig.tail = &self->sig_tail;
    ioc_initialize_brick_buffer(&self->brick_buffer, &sig, iocroot, timeout_ms,
        self->is_device ? IOC_BRICK_DEVICE : IOC_BRICK_CONTROLLER);

    self->status = OSAL_SUCCESS;

#if IOPYTHON_TRACE
    PySys_WriteStdout("BrickBuffer.new(%s,%s)\n", exp, imp);
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

  The BrickBuffer_dealloc function releases the associated Python object and IOCOM brick_buffer.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_dealloc(
    BrickBuffer *self)
{
    if (self->pyroot)
    {
        if (self->h_exp.mblk) {
            ioc_release_handle(&self->h_exp);
        }

        if (self->h_imp.mblk) {
            ioc_release_handle(&self->h_imp);
        }
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *)self);

#if IOPYTHON_TRACE
    PySys_WriteStdout("BrickBuffer.dealloc()\n");
#endif
}


/**
****************************************************************************************************

  @brief Delete an IOCOM brick_buffer.

  The BrickBuffer_delete function closes the brick_buffer and releases any ressources for it.
  The brick_buffer must be explisitly closed by calling .delete() function, or by calling
  .delete() on the root object. But not both.

  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static PyObject *BrickBuffer_delete(
    BrickBuffer *self)
{
    if (self->pyroot)
    {
        if (self->h_exp.mblk) {
            ioc_release_handle(&self->h_exp);
        }

        if (self->h_imp.mblk) {
            ioc_release_handle(&self->h_imp);
        }
        Py_DECREF(self->pyroot);
        self->pyroot = OS_NULL;
    }

#if IOPYTHON_TRACE
    PySys_WriteStdout("BrickBuffer.delete()\n");
#endif
    /* Return "None".
     */
    Py_INCREF(Py_None);
    return Py_None;
}


/**
****************************************************************************************************

  Initialize unused signal structure.

  @param   sig Pointer to signal structure to initialize.
  @param   handle Pointer to memory block handle structure. Handle doesn't have to be initialized,
           just pointer is saved within the signal structure.
  @return  None.

****************************************************************************************************
*/
static void bb_init_signal(
    iocSignal *sig,
    iocHandle *handle)
{
    os_memclear(sig, sizeof(iocSignal));
    sig->handle = handle;
}


/**
****************************************************************************************************

  Set memory block handle for a signal.

  Lock must be on

  @param   sig Pointer to signal structure to set up.
  @param   handle Pointer to memory block handle structure. Handle doesn't have to be initialized,
           just pointer is saved within the signal structure.
  @return  None.

****************************************************************************************************
*/
static osalStatus bb_try_signal_setup(
    iocSignal *sig,
    os_char *name,
    os_char *prefix,
    iocIdentifiers *mblk_identifiers,
    iocRoot *iocroot)
{
    iocIdentifiers identifiers;

    /* If setup is already good.
     */
    if (sig->handle->mblk) {
        return OSAL_SUCCESS;
    }

    os_memcpy(&identifiers, mblk_identifiers, sizeof(iocIdentifiers));
    os_strncpy(identifiers.signal_name, prefix, IOC_SIGNAL_NAME_SZ);
    os_strncat(identifiers.signal_name, name, IOC_SIGNAL_NAME_SZ);

    ioc_setup_signal_by_identifiers(iocroot, &identifiers, sig);
    return sig->handle->mblk ? OSAL_SUCCESS : OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  Initialize all signal structires needed.

  Lock must be on

  @param   sig Pointer to signal structure to initialize.
  @param   sig Pointer to memory block handle structure. Handle doesn't have to be initialized,
           just pointer is saved within the signal structure.
  @return  None.

****************************************************************************************************
*/
static osalStatus bb_try_setup(
    BrickBuffer *self,
    iocRoot *iocroot)
{
     /* If setup is already good. We check head and tail because they are in different
       memory blocks and last to be set up.
     */
    if (self->sig_head.handle->mblk && self->sig_tail.handle->mblk) {
        return OSAL_SUCCESS;
    }

    if (bb_try_signal_setup(&self->sig_cmd, "cmd", self->prefix, &self->imp_ids, iocroot)) goto getout;
    if (bb_try_signal_setup(&self->sig_select, "select", self->prefix, &self->imp_ids, iocroot)) goto getout;
    if (bb_try_signal_setup(&self->sig_state, "state", self->prefix, &self->exp_ids, iocroot)) goto getout;

    if (self->from_device) {
        if (bb_try_signal_setup(&self->sig_buf, "buf", self->prefix, &self->exp_ids, iocroot)) goto getout;
        if (bb_try_signal_setup(&self->sig_head, "head", self->prefix, &self->exp_ids, iocroot)) goto getout;
        if (bb_try_signal_setup(&self->sig_tail, "tail", self->prefix, &self->imp_ids, iocroot)) goto getout;
    }
    else {
        if (bb_try_signal_setup(&self->sig_buf, "buf", self->prefix, &self->imp_ids, iocroot)) goto getout;
        if (bb_try_signal_setup(&self->sig_head, "head", self->prefix, &self->imp_ids, iocroot)) goto getout;
        if (bb_try_signal_setup(&self->sig_tail, "tail", self->prefix, &self->exp_ids, iocroot)) goto getout;
    }

    return OSAL_COMPLETED;

getout:
    return OSAL_STATUS_FAILED;
}



/**
****************************************************************************************************

  @brief Enable/disable reciving data.

****************************************************************************************************
*/
static PyObject *BrickBuffer_set_recive(
    BrickBuffer *self,
    PyObject *args,
    PyObject *kwds)
{
    int enable = OS_TRUE;
    static char *kwlist[] = {
        "enable",
        NULL
    };

    /* Get iocom root object pointer.
     */
    if (self->pyroot == OS_NULL) {
        PyErr_SetString(iocomError, "Root has been deleted");
        return NULL;
    }

    /* Parse Python argumenrs.
     */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &enable)) {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    ioc_brick_set_receive(&self->brick_buffer, enable);
    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Get brick from this buffer.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get(
    BrickBuffer *self,
    PyObject *args,
    PyObject *kwds)
{
    iocRoot *iocroot;
    int reserved = 0;
    osalStatus s;
    static char *kwlist[] = {
        "reserved",
        NULL
    };

    /* Get iocom root object pointer.
     */
    if (self->pyroot == OS_NULL) {
        PyErr_SetString(iocomError, "Root has been deleted");
        return NULL;
    }
    iocroot = self->pyroot->root;

    /* Parse Python argumenrs.
     */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &reserved)) {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    /* Synchronize and setup all signals, if we have not done that already.
     */
    ioc_lock(iocroot);
    s = bb_try_setup(self, iocroot);
    if (OSAL_IS_ERROR(s))
    {
        ioc_unlock(iocroot);
        Py_RETURN_NONE;
    }

    /* Receive data, return data if we got it.
     */
    ioc_run_brick_receive(&self->brick_buffer);

    /* End synchronization.
     */
    ioc_unlock(iocroot);
    // return BrickBuffer_get_internal(self, &state,
//        check_tbuf ? IOC_SIGNAL_DEFAULT : IOC_SIGNAL_NO_TBUF_CHECK);

    Py_RETURN_NONE;
}




#if 0
/**
****************************************************************************************************

  @brief Store long integer parsed from Python arguments in format expected for the brick_buffer.
  @anchor BrickBuffer_store_double

  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_store_long(
    os_long x,
    BrickBufferSetParseState *state)
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

  @brief Store double parsed from Python arguments in format expected for the brick_buffer.
  @anchor BrickBuffer_store_double

  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_store_double(
    os_double x,
    BrickBufferSetParseState *state)
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
  @anchor BrickBuffer_set_sequence

  Idea here is to parse python arguments, how ever structured in python call, to format needed
  for storing value(s) into register map.

  - String brick_buffer, as one string argument.
  - Array, as array of numbers
  - Single value

  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_set_sequence(
    PyObject *args,
    BrickBufferSetParseState *state)
{
    PyObject *a, *py_repr, *py_str;

    long i, length, l;
    double d;
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
            if (str == NULL) str = "";
            if (*str == '\'') str++;
            d = osal_str_to_double(str, OS_NULL);
            BrickBuffer_store_double(d, state);

            Py_XDECREF(py_repr);
            Py_XDECREF(py_str);
        }

        else if (PyLong_Check(a)) {
            l = PyLong_AsLong(a);
            BrickBuffer_store_long(l, state);
        }

        else if (PyFloat_Check(a)) {
            d = PyFloat_AsDouble(a);
            BrickBuffer_store_double(d, state);
        }

        else if (PySequence_Check(a))
        {
            BrickBuffer_set_sequence(a, state);
        }

        Py_DECREF(a);
    }
}


/**
****************************************************************************************************

  @brief Set simple brick_buffer containing one numeric value.
  @anchor BrickBuffer_set_one_value

  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_set_one_value(
    PyObject *args,
    BrickBufferSetParseState *state)
{
    state->max_values = 1;

    /* Process the Python arguments into format we need.
     */
    BrickBuffer_set_sequence(args, state);

    if (state->n_values)
    {
        state->storage.vv.state_bits = state->state_bits;
        ioc_movex_brick_buffers(state->brick_buffer, &state->storage.vv, 1,
            IOC_SIGNAL_WRITE|IOC_SIGNAL_NO_THREAD_SYNC);
    }
}


/**
****************************************************************************************************

  @brief Set string brick_buffer.
  @anchor BrickBuffer_set_string_value

  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_set_string_value(
    PyObject *args,
    BrickBufferSetParseState *state)
{
    os_char *str_value = NULL;

    if (!PyArg_ParseTuple(args, "s", &str_value))
    {
        PyErr_SetString(iocomError, "String argument expected");
        return;
    }

    ioc_moves_str(state->brick_buffer, str_value, -1, state->state_bits, IOC_SIGNAL_WRITE|OS_STR);
}


/**
****************************************************************************************************

  @brief Set brick_buffer containing array of values.
  @anchor BrickBuffer_set_array

  @return  None.

****************************************************************************************************
*/
static void BrickBuffer_set_array(
    PyObject *args,
    BrickBufferSetParseState *state)
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
    BrickBuffer_set_sequence(args, state);

    /* Write always all values in array, even if caller would provides fewer. Rest will be zeros.
     */
    ioc_moves_array(state->brick_buffer, offset, state->buf, state->max_values,
        state->state_bits, IOC_SIGNAL_WRITE|IOC_SIGNAL_NO_THREAD_SYNC);

    /* If we allocated extra buffer space, free it.
     */
    if (buf_sz > IOPY_FIXBUF_SZ)
    {
        os_free(state->buf, buf_sz);
    }
}
#endif

#if 0
/**
****************************************************************************************************

  @brief Store brick_buffer value into memory block.
  @anchor BrickBuffer_set

  The BrickBuffer.set() function finds mathing dynamic information for this brick_buffer, and according
  to the information stores brick_buffer value: string, array or one numerical value into the memory
  block containing the brick_buffer.

****************************************************************************************************
*/
static PyObject *BrickBuffer_set(
    BrickBuffer *self,
    PyObject *args)
{
    iocRoot *iocroot;
    BrickBufferSetParseState state;

    /* Get IOC root pointer. Check not to crash on exceptional situations.
     */
    if (self->pyroot == OS_NULL) goto getout;
    iocroot = self->pyroot->root;
    if (iocroot == OS_NULL) goto getout;
    ioc_lock(iocroot);

    /* Find dynamic information for this brick_buffer.
     */
    os_memclear(&state, sizeof(state));
    state.brick_buffer = &self->brick_buffer;
    if (BrickBuffer_try_setup(self, iocroot))
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    /* If the brick_buffer is string.
     */
    state.type_id = (state.brick_buffer->flags & OSAL_TYPEID_MASK);
    state.max_values = state.brick_buffer->n;
    state.state_bits = OSAL_STATE_CONNECTED;
    if (state.type_id == OS_STR)
    {
        state.is_string = OS_TRUE;
        BrickBuffer_set_string_value(args, &state);
    }

    /* If this brick_buffer is an array
     */
    else if (state.max_values > 1)
    {
        state.is_array = OS_TRUE;
        BrickBuffer_set_array(args, &state);
    }

    /* Otherwise this is single value brick_buffer.
     */
    else
    {
        BrickBuffer_set_one_value(args, &state);
    }

    ioc_unlock(iocroot);

getout:
    Py_RETURN_NONE;
}


/**
****************************************************************************************************

  @brief Store brick_buffer value with state bits into memory block.
  @anchor BrickBuffer_set_ext

  The BrickBuffer.set_ext() sets brick_buffer value and state bits. For example
  mybrick_buffer.set_ext(value=(1), state_bits=3).

****************************************************************************************************
*/
static PyObject *BrickBuffer_set_ext(
    BrickBuffer *self,
    PyObject *args,
    PyObject *kwds)
{
    iocRoot *iocroot;
    BrickBufferSetParseState state;
    PyObject *value = NULL;
    int state_bits = OSAL_STATE_CONNECTED;

    static char *kwlist[] = {
        "value",
        "state_bits",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i",
         kwlist, &value, &state_bits))
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

    /* Find dynamic information for this brick_buffer.
     */
    os_memclear(&state, sizeof(state));
    state.brick_buffer = &self->brick_buffer;
    state.state_bits = (os_char)state_bits;
    if (BrickBuffer_try_setup(self, iocroot))
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    /* If string?
     */
    state.type_id = (state.brick_buffer->flags & OSAL_TYPEID_MASK);
    state.max_values = state.brick_buffer->n;
    if (state.type_id == OS_STR)
    {
        state.is_string = OS_TRUE;
        BrickBuffer_set_string_value(args, &state);
    }

    /* If array?
     */
    else if (state.max_values > 1)
    {
        state.is_array = OS_TRUE;
        BrickBuffer_set_array(value, &state);
    }

    /* Otherwise single value
     */
    else
    {
        BrickBuffer_set_one_value(args, &state);
    }

    ioc_unlock(iocroot);

getout:
    Py_RETURN_NONE;
}

/* Lock must be on
 * */
static osalStatus BrickBuffer_try_setup(
    BrickBuffer *self,
    iocRoot *iocroot)
{
    iocDynamicBrickBuffer *dbrick_buffer;

    /* If setup is already good.
     */
    if (self->brick_buffer.handle->mblk)
    {
        return OSAL_SUCCESS;
    }

    dbrick_buffer = ioc_setup_brick_buffer_by_identifiers(iocroot, &self->identifiers, &self->brick_buffer);
    if (self->brick_buffer.handle->mblk == OS_NULL)
    {
        return OSAL_STATUS_FAILED;
    }

    if (dbrick_buffer)
    {
        self->ncolumns = dbrick_buffer->ncolumns;
    }

    return OSAL_SUCCESS;
}

#endif


#if 0

/**
****************************************************************************************************

  @brief Get simple brick_buffer value.
  @anchor BrickBuffer_get_one_value

  @param   flags IOC_SIGNAL_DEFAULT for default operation. IOC_SIGNAL_NO_TBUF_CHECK disables
           checking if target buffer is connected to this memory block.
  @return  None.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get_one_value(
    BrickBufferGetState *state,
    os_short flags)
{
    iocValue vv;
    PyObject *rval, *value;

    ioc_movex_brick_buffers(state->brick_buffer, &vv, 1, IOC_SIGNAL_NO_THREAD_SYNC|flags);

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

  @brief Get string brick_buffer value.
  @anchor BrickBuffer_get_str_value

  @param   flags IOC_SIGNAL_DEFAULT for default operation. IOC_SIGNAL_NO_TBUF_CHECK disables
           checking if target buffer is connected to this memory block.
  @return  None.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get_str_value(
    BrickBufferGetState *state,
    os_short flags)
{
    PyObject *rval, *value;
    os_int n;

    os_char fixed_buf[64], *p, state_bits;

    n = state->brick_buffer->n;
    p = fixed_buf;
    if (n > sizeof(fixed_buf))
    {
        p = os_malloc(n, OS_NULL);
    }

    state_bits = ioc_moves_str(state->brick_buffer, p, n,
        OSAL_STATE_CONNECTED, IOC_SIGNAL_NO_THREAD_SYNC|flags);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0 && state->no_state_bits)
    {
        return Py_BuildValue("s", (char *)"");
    }

    value = Py_BuildValue("s", p);
    if (state->no_state_bits)
    {
        return value;
    }

    rval = PyList_New(2);
    PyList_SetItem(rval, 0, Py_BuildValue("i", (int)state_bits));
    PyList_SetItem(rval, 1, value);

    if (n > sizeof(fixed_buf))
    {
        os_free(p, n);
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get brick_buffer containing array of values.
  @anchor BrickBuffer_get_array

  @param   flags IOC_SIGNAL_DEFAULT for default operation. IOC_SIGNAL_NO_TBUF_CHECK disables
           checking if target buffer is connected to this memory block.
  @return  None.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get_array(
    BrickBufferGetState *state,
    os_short flags)
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
    state_bits = ioc_moves_array(state->brick_buffer, offset, buf, state->max_values,
        OSAL_STATE_CONNECTED, IOC_SIGNAL_NO_THREAD_SYNC|flags);

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

  @brief Get brick_buffer value from memory block.
  @anchor BrickBuffer_get_internal

  @param   flags IOC_SIGNAL_DEFAULT for default operation. IOC_SIGNAL_NO_TBUF_CHECK disables
           checking if target buffer is connected to this memory block.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get_internal(
    BrickBuffer *self,
    BrickBufferGetState *state,
    os_short flags)
{
    iocRoot *iocroot;
    PyObject *rval = OS_NULL;

    /* Get IOC root pointer. Check not to crash on exceptional situations.
     */
    if (self->pyroot == OS_NULL) goto getout;
    iocroot = self->pyroot->root;
    if (iocroot == OS_NULL) goto getout;
    ioc_lock(iocroot);

    /* Find dynamic information for this brick_buffer.
     */
    state->brick_buffer = &self->brick_buffer;
    if (BrickBuffer_try_setup(self, iocroot))
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    state->type_id = (state->brick_buffer->flags & OSAL_TYPEID_MASK);
    if (!state->max_values || state->brick_buffer->n < state->max_values)
    {
        state->max_values = state->brick_buffer->n;
    }

    /* If the brick_buffer is string.
     */
    if (state->type_id == OS_STR)
    {
        rval = BrickBuffer_get_str_value(state, flags);
    }

    /* If this brick_buffer is an array
     */
    else if (state->brick_buffer->n > 1)
    {
        rval = BrickBuffer_get_array(state, flags);
    }

    /* Otherwise this is single value brick_buffer.
     */
    else
    {
        rval = BrickBuffer_get_one_value(state, flags);
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

  @brief Get brick_buffer value from memory block.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get_ext(
    BrickBuffer *self,
    PyObject *args,
    PyObject *kwds)
{
    BrickBufferGetState state;
    int nro_values = 0, max_values = 0, check_tbuf = 1;

    static char *kwlist[] = {
        "nro_values",
        "max_values",
        "check_tbuf",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iii",
         kwlist, &nro_values, &max_values, &check_tbuf))
    {
        PyErr_SetString(iocomError, "Errornous function arguments");
        return NULL;
    }

    os_memclear(&state, sizeof(state));
    state.max_values = max_values;
    state.nro_values = nro_values;

    return BrickBuffer_get_internal(self, &state,
        check_tbuf ? IOC_SIGNAL_DEFAULT : IOC_SIGNAL_NO_TBUF_CHECK);
}


/**
****************************************************************************************************

  @brief Get brick_buffer value from memory block without state bits.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get(
    BrickBuffer *self,
    PyObject *args,
    PyObject *kwds)
{
    BrickBufferGetState state;
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

    return BrickBuffer_get_internal(self, &state, IOC_SIGNAL_NO_TBUF_CHECK);
}


/**
****************************************************************************************************

  @brief Get brick_buffer value from memory block without state bits.

****************************************************************************************************
*/
static PyObject *BrickBuffer_get_attribute(
    BrickBuffer *self,
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

    /* Find dynamic information for this brick_buffer.
     */
    if (BrickBuffer_try_setup(self, iocroot))
    {
        ioc_unlock(iocroot);
        goto getout;
    }

    if (!os_strcmp(attrib_name, "n"))
    {
        rval = Py_BuildValue("i", (int)self->brick_buffer.n);
    }
    else if (!os_strcmp(attrib_name, "ncolumns"))
    {
        w = self->ncolumns;
        rval = Py_BuildValue("i", (int)(w >= 1 ? w : 1));
    }
    else if (!os_strcmp(attrib_name, "type"))
    {
        rval = Py_BuildValue("s", osal_typeid_to_name(self->brick_buffer.flags & OSAL_TYPEID_MASK));
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

#endif

/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef BrickBuffer_members[] = {
    {(char*)"status", T_INT, offsetof(BrickBuffer, status), 0, (char*)"constructor status"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef BrickBuffer_methods[] = {
    {"delete", (PyCFunction)BrickBuffer_delete, METH_NOARGS, "Deletes IOCOM brick_buffer"},
    {"set_recive", (PyCFunction)BrickBuffer_set_recive, METH_VARARGS|METH_KEYWORDS, "Enable/disable reciving data"},
    {"get", (PyCFunction)BrickBuffer_get, METH_VARARGS|METH_KEYWORDS, "Get buffered data brick"},
//    {"set", (PyCFunction)BrickBuffer_set, METH_VARARGS, "Store brick_buffer data"},
//    {"set_ext", (PyCFunction)BrickBuffer_set_ext, METH_VARARGS|METH_KEYWORDS, "Set data and state bits"},
//    {"get_ext", (PyCFunction)BrickBuffer_get_ext, METH_VARARGS|METH_KEYWORDS, "Get brick_buffer data and state bits"},
//    {"get_attribute", (PyCFunction)BrickBuffer_get_attribute, METH_VARARGS|METH_KEYWORDS, "Get brick_buffer attribute"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject BrickBufferType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".BrickBuffer",  /* tp_name */
    sizeof(BrickBuffer),                           /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)BrickBuffer_dealloc,               /* tp_dealloc */
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
    "BrickBuffer objects",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    BrickBuffer_methods,                           /* tp_methods */
    BrickBuffer_members,                           /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    BrickBuffer_new,                               /* tp_new */
};
