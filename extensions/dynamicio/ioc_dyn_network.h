/**

  @file    ioc_dyn_network.h
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic network structure organizes signals of one network. The dynamic configuration is
  used by server when IO devices are not known and must be accessed as plug and play.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_DYN_NETWORK_INCLUDED
#define IOC_DYN_NETWORK_INCLUDED
#if IOC_DYNAMIC_MBLK_CODE

struct iocMblkShortcut;

/** We use fixed hash table size for now. Memory use/performance ratio can be improved
    in futute by adopting hash table memory allocation to number of signals.
 */
#define IOC_DNETWORK_HASH_TAB_SZ 64


/**
****************************************************************************************************
  The dynamic network class structure.
****************************************************************************************************
*/
typedef struct iocDynamicNetwork
{
    os_char network_name[IOC_NETWORK_NAME_SZ];

    iocDynamicSignal *hash[IOC_DNETWORK_HASH_TAB_SZ];

    /* Set to TRUE when new dynamic network structure is allocated. Set to false, once
       application has been informed about the new network.
     */
    os_boolean new_network;

    struct iocDynamicNetwork *next;

    /** Two directional list of memory blocks handles belonging to this IO network.
     */
    struct iocMblkShortcut *mlist_first, *mlist_last;
}
iocDynamicNetwork;


/**
****************************************************************************************************
  Dynamic network information functions
****************************************************************************************************
*/
/* Allocate and initialize dynamic network object.
 */
iocDynamicNetwork *ioc_initialize_dynamic_network(
    void);

/* Release dynamic network object.
 */
void ioc_release_dynamic_network(
    iocDynamicNetwork *dnetwork);

/* Add a signal to dynamic information.
 */
iocDynamicSignal *ioc_add_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    const os_char *signal_name,
    const os_char *mblk_name,
    const os_char *device_name,
    os_uint device_nr,
    os_int addr,
    os_int n,
    os_int ncolumns,
    os_char flags);

/* Find a signal from dynamic information.
 */
iocDynamicSignal *ioc_find_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocIdentifiers *identifiers);

/* Delete all dynamic signal information related to a memory block.
 */
void ioc_network_mblk_is_deleted(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *mblk);

#endif
#endif
