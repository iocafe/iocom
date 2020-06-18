/**

  @file    ioc_signal_addr.c
  @brief   Signal address related functions
  @author  Pekka Lehtikoski
  @version 1.0
  @date    16.6.2020

  Helper functions for implementing communication callback. These function check
  signal address.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/**
****************************************************************************************************

  @brief Get number of bytes needed to store the signal.
  @anchor ioc_nro_signal_bytes

  The ioc_nro_signal_bytes() function clalculates how many bytes are needed for the signal
  within memory block.

  @param   signal Pointer to signal structure.
  @return  Number of memory block bytes needed to store this signal.

****************************************************************************************************
*/
os_int ioc_nro_signal_bytes(
    const struct iocSignal *signal)
{
    os_int n;
    osalTypeId type_id;

    type_id = signal->flags & OSAL_TYPEID_MASK;

    n = signal->n;
    if (n < 1) n = 1;

    switch (type_id)
    {
        case OS_STR:
            n++;
            break;

        case OS_BOOLEAN:
            if (n > 1)
            {
                n = (n + 7) / 8 + 1;
            }
            break;

        default:
            n = n * (os_int)osal_type_size(type_id) + 1;
            break;
    }

    return n;
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
    const struct iocSignal *signal,
    os_int start_addr,
    os_int end_addr)
{
    os_int addr;

    addr = signal->addr;
    if (end_addr < addr) return OS_FALSE;
    return (os_boolean)(start_addr < addr + ioc_nro_signal_bytes(signal));
}


/**
****************************************************************************************************

  @brief Which signals are effected by changes in memory address range.
  @anchor ioc_get_signal_range_by_hdr

  The ioc_get_signal_range_by_hdr() function gets range of signals that are at least partly within
  start, end address range given as argument. This is log N algorithm.

  @param   hdr Pointer to signal header for the memory block.
  @param   start_addr First address.
  @param   end_addr Last address.
  @param   n_signal Pointer to integer where to store number of effected signals.
  @return  Pointer to first effected signal. OS_NULL if no signals effected.

****************************************************************************************************
*/
const iocSignal *ioc_get_signal_range_by_hdr(
    const iocMblkSignalHdr *hdr,
    os_int start_addr,
    os_int end_addr,
    os_int *n_signals)
{
    const iocSignal *sig;
    os_int first, last, mid, signal_end;

    osal_debug_assert(hdr);
    sig = hdr->first_signal;
    osal_debug_assert(sig);

    first = 0;
    last = hdr->n_signals - 1;

    /* If address is outside signals range
     */
    if (end_addr < sig[0].addr ||
        start_addr > sig[last].addr + ioc_nro_signal_bytes(sig + last) - 1)
    {
        *n_signals = 0;
        return OS_NULL;
    }

    /* First first effected signal
     */
    while (last > first + 1)
    {
        mid = (first + last) / 2;
        if (start_addr < sig[mid].addr) {
            last = mid - 1;
        }
        else {
            signal_end = sig[mid].addr + ioc_nro_signal_bytes(sig + mid) - 1;
            if (start_addr > signal_end) {
                first = mid + 1;
            }
            else {
                first = last = mid;
                break;
            }
        }
    }

    if (start_addr >= sig[last].addr) {
        first = last;
    }

    /* Search last signal only part of signal list, which follows first
     */
    sig += first;
    first = 0;
    last = hdr->n_signals - first - 1;

    /* First last effected signal
     */
    while (last > first + 1)
    {
        mid = (first + last) / 2;
        if (end_addr < sig[mid].addr) {
            last = mid - 1;
        }
        else
        {
            signal_end = sig[mid].addr + ioc_nro_signal_bytes(sig + mid) - 1;
            if (end_addr > signal_end) {
                first = mid + 1;
            }
            else {
                first = last = mid;
                break;
            }
        }
    }

    if (end_addr >= sig[last].addr) {
        first = last;
    }

    /* If "hole" in signal memory?
     */
    if (first == 0) {
        signal_end = sig[0].addr + ioc_nro_signal_bytes(sig) - 1;
        if (end_addr < sig[0].addr || start_addr > signal_end) {
            *n_signals = 0;
            return OS_NULL;
        }
    }

    *n_signals = first + 1;
    return sig;
}



/**
****************************************************************************************************

  @brief Which signals are effected by changes in memory address range.
  @anchor ioc_get_signal_range

  The ioc_get_signal_range() function gets range of signals that are at least partly within
  start, end address range given as argument. This is log N algorithm.

  @param   handle Memory block handle.
  @param   start_addr First address.
  @param   end_addr Last address.
  @param   n_signal Pointer to integer where to store number of effected signals.
  @return  Pointer to first effected signal. OS_NULL if no signals effected.

****************************************************************************************************
*/
const iocSignal *ioc_get_signal_range(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_int *n_signals)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    const iocSignal *sig = OS_NULL;

    *n_signals = 0;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return OS_NULL;

    if (mblk->signal_hdr) {
        sig = ioc_get_signal_range_by_hdr(mblk->signal_hdr, start_addr, end_addr, n_signals);
    }

    /* End syncronization.
     */
    ioc_unlock(root);
    return sig;
}
