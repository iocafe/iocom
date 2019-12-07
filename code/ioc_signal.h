/**

  @file    ioc_signal.h
  @brief   Signals and memory blocks.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    3.11.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_SIGNAL_INCLUDED
#define IOC_SIGNAL_INCLUDED

/**
****************************************************************************************************
  We may have multiple iocSignal structures within one app defined structure. This is common
  header for the application defined structure.
****************************************************************************************************
 */
struct iocSignal;

typedef struct iocMblkSignalHdr
{
    const os_char *mblk_name;
    iocHandle *handle;

    os_int n_signals;
    os_uint mblk_sz;

    struct iocSignal *first_signal;
}
iocMblkSignalHdr;


typedef struct iocDeviceHdr
{
    iocMblkSignalHdr **mblk_hdr;
    os_short n_mblk_hdrs;
}
iocDeviceHdr;

/* Additional flag bits to type in bits byte
 */
#define IOC_PIN_PTR 0x20


/**
****************************************************************************************************
  IO signal state structure. Keep the order for initialization.
****************************************************************************************************
 */
struct Pin;
typedef struct iocSignal
{
    /** Starting address in memory block.
     */
    os_int addr;

    /** For strings n can be number of bytes in memory block for the string. For arrays n is
        number of elements reserved in memory block. Either 0 or 1 for single variables.
        Unsigned type used for reason, we want to have max 65535 items.
     */
    os_ushort n;

    /** One of: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT, OS_INT, OS_UINT, OS_FLOAT
        or OS_STR.
        Flag bit IOC_PIN_PTR marks that ptr is "Pin *" pointer.
     */
    os_char flags;

    /** Pointer to memory block handle.
     */
    iocHandle *handle;

    /** Pointer to IO pin configuration structure. OS_NULL if this signal doesn't
        match to an IO pin.
     */
    const void *ptr;
}
iocSignal;

/** Current value and state bits for simple types.
 */
typedef struct iocValue
{
    /** State bits, indicate if signal is connected, or if any error is active.
     */
    os_char state_bits;

    /** Value, this is either integer l or floating point value d, depending on data type (flags).
     */
    union
    {
        os_long l;
        os_double d;
    }
    value;
}
iocValue;


/**
****************************************************************************************************
  Macros to simplify memory block access
****************************************************************************************************
 */
/* flags for memory block functions contain type, like OS_BOOLEAN, OS_USHORT, OS_FLOAT, etc,
   and may contain these additional flags.
 */
#define IOC_SIGNAL_DEFAULT 0
#define IOC_SIGNAL_WRITE 0x20
#define IOC_SIGNAL_DO_NOT_SET_CONNECTED_BIT 0x40
#define IOC_SIGNAL_NO_THREAD_SYNC 0x80
#define IOC_SIGNAL_FLAGS_MASK 0xE0

/* Macros for signal value and state bits within iocValue structure
 */
#define ioc_value_int(s) (s).value.i
#define ioc_value_double(s) (s).value.d
#define ioc_value_state_bits(s) (s).state_bits
#define ioc_value_typeid(s) ((s).flags & OSAL_TYPEID_MASK)
#define ioc_value_flags(s) ((s).flags)
#define ioc_is_value_connected(s) ((s).state_bits & OSAL_STATE_CONNECTED)
#define ioc_is_value_error_on(s) ((s).state_bits & OSAL_STATE_ERROR_MASK)
#define ioc_is_value_yellow(s) (((s).state_bits & OSAL_STATE_ERROR_MASK) == OSAL_STATE_YELLOW)
#define ioc_is_value_orange(s) (((s).state_bits & OSAL_STATE_ERROR_MASK) == OSAL_STATE_ORANGE)
#define ioc_is_value_red(s) (((s).state_bits & OSAL_STATE_ERROR_MASK) == OSAL_STATE_RED)

/* Set signal to memory block without initialized signal structure.
 */
#define ioc_set_boolean(h,a,v) ioc_setx_int((h), (a), (v), OSAL_STATE_CONNECTED, OS_BOOLEAN)
#define ioc_set_char(h,a,v) ioc_setx_int((h), (a), (v), OSAL_STATE_CONNECTED, OS_CHAR)
#define ioc_set_uchar(h,a,v) ioc_setx_int((h), (a), (os_uchar)(v), OSAL_STATE_CONNECTED, OS_UCHAR)
#define ioc_set_short(h,a,v) ioc_setx_int((h), (a), (v), OSAL_STATE_CONNECTED, OS_SHORT)
#define ioc_set_ushort ioc_setx_int((h), (a), (os_ushort)(v), OSAL_STATE_CONNECTED, OS_USHORT)
#define ioc_set_int(h,a,v) ioc_setx_int((h), (a), (v), OSAL_STATE_CONNECTED, OS_INT)
#define ioc_set_uint(h,a,v) ioc_setx_int((h), (a), (os_uint)(v), OSAL_STATE_CONNECTED, OS_UINT)
#define ioc_set_long(h,a,v) ioc_setx_int((h), (a), (v), OSAL_STATE_CONNECTED, OS_LONG)
#define ioc_set_str(h,a,st,ss) ioc_setx_str((h), (a), (st), (os_int)(ss), OSAL_STATE_CONNECTED, OS_STR)
#define ioc_set_float(h,a,v) ioc_setx_double((h), (a), (v), OSAL_STATE_CONNECTED, OS_FLOAT)
#define ioc_set_double(h,a,v) ioc_setx_double((h), (a), (v), OSAL_STATE_CONNECTED, OS_DOUBLE)

/* Get signal from memory block without initialized signal structure.
 */
#define ioc_get_boolean(h,a,sb) (os_boolean)ioc_getx_int((h), (a), (sb), OS_BOOLEAN)
#define ioc_get_char(h,a,sb) (os_char)ioc_getx_int((h), (a), (sb), OS_CHAR)
#define ioc_get_uchar(h,a,sb) (os_uchar)ioc_getx_int((h), (a), (sb), OS_UCHAR)
#define ioc_get_short(h,a,sb) (os_short)ioc_getx_int((h), (a), (sb), OS_SHORT)
#define ioc_get_ushort(h,a,sb) (os_ushort)ioc_getx_int((h), (a), (sb), OS_USHORT)
#define ioc_get_int(h,a,sb) (os_int)ioc_getx_int((h), (a), (sb), OS_INT)
#define ioc_get_uint(h,a,sb) (os_uint)ioc_getx_uint((h), (a), (sb), OS_UINT)
#define ioc_get_long(h,a,sb) ioc_getx_int((h), (a), (sb), OS_LONG)
#define ioc_get_str(h,a,st,ss) ioc_getx_str((h), (a), (st), (os_int)(ss), OS_STR)
#define ioc_get_float(h,a,sb) (os_float)ioc_getx_double((h), (a), (sb), OS_FLOAT);
#define ioc_get_double(h,a,sb) ioc_getx_double((h), (a), (sb), OS_DOUBLE);

/* Set or get a string using initialized signal structure.
 */
#define ioc_sets_str(s,st) ioc_moves_str((s), (st), 0, IOC_SIGNAL_WRITE|OS_STR)
#define ioc_gets_str(s,st,ss) ioc_moves_str((s), (st), (ss), OS_STR)

/* Set array of values without initialized signal structure..
 */
#define ioc_set_boolean_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_BOOLEAN)
#define ioc_set_char_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_CHAR)
#define ioc_set_uchar_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_UCHAR)
#define ioc_set_short_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_SHORT)
#define ioc_set_ushort_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_USHORT)
#define ioc_set_int_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_INT)
#define ioc_set_uint_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_UINT)
#define ioc_set_float_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_FLOAT)

/* Get array of values without initialized signal structure.
 */
#define ioc_get_boolean_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_BOOLEAN)
#define ioc_get_char_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_CHAR)
#define ioc_get_uchar_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_CHAR)
#define ioc_get_short_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_SHORT)
#define ioc_get_ushort_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_USHORT)
#define ioc_get_int_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_INT)
#define ioc_get_uint_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_UINT)
#define ioc_get_float_array(h,a,v,n) ioc_movex_array((h), (a), 0, (v), (n), OS_FLOAT)

/* Set or get array of values using initialized signal structure. Array type must match to JSON configuration.
 */
#define ioc_sets_array(s,v,n) ioc_moves_array((s), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE)
#define ioc_gets_array(s,v,n) ioc_moves_array((s), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT)

/* Shorter markings when we dot care to set state bits ourselves.
 */
#define ioc_sets_int0(s, v) ioc_sets_int((s), (v), OSAL_STATE_CONNECTED);
#define ioc_sets_double0(s, v) ioc_sets_double((s), (v), OSAL_STATE_CONNECTED);

/** 
****************************************************************************************************
  Memory block / signal releated functions
****************************************************************************************************
 */
/*@{*/

/* Read or write one or more signals to memory block.
 */
 void ioc_movex_signals(
    const iocSignal *signal,
    iocValue *vv,
    os_int n_signals,
    os_short flags);

/* Set integer value as a signal.
 */
os_char ioc_sets_int(
    const iocSignal *signal,
    os_long value,
    os_char state_bits);

/* Set floating point value as a signal.
 */
os_char ioc_sets_double(
    const iocSignal *signal,
    os_double value,
    os_char state_bits);

/* Set integer value as a signal.
 */
os_char ioc_setx_int(
    iocHandle *handle,
    os_int addr,
    os_long value,
    os_char state_bits,
    os_short flags);

/* Set floating point value as a signal.
 */
os_char ioc_setx_double(
    iocHandle *handle,
    os_int addr,
    os_double value,
    os_char state_bits,
    os_short flags);

/* Get integer signal value.
 */
os_long ioc_gets_int(
    const iocSignal *signal,
    os_char *state_bits);

/* Get floating point signal value.
 */
os_double ioc_gets_double(
    const iocSignal *signal,
    os_char *state_bits);

/* Get integer signal value.
 */
os_long ioc_getx_int(
    iocHandle *handle,
    os_int addr,
    os_char *state_bits,
    os_short flags);

/* Get floating point signal value.
 */
os_double ioc_getx_double(
    iocHandle *handle,
    os_int addr,
    os_char *state_bits,
    os_short flags);

/* Read or write string from/to memory block (with signal structure).
 */
os_char ioc_moves_str(
    const iocSignal *signal,
    os_char *str,
    os_memsz str_sz,
    os_char state_bits,
    os_short flags);

/* Read or write string from/to memory block (without signal structure).
 */
os_char ioc_movex_str(
    iocHandle *handle,
    os_int addr,
    os_char *str,
    os_memsz str_sz,
    os_char state_bits,
    os_short flags);

/* Read or write array from/to memory block (with signal structure).
 */
os_char ioc_moves_array(
    const iocSignal *signal,
    os_int offset,
    void *array,
    os_int n,
    os_char state_bits,
    os_short flags);

/* Read or write array from/to memory block (without signal structure).
 */
os_char ioc_movex_array(
    iocHandle *handle,
    os_int addr,
    os_int offset,
    void *array,
    os_int n,
    os_char state_bits,
    os_short flags);

/* Check if memory address range belongs to signal.
 */
os_boolean ioc_is_my_address(
    const iocSignal *signal,
    os_int start_addr,
    os_int end_addr);

/*@}*/

#endif
