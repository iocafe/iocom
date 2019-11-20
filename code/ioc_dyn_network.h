/**

  @file    ioc_dyn_network.h
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic network organizes signals of one network .

  An IO path can be split to individual identifiers by ioc_iopath_to_identifiers() function.
  The network name and signal name are used as hash keys, since these are known explisitely
  by application and are efficient for the purpose.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_DYN_NETWORK_INCLUDED
#define IOC_DYN_NETWORK_INCLUDED
#if IOC_DYNAMIC_MBLK_CODE


/** We use fixed hash table size for now. Memory use/performance ratio can be improved
    in futute by adopting hash table memory allocation to number of signals.
 */
#define IOC_DNETWORK_HASH_TAB_SZ 64


/** The dynamic network class structure.
 */
typedef struct iocDynamicNetwork
{
    os_char network_name[IOC_NETWORK_NAME_SZ];

    iocDynamicSignal *hash[IOC_DNETWORK_HASH_TAB_SZ];

    struct iocDynamicNetwork *next;
}
iocDynamicNetwork;


/* Allocate and initialize dynamic network object.
 */
iocDynamicNetwork *ioc_initialize_dynamic_network(void);

/* Release dynamic network object.
 */
void ioc_release_dynamic_network(
    iocDynamicNetwork *dnetwork);

/* Add a dynamic signal.
 */
    /** @param addr Starting address of the signal in memory block.

        @param n For strings n can be number of bytes in memory block for the string. For arrays n is
        number of elements reserved in memory block. Either 0 or 1 for single variables.
        Unsigned type used for reason, we want to have max 65535 items.

        @param flags: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT, OS_INT, OS_UINT, OS_FLOAT
        or OS_STR.
     */
iocDynamicSignal *ioc_add_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    const os_char *signal_name,
    os_int addr,
    os_ushort n,
    os_char flags,
    iocHandle *mblk_handle);

/* Remove a dynamic signal.
 */
void ioc_remove_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocDynamicSignal *dsignal);

/* Find a dynamic signal.
 */
iocDynamicSignal *ioc_find_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocIdentifiers *identifiers); /* should we have identifiers as separate arguments ? */

#endif
#endif
