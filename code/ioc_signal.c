/**

  @file    ioc_signal.c
  @brief   Signals and memory blocks.
  @author  Pekka Lehtikoski
  @version 1.1
  @date    3.11.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/**
****************************************************************************************************

  @brief Read or write one or more signals to memory block.
  @anchor ioc_movex_signals

  The ioc_movex_signals() function reads or writes one or more signal values to memory block.
  This is used for basic types, like integers and floats. Use ioc_setx_str() for strings or
  ioc_setx_int_array() for arrays.

  The IOC_SIGNAL_WRITE Write signals to memory block. If this flag is not given, signals
  are read.

  IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT: Do not try to set OSAL_STATE_CONNECTED in state bits:
  if it as off, leave it off. This flag is meaningfull only when combined with IOC_SIGNAL_WRITE
  flag.

  If IOC_SIGNAL_NO_THREAD_SYNC is specified, this function does no thread synchronization.
  The caller must take care of synchronization by calling ioc_lock()/iocom_unlock() to
  synchronize thread access to IOCOM data structures.

  @param   signal Pointer to array of signal structures. This holds memory address,
           state bits and data type for each signal.
  @param   vv Array where signal values and state bits are stored, n_signals elements.
  @oaram   n_signals Number of elements in signals array.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flags can be combined by or
           operator: IOC_SIGNAL_WRITE, IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT,
           and IOC_SIGNAL_NO_THREAD_SYNC.
           Type flags here are ignored, since type is set for each signal separately in
           the signals array.
  @return  None.

****************************************************************************************************
*/
void ioc_movex_signals(
    const iocSignal *signal,
    iocValue *vv,
    os_int n_signals,
    os_short flags)
{
    iocRoot *root = OS_NULL;
    iocMemoryBlock *mblk = OS_NULL;
    const iocSignal *sig;
    os_char *p, nbuf[OSAL_NBUF_SZ], sb;
    os_memsz type_sz;
    os_int addr, i, v;
    os_short type_id;
    iocHandle *handle;
    os_boolean handle_tried = OS_FALSE, unlock_now;

    /* Check function arguments.
     */
    osal_debug_assert(signal != OS_NULL);
    osal_debug_assert(vv != OS_NULL);
    osal_debug_assert(n_signals > 0);

    if ((flags & IOC_SIGNAL_WRITE) == 0)
    {
        os_memclear(vv, n_signals * sizeof(iocValue));
    }

    /* Loop trough signal array.
     */
    for (i = 0; i < n_signals; i++)
    {
        sig = signal + i;

        handle = sig->handle;

        /* Get memory block pointer and start synchronization (unless disabled by no thread sync flag).
         */
        if (!handle_tried)
        {
            handle_tried = OS_TRUE;
            if (flags & IOC_SIGNAL_NO_THREAD_SYNC)
            {
                mblk = handle->mblk;
            }
            else
            {
                mblk = ioc_handle_lock_to_mblk(handle, &root);
            }
        }

        /* If memory block is not found, we do not know signal value.
         */
        if (mblk == OS_NULL)
        {
            vv[i].state_bits &= ~OSAL_STATE_CONNECTED;
            goto nextone;
        }

        type_id = sig->flags & OSAL_TYPEID_MASK;
        if (type_id == OS_STR)
        {
            if (flags & IOC_SIGNAL_WRITE)
            {
                osal_int_to_string(nbuf, sizeof(nbuf), vv[i].value.i);
                vv[i].state_bits = ioc_movex_str_signal(sig, nbuf, sizeof(nbuf), OSAL_STATE_CONNECTED, flags);
            }
            else
            {
                vv[i].state_bits = ioc_movex_str_signal(sig, nbuf, sizeof(nbuf), OSAL_STATE_CONNECTED, flags);
                vv[i].value.i = (os_int)osal_str_to_int(nbuf, OS_NULL);
            }
            goto nextone;
        }

        addr = sig->addr;
        if (type_id == OS_BOOLEAN)
        {
            type_sz = 0;
        }
        else
        {
            type_sz = osal_typeid_size(type_id);
            osal_debug_assert(type_sz > 0);
        }

        /* Verify that address is within memory block.
         */
        if (addr < 0 || addr + type_sz >= mblk->nbytes) /* >= one for state byte */
        {
            goto nextone;
        }

        /* Copy the state bits.
         */
        p = mblk->buf + addr;

        if (flags & IOC_SIGNAL_WRITE)
        {
            sb = vv[i].state_bits;

            /* Set boolean value (works only for integers)
             */
            v = vv[i].value.i;
            if (v) sb |= OSAL_STATE_BOOLEAN_VALUE;
            else  sb &= ~OSAL_STATE_BOOLEAN_VALUE;

            /* If memory block is connected as source, we may turn OSAL_STATE_CONNECTED
             * bit on. If memory block is disconnected, we sure turn it off.
             */
            if (mblk->sbuf.first)
            {
                if ((flags & IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT) == 0)
                    sb |= OSAL_STATE_CONNECTED;
            }
            else
            {
                sb &= ~OSAL_STATE_CONNECTED;
            }

            *(p++) = vv[i].state_bits = sb;
            ioc_byte_ordered_copy(p, (os_char*)&vv[i].value, type_sz, type_sz);
            ioc_mblk_invalidate(mblk, addr, (int)(addr + type_sz) /* no -1, we need also state byte */);
        }
        else
        {
            /* Get state bits from memory block. If memory block is not connected
               as target, turn OSAL_STATE_CONNECTED bit off in returned state, but
               do not modify memory block (we are receiving).
             */
            sb = *(p++);
            if (mblk->tbuf.first == OS_NULL) sb &= ~OSAL_STATE_CONNECTED;

            vv[i].state_bits = sb;

            /* If boolean, no more data
             */
            if (type_id == OS_BOOLEAN)
            {
                vv[i].value.i = (sb & OSAL_STATE_BOOLEAN_VALUE) ? 1 : 0;
            }
            else
            {
                ioc_byte_ordered_copy((os_char*)&vv[i].value, p, type_sz, type_sz);
            }
        }

nextone:
        /* We need to end synchronization nor if this is last signal, or
           the next signal uses different handle.
         */
        if (i + 1 < n_signals)
        {
            unlock_now = (os_boolean) (signal[i+1].handle != handle);
        }
        else
        {
            unlock_now = OS_TRUE;
        }

        /* End synchronization (unless disabled by no thread sync flag).
         */
        if (unlock_now)
        {
            if ((flags & IOC_SIGNAL_NO_THREAD_SYNC) == 0)
            {
                ioc_unlock(root);
            }
            handle_tried = OS_FALSE;
        }
    }
}


/**
****************************************************************************************************

  @brief Set integer value as a signal.
  @anchor ioc_sets_int

  The ioc_sets_int() function writes one signal value to memory block. This is used for basic
  types like integers and floats and cannot be used for strings or arrays.

  @param   signal Pointer to signal structure. This holds memory address,  state bits and data
           type for the signal.
  @param   value Integer value to write.

  @return  Updated state bits, at least OSAL_STATE_CONNECTED and possibly other bits.

****************************************************************************************************
*/
os_char ioc_sets_int(
    const iocSignal *signal,
    os_int value,
    os_char state_bits)
{
    iocValue vv;

    if ((signal->flags & OSAL_TYPEID_MASK) == OS_FLOAT) vv.value.f = (os_float)value;
    else vv.value.i = value;
    vv.state_bits = state_bits;

    ioc_movex_signals(signal, &vv, 1, IOC_SIGNAL_WRITE);
    return vv.state_bits;
}


os_char ioc_sets_double(
    const iocSignal *signal,
    os_double value,
    os_char state_bits)
{
    iocValue vv;

    switch (signal->flags & OSAL_TYPEID_MASK)
    {
        case OS_FLOAT: vv.value.f = (os_float)value; break;
        case OS_DOUBLE: vv.value.d = value; break;
        case OS_LONG: vv.value.l = os_round_long(value); break;
        default: vv.value.i = os_round_int(value); break;
    }
    vv.state_bits = state_bits;

    ioc_movex_signals(signal, &vv, 1, IOC_SIGNAL_WRITE);
    return vv.state_bits;
}


/**
****************************************************************************************************

  @brief Set integer value as a signal.
  @anchor ioc_setx_int

  The ioc_setx_int() function writes one signal value to memory block. This is used for basic
  types like integers and floats and cannot be used for strings or arrays.

  @param   handle Memory block handle.
  @param   address Address within memory block.
  @param   value Integer value to write.
  @oaram   state_bits State bits. This typically has OSAL_STATE_CONNECTED and if we have a problem
           with this signal OSAL_STATE_ORANGE and/or OSAL_STATE_YELLOW bit.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flags can be combined by or
           operator: IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT and IOC_SIGNAL_NO_THREAD_SYNC.
           Storage type to be used in memory block needs to be specified here by setting one
           of: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT, OS_INT, OS_UINT or OS_FLOAT

  @return  Updated state bits, at least OSAL_STATE_CONNECTED and possibly other bits.

****************************************************************************************************
*/
os_char ioc_setx_int(
    iocHandle *handle,
    os_int addr,
    os_int value,
    os_char state_bits,
    os_short flags)
{
    iocSignal signal;
    iocValue vv;

    if ((flags & OSAL_TYPEID_MASK) == OS_FLOAT) vv.value.f = (os_float)value;
    else vv.value.i = value;
    vv.state_bits = state_bits;

    os_memclear(&signal, sizeof(signal));
    signal.handle = handle;
    signal.addr = addr;
    signal.flags = (flags & ~IOC_SIGNAL_FLAGS_MASK);
    signal.n = 1;

    ioc_movex_signals(&signal, &vv, 1, flags|IOC_SIGNAL_WRITE);
    return vv.state_bits;
}


/**
****************************************************************************************************

  @brief Set floating point value as a signal.
  @anchor ioc_setx_float

  The ioc_setx_float() function writes one signal value to memory block. This is used for basic
  types like integers and floats and cannot be used for strings or arrays.

  @param   handle Memory block handle.
  @param   address Address within memory block.
  @param   value Floating point value to write.
  @oaram   state_bits State bits. This typically has OSAL_STATE_CONNECTED and if we have a problem
           with this signal OSAL_STATE_ORANGE and/or OSAL_STATE_YELLOW bit. Can be OS_NULL if
           not needed.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flags can be combined by or
           operator: IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT and IOC_SIGNAL_NO_THREAD_SYNC.
           Storage type to be used in memory block needs to be specified here by setting one
           of: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT, OS_INT, OS_UINT or OS_FLOAT

  @return  Updated state bits, at least OSAL_STATE_CONNECTED and possibly other bits.

****************************************************************************************************
*/
os_char ioc_setx_float(
    iocHandle *handle,
    os_int addr,
    os_float value,
    os_char state_bits,
    os_short flags)
{
    iocSignal signal;
    iocValue vv;

    os_memclear(&signal, sizeof(signal));

    if ((flags & OSAL_TYPEID_MASK) == OS_FLOAT) vv.value.f = value;
    else vv.value.i = os_round_int(value);
    vv.state_bits = state_bits;

    signal.handle = handle;
    signal.addr = addr;
    signal.flags = (flags & ~IOC_SIGNAL_FLAGS_MASK);
    signal.n = 1;

    ioc_movex_signals(&signal, &vv, 1, flags|IOC_SIGNAL_WRITE);
    return vv.state_bits;
}


/**
****************************************************************************************************

  @brief Get signal as integer value.
  @anchor ioc_gets_int

  The ioc_gets_int() function reads one signal value from memory block. This is used for basic
  types like integers and floats and cannot be used for strings or arrays.

  @param   signal Pointer to signal structure. This holds memory address,  state bits and data
           type for the signal.
  @oaram   state_bits Pointer to integer where to store state bits. OSAL_STATE_CONNECTED bit
           indicates that we have the signal value. HW errors are indicated.

  @return  Signal value as integer.

****************************************************************************************************
*/
os_int ioc_gets_int(
    const iocSignal *signal,
    os_char *state_bits)
{
    iocValue vv;

    ioc_movex_signals(signal, &vv, 1, IOC_SIGNAL_DEFAULT);
    if (state_bits) *state_bits = vv.state_bits;


    if ((signal->flags & OSAL_TYPEID_MASK) == OS_FLOAT) return os_round_int(vv.value.f);
    else return vv.value.i;
}


os_double ioc_gets_double(
    const iocSignal *signal,
    os_char *state_bits)
{
    iocValue vv;

    ioc_movex_signals(signal, &vv, 1, IOC_SIGNAL_DEFAULT);
    if (state_bits) *state_bits = vv.state_bits;

    switch (signal->flags & OSAL_TYPEID_MASK)
    {
        case OS_FLOAT: return vv.value.f; break;
        case OS_DOUBLE: return vv.value.d; break;
        case OS_LONG: return (os_double)vv.value.l; break;
        default: return vv.value.i; break;
    }
}


/**
****************************************************************************************************

  @brief Get integer signal value.
  @anchor ioc_getx_int

  The ioc_getx_int() function reads one signal value from to memory block. This is used for basic
  types like integers and floats and cannot be used for strings or arrays.

  @param   handle Memory block handle.
  @param   address Address within memory block.
  @oaram   state_bits Pointer to integer where to store state bits.
           OSAL_STATE_CONNECTED indicates that we have the signal value. HW errors are indicated
           by OSAL_STATE_ORANGE and/or OSAL_STATE_YELLOW bits.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flag can be combined by or
           operator: IOC_SIGNAL_NO_THREAD_SYNC. Storage type to be used in memory block needs to
           be specified here by setting one of: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT,
           OS_INT, OS_UINT or OS_FLOAT

  @return  Integer value.

****************************************************************************************************
*/
os_int ioc_getx_int(
    iocHandle *handle,
    os_int addr,
    os_char *state_bits,
    os_short flags)
{
    iocSignal signal;
    iocValue vv;

    os_memclear(&signal, sizeof(signal));

    signal.handle = handle;
    signal.addr = addr;
    signal.flags = (flags & ~IOC_SIGNAL_FLAGS_MASK);
    signal.n = 1;

    ioc_movex_signals(&signal, &vv, 1, flags);
    if (state_bits) *state_bits = vv.state_bits;

    if ((flags & OSAL_TYPEID_MASK) == OS_FLOAT) return os_round_int(vv.value.f);
    else return vv.value.i;
}


/**
****************************************************************************************************

  @brief Get floating point signal value.
  @anchor ioc_getx_float

  The ioc_getx_float() function reads one signal value from to memory block. This is returns
  value as float, integers and string are converted.

  @param   handle Memory block handle.
  @param   address Address within memory block.
  @oaram   state_bits Pointer to integer where to store state bits.
           OSAL_STATE_CONNECTED indicates that we have the signal value. HW errors are indicated
           by OSAL_STATE_ORANGE and/or OSAL_STATE_YELLOW bits. Can be OS_NULL if not needed.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flag can be combined by or
           operator: IOC_SIGNAL_NO_THREAD_SYNC. Storage type to be used in memory block needs to
           be specified here by setting one of: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT,
           OS_INT, OS_UINT or OS_FLOAT

  @return  Floating point value.

****************************************************************************************************
*/
os_float ioc_getx_float(
    iocHandle *handle,
    os_int addr,
    os_char *state_bits,
    os_short flags)
{
    iocSignal signal;
    iocValue vv;

    os_memclear(&signal, sizeof(signal));
    signal.handle = handle;
    signal.addr = addr;
    signal.flags = (flags & ~IOC_SIGNAL_FLAGS_MASK);
    signal.n = 1;

    ioc_movex_signals(&signal, &vv, 1, flags);
    if (state_bits) *state_bits = vv.state_bits;

    if ((flags & OSAL_TYPEID_MASK) == OS_FLOAT) return vv.value.f;
    else return (os_float)vv.value.i;
}


/**
****************************************************************************************************

  @brief Read or write one string from/to memory block.
  @anchor ioc_movex_str_signal

  The ioc_movex_str_signal() function reads or writes one string signal from or to memory
  block.

  The IOC_SIGNAL_WRITE Write string to memory block. If this flag is not given, string
  is read.

  IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT: Do not try to set OSAL_STATE_CONNECTED in state bits:
  if it as off, leave it off. This flag is meaningfull only when combined with IOC_SIGNAL_WRITE
  flag.

  If IOC_SIGNAL_NO_THREAD_SYNC is specified, this function does no thread synchronization.
  The caller must take care of synchronization by calling ioc_lock()/iocom_unlock() to
  synchronize thread access to IOCOM data structures.

  @param   signal Pointer to signal structure. This holds memory address, value,
           state bits and data type.
  @param   str Pointer to string buffer
  @param   str_sz String buffer size in bytes (including terminating NULL character).
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flags can be combined by or
           operator: IOC_SIGNAL_WRITE, IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT and
           IOC_SIGNAL_NO_THREAD_SYNC.
           Type flags here are ignored, since type is set for each signal separately in
           the signals array.
  @return  None.

****************************************************************************************************
*/
os_char ioc_movex_str_signal(
    const iocSignal *signal,
    os_char *str,
    os_memsz str_sz,
    os_char state_bits,
    os_short flags)
{
    iocRoot *root = OS_NULL;
    iocMemoryBlock *mblk;
    os_char *p;
    os_double dvalue;
    os_int addr, value;
    os_memsz len;
    iocHandle *handle;
    handle = signal->handle;

    /* Check function arguments.
     */
    osal_debug_assert(handle != OS_NULL);
    osal_debug_assert(signal != OS_NULL);
    osal_debug_assert(str != OS_NULL);

    /* If the value in memory block is actually integer or float.
     */
    switch (signal->flags & OSAL_TYPEID_MASK)
    {
        case OS_STR:
            break;

        case OS_FLOAT:
            if (flags & IOC_SIGNAL_WRITE)
            {
                return ioc_sets_double(signal, osal_string_to_double(str, OS_NULL), state_bits);
            }
            else
            {
                dvalue = ioc_gets_double(signal, &state_bits);
                osal_double_to_string(str, str_sz, dvalue, 4, OSAL_FLOAT_DEFAULT);
                return state_bits;
            }

        default:
            if (flags & IOC_SIGNAL_WRITE)
            {
                return ioc_sets_int(signal, (os_int)osal_str_to_int(str, OS_NULL), state_bits);
            }
            else
            {
                value = ioc_gets_int(signal, &state_bits);
                osal_int_to_string(str, str_sz, value);
                return state_bits;
            }
    }

    /* In case of errors.
     */
    if ((flags & IOC_SIGNAL_WRITE) == 0) *str = '\0';


    /* Get memory block pointer and start synchronization (unless disabled by no thread sync flag).
     */
    if (flags & IOC_SIGNAL_NO_THREAD_SYNC)
    {
        mblk = handle->mblk;
    }
    else
    {
        mblk = ioc_handle_lock_to_mblk(handle, &root);
    }

    /* If memory block is not found, we do not know signal value.
     */
    if (mblk == OS_NULL)
    {
        return 0;
    }

    /* If address is outside the memory block.
     */
    addr = signal->addr;
    if (addr < 0 || addr + signal->n + 1 > mblk->nbytes)
    {
        state_bits = 0;
        goto goon;
    }

    /* Copy the state bits.
     */
    p = mblk->buf + addr;

    if (flags & IOC_SIGNAL_WRITE)
    {
        /* If memory block is connected as source, we may turn OSAL_STATE_CONNECTED
         * bit on. If memory block is disconnected, we sure turn it off.
         */
        if (mblk->sbuf.first)
        {
            if ((flags & IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT) == 0)
                state_bits |= OSAL_STATE_CONNECTED;
        }
        else
        {
            state_bits &= ~OSAL_STATE_CONNECTED;
        }

        *(p++) = state_bits;
        len = os_strlen(str);
        if (signal->n < len) len = signal->n;
        ioc_byte_ordered_copy(p, str, len, 1);
        ioc_mblk_invalidate(mblk, addr, (int)(addr + len) /* no -1, we need also state byte */);
    }
    else
    {
        /* Get state bits from memory block. If memory block is not connected
           as target, turn OSAL_STATE_CONNECTED bit off in returned state, but
           do not modify memory block (we are receiving).
         */
        state_bits = *(p++);
        if (mblk->tbuf.first == OS_NULL)
        {
            state_bits &= ~OSAL_STATE_CONNECTED;
        }
        len = str_sz;
        if (signal->n < len) len = signal->n;
        ioc_byte_ordered_copy(str, p, len, 1);
    }

goon:
    /* End synchronization (unless disabled by no thread sync flag).
     */
    if ((flags & IOC_SIGNAL_NO_THREAD_SYNC) == 0)
    {
        ioc_unlock(root);
    }

    return state_bits;
}


/**
****************************************************************************************************

  @brief Read or write string from/to memory block.
  @anchor ioc_movex_signals

  The ioc_movex_str() function reads or writes a string from/to memory block.

  @param   handle Memory block handle.
  @param   signal Pointer to signal structure. This holds memory address, value,
           state bits and data type.
  @param   str Pointer to string buffer
  @param   str_sz String buffer size in bytes (including terminating NULL character).
  @oaram   state_bits State bits. This can have OSAL_STATE_CONNECTED and if we have a problem
           with this signal OSAL_STATE_ORANGE and/or OSAL_STATE_YELLOW bit.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flags can be combined by or
           operator: IOC_SIGNAL_WRITE, IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT and
           IOC_SIGNAL_NO_THREAD_SYNC.
           Type flags here are ignored, since type is set for each signal separately in
           the signals array.
  @return  Updated state bits.

****************************************************************************************************
*/
os_char ioc_movex_str(
    iocHandle *handle,
    os_int addr,
    os_char *str,
    os_memsz str_sz,
    os_char state_bits,
    os_short flags)
{
    iocSignal signal;
    os_memclear(&signal, sizeof(signal));

    signal.handle = handle;
    signal.addr = addr;
    signal.flags = (flags & ~IOC_SIGNAL_FLAGS_MASK);
    signal.n = (os_short)str_sz;

    state_bits = ioc_movex_str_signal(&signal, str, str_sz, state_bits, flags);

    return state_bits;
}


/**
****************************************************************************************************

  @brief Read or write array from/to memory block.
  @anchor ioc_movex_str_signal

  The ioc_movex_array_signal() function reads or writes array as one from or to memory
  block.

  The IOC_SIGNAL_WRITE Write string to memory block. If this flag is not given, string
  is read.

  IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT: Do not try to set OSAL_STATE_CONNECTED in state bits:
  if it as off, leave it off. This flag is meaningfull only when combined with IOC_SIGNAL_WRITE
  flag.

  If IOC_SIGNAL_NO_THREAD_SYNC is specified, this function does no thread synchronization.
  The caller must take care of synchronization by calling ioc_lock()/iocom_unlock() to
  synchronize thread access to IOCOM data structures.

  @param   signal Pointer to signal structure. This holds memory address, value,
           state bits and data type.
  @param   str Pointer to array buffer
  @param   Number of elements in array.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flags can be combined by or
           operator: IOC_SIGNAL_WRITE, IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT and
           IOC_SIGNAL_NO_THREAD_SYNC.
           Type flags here are ignored, since type is set for each signal separately in
           the signals array.

  @return  None.

****************************************************************************************************
*/
os_char ioc_movex_array_signal(
    const iocSignal *signal,
    void *array,
    os_int n,
    os_char state_bits,
    os_short flags)
{
    iocRoot *root= OS_NULL;
    iocMemoryBlock *mblk;
    os_char *p;
    os_uchar *b, ubyte;
    os_ushort bit;
    os_int addr, nn;
    os_memsz type_sz;
    osalTypeId type_id;
    iocHandle *handle;
    handle = signal->handle;

    /* Check function arguments.
     */
    osal_debug_assert(handle != OS_NULL);
    osal_debug_assert(signal != OS_NULL);
    osal_debug_assert(array != OS_NULL);
    osal_debug_assert(n > 0);

    type_id = signal->flags & OSAL_TYPEID_MASK;
    type_sz = osal_typeid_size(type_id);

    /* Get memory block pointer and start synchronization (unless disabled by no thread sync flag).
     */
    if (flags & IOC_SIGNAL_NO_THREAD_SYNC)
    {
        mblk = handle->mblk;
    }
    else
    {
        mblk = ioc_handle_lock_to_mblk(handle, &root);
    }

    /* If memory block is not found, we do not know signal value.
     */
    if (mblk == OS_NULL)
    {
        return 0;
    }

    /* If address is outside the memory block.
     */
    addr = signal->addr;
    nn = signal->n;
    if (type_id == OS_BOOLEAN) nn = (nn == 1 ? 0 : (nn + 7) >> 3);

    if (addr < 0 || addr + nn * type_sz >= mblk->nbytes)
    {
        state_bits = 0;
        goto goon;
    }

    /* Copy the state bits.
     */
    p = mblk->buf + addr;

    if (flags & IOC_SIGNAL_WRITE)
    {
        /* If memory block is connected as source, we may turn OSAL_STATE_CONNECTED
         * bit on. If memory block is disconnected, we sure turn it off.
         */
        if (mblk->sbuf.first)
        {
            if ((flags & IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT) == 0)
                state_bits |= OSAL_STATE_CONNECTED;
        }
        else
        {
            state_bits &= ~OSAL_STATE_CONNECTED;
        }
        if (signal->n < n) n = signal->n;

        /* Pack os_boolean as bits.
         */
        if (type_id == OS_BOOLEAN)
        {
            b = (os_uchar*)array;
            if (*b) { state_bits |= OSAL_STATE_BOOLEAN_VALUE; }
            else { state_bits &= ~OSAL_STATE_BOOLEAN_VALUE; }
            *(p++) = state_bits;

            if (n > 1)
            {
                nn = n;
                while (nn > 0 )
                {
                    bit = 1;
                    ubyte = 0;

                    while (nn > 0 && bit < 256)
                    {
                        if (*(b++)) ubyte |= (os_uchar)bit;
                        bit <<= 1;
                        nn--;
                    }
                    *(p++) = ubyte;
                }
                n = ((n + 7) >> 3);
            }
            else
            {
                n = 0;
            }

            ioc_mblk_invalidate(mblk, addr, (int)(addr + n) /* no -1, we need also state byte */);

        }

        else
        {
            *(p++) = state_bits;
            ioc_byte_ordered_copy(p, array, n, type_sz);
            ioc_mblk_invalidate(mblk, addr, (int)(addr + n * type_sz) /* no -1, we need also state byte */);
        }
    }
    else
    {
        /* Get state bits from memory block. If memory block is not connected
           as target, turn OSAL_STATE_CONNECTED bit off in returned state, but
           do not modify memory block (we are receiving).
         */
        state_bits = *(p++);
        if (mblk->tbuf.first == OS_NULL)
        {
            state_bits &= ~OSAL_STATE_CONNECTED;
        }
        if (signal->n < n) n = signal->n;

        /* Unpack os_boolean array from bits.
         */
        if (type_id == OS_BOOLEAN)
        {
            b = (os_uchar*)array;
            if (n > 1)
            {
                while (n > 0 )
                {
                    bit = 1;
                    ubyte = *(p++);

                    while (n > 0 && bit < 256)
                    {
                        *(b++) = (ubyte & (os_uchar)bit) ? OS_TRUE : OS_FALSE;
                        bit <<= 1;
                        n--;
                    }
                }

            }
            else
            {
                *b =  (state_bits &= OSAL_STATE_BOOLEAN_VALUE) ? OS_TRUE : OS_FALSE;
            }
        }
        else
        {
            ioc_byte_ordered_copy(array, p, n, type_sz);
        }
    }

goon:
    /* End synchronization (unless disabled by no thread sync flag).
     */
    if ((flags & IOC_SIGNAL_NO_THREAD_SYNC) == 0)
    {
        ioc_unlock(root);
    }

    return state_bits;
}


/**
****************************************************************************************************

  @brief Read or write array from/to memory block.
  @anchor ioc_movex_array

  The ioc_movex_array() function reads or writes an array as one signal.

  @param   handle Memory block handle.
  @param   signal Pointer to signal structure. This holds memory address, value,
           state bits and data type.
  @param   str Pointer to string buffer
  @param   str_sz String buffer size in bytes (including terminating NULL character).
  @oaram   state_bits State bits. This can have OSAL_STATE_CONNECTED and if we have a problem
           with this signal OSAL_STATE_ORANGE and/or OSAL_STATE_YELLOW bit.
  @param   flags IOC_SIGNAL_DEFAULT (0) for no flags. Following flags can be combined by or
           operator: IOC_SIGNAL_WRITE, IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT and
           IOC_SIGNAL_NO_THREAD_SYNC.
           Array element type must be here, one of OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT,
           OS_USHORT, OS_INT, OS_UINT or OS_FLOAT. This is used for type checking.
  @return  Updated state bits.

****************************************************************************************************
*/
os_char ioc_movex_array(
    iocHandle *handle,
    os_int addr,
    void *array,
    os_int n,
    os_char state_bits,
    os_short flags)
{
    iocSignal signal;
    os_memclear(&signal, sizeof(signal));

    signal.handle = handle;
    signal.addr = addr;
    signal.flags = (flags & ~IOC_SIGNAL_FLAGS_MASK);
    signal.n = n;

    return ioc_movex_array_signal(&signal, array, n, state_bits, flags);
}


/**
****************************************************************************************************

  @brief Check if memory address range belongs to signal.
  @anchor ioc_is_my_address

  The ioc_is_my_address() function checks if memory address range given as argument touches
  the address range of the signal. This is typically used by callback function to ask
  "is this signal affected?".

  @param   signal Pointer to signal structure.
  @param   start_addr First changed address.
  @param   end_addr Last changed address.
  @return  OS_TRUE if this address effects to the signal.

****************************************************************************************************
*/
os_boolean ioc_is_my_address(
    const iocSignal *signal,
    int start_addr,
    int end_addr)
{
    int addr, n;
    osalTypeId type_id;

    addr = signal->addr;
    if (end_addr < addr) return OS_FALSE;

    type_id = signal->flags & OSAL_TYPEID_MASK;

    n = signal->n;
    if (n < 1) n = 1;

    switch (type_id)
    {
        case OS_STR:
            n++;
            break;

        case OS_BOOLEAN:
            break;

        default:
            n = n * (int)osal_typeid_size(type_id) + 1;
            break;
    }

    return (os_boolean)(start_addr < addr + n);
}
