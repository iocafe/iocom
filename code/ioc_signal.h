/**

  @file    ioc_signal.h
  @brief   Signals and memory blocks.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

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
   and may contain these additional flags. Flags >= 0x80 are used only as function arguments
   for options for the ioc_moves*, etc, functions, and not stored within iocSignal structure.
 */
#define IOC_SIGNAL_DEFAULT 0
#define IOC_SIGNAL_WRITE 0x20
#define IOC_SIGNAL_NO_THREAD_SYNC 0x80
#define IOC_SIGNAL_NO_TBUF_CHECK 0x100
#define IOC_SIGNAL_FLAGS_MASK 0xF80

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

/* Set or get a string using initialized signal structure.
 */
#define ioc_sets_str(s,st) ioc_move_str((s), (os_char*)(st), -1, OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE|OS_STR)
#define ioc_gets_str(s,st,ss) ioc_move_str((s), (st), (ss), OSAL_STATE_CONNECTED, OS_STR)

/* Set or get array of values using initialized signal structure. Array type must match to JSON configuration.
 */
#define ioc_sets_array(s,v,n) ioc_move_array((s), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE)
#define ioc_gets_array(s,v,n) ioc_move_array((s), 0, (v), (n), OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT)

/* Shorter markings when we dot care to set state bits ourselves.
 */
#define ioc_set(s, v) ioc_set_ext((s), (v), OSAL_STATE_CONNECTED)
#define ioc_get(s) ioc_get_ext((s), OS_NULL, IOC_SIGNAL_NO_TBUF_CHECK)
#define ioc_set_double(s, v) ioc_set_double_ext((s), (v), OSAL_STATE_CONNECTED)
#define ioc_get_double(s) ioc_get_double_ext((s), OS_NULL, IOC_SIGNAL_NO_TBUF_CHECK)


/**
****************************************************************************************************
  Memory block / signal releated functions
****************************************************************************************************
 */
/*@{*/

/* Read or write one or more signals to memory block.
 */
 void ioc_move(
    const iocSignal *signal,
    iocValue *vv,
    os_int n_signals,
    os_short flags);

/* Set integer signal.
 */
os_char ioc_set_ext(
    const iocSignal *signal,
    os_long value,
    os_char state_bits);

/* Set floating point signal.
 */
os_char ioc_set_double_ext(
    const iocSignal *signal,
    os_double value,
    os_char state_bits);

/* Get integer signal value.
 */
os_long ioc_get_ext(
    const iocSignal *signal,
    os_char *state_bits,
    os_short flags);

/* Get floating point signal value.
 */
os_double ioc_get_double_ext(
    const iocSignal *signal,
    os_char *state_bits,
    os_short flags);

/* Read or write string from/to memory block.
 */
os_char ioc_move_str(
    const iocSignal *signal,
    os_char *str,
    os_memsz str_sz,
    os_char state_bits,
    os_short flags);

/* Read or write array from/to memory block.
 */
os_char ioc_move_array(
    const iocSignal *signal,
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
