/**

  @file    ioc_dyn_signal.h
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic signal is extended signal structure, which is part of dynamic IO network
  information.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_DYN_SIGNAL_INCLUDED
#define IOC_DYN_SIGNAL_INCLUDED
#if IOC_DYNAMIC_MBLK_CODE


struct iocDynamicNetwork;


typedef struct iocDynamicSignal
{
    /** Signal name. Dynamically allocated, can be up to 31 characters.
     */
    os_char *signal_name;

    /** Memory block name, max 15 characters.
     */
    os_char mblk_name[IOC_NAME_SZ];

    /** Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL".
     */
    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in
        context as device name, like "TEMPCTRL1".
     */
    os_short device_nr;

    /** One of: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT, OS_INT, OS_UINT, OS_FLOAT
        or OS_STR. Flag bit IOC_PIN_PTR marks that ptr is "Pin *" pointer.
     */
    os_char flags;

    /** Pointer to dynamic network, can be used for network name.
     */
    struct iocDynamicNetwork *dnetwork;

    /** Starting address in memory block.
     */
    os_int addr;

    /** For strings n can be number of bytes in memory block for the string. For arrays n is
        number of elements reserved in memory block. Either 0 or 1 for single variables.
     */
    os_int n;

    /** If array represents matrix, the width is number of columns in the matrix.
        If vector or sigle variable, ncolumns is 1.
     */
    os_int ncolumns;

    /** Next dynamic signal with same hash key.
     */
    struct iocDynamicSignal *next;
}
iocDynamicSignal;


/* Allocate and initialize dynamic signal.
 */
iocDynamicSignal *ioc_initialize_dynamic_signal(
    const os_char *signal_name);

/* Release dynamic signal.
 */
void ioc_release_dynamic_signal(
    iocDynamicSignal *dsignal);

/* Allocate or maintain signal structure using IO path.
 */
void ioc_new_signal(
    iocRoot *root,
    const os_char *iopath,
    const os_char *network_name,
    iocSignal **psignal);

/* Allocate and initialize dynamic signal using identifiers structure.
 */
iocDynamicSignal *ioc_setup_signal_by_identifiers(
    iocRoot *root,
    iocIdentifiers *identifiers,
    iocSignal *signal);

/* Free signal allocated by ioc_new_signal() function.
 */
void ioc_delete_signal(
    iocSignal *signal);

#endif
#endif
