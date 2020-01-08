/**

  @file    ioc_dyn_root.h
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic root holds data structure to manage information about IO networks and signals.
  It is used to convert io path (signal name, memory block name, device name and number, network
  name) to IO signal object pointers, or to memory block pointers.

  An IO path can be split to individual identifiers by ioc_iopath_to_identifiers() function.
  The network name and signal name are used as hash keys, since these are known explisitely
  by application and are efficient for the purpose.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#if IOC_DYNAMIC_MBLK_CODE

struct iocDynamicRoot;
struct iocDynamicNetwork;


/**
****************************************************************************************************
    Dynamic root structure.
****************************************************************************************************
*/

/** We use fixed hash table size for now. Memory use/performance ratio can be improved
    in futute by adopting hash table memory allocation to number of signals.
 */
#define IOC_DROOT_HASH_TAB_SZ 128

/** The dynamic root class structure.
 */
typedef struct iocDynamicRoot
{
    iocDynamicNetwork *hash[IOC_DROOT_HASH_TAB_SZ];

    /** Pointer back to root object.
     */
    iocRoot *root;
}
iocDynamicRoot;


/**
****************************************************************************************************
    Dynamic network configuration root functions.
****************************************************************************************************
*/
/* Allocate and initialize dynamic root object.
 */
iocDynamicRoot *ioc_initialize_dynamic_root(
    iocRoot *root);

/* Release dynamic root structure.
 */
void ioc_release_dynamic_root(
    iocDynamicRoot *droot);

/* Add an IO device network to dynamic information.
 */
iocDynamicNetwork *ioc_add_dynamic_network(
    iocDynamicRoot *droot,
    const os_char *network_name);

/* Remove a dynamic network.
 */
void ioc_remove_dynamic_network(
    iocDynamicRoot *droot,
    iocDynamicNetwork *dnetwork);

/* Find a dynamic network.
 */
iocDynamicNetwork *ioc_find_dynamic_network(
    iocDynamicRoot *droot,
    const os_char *network_name);

/* Add dynamic memory block/signal information.
 */
osalStatus ioc_add_dynamic_info(
    iocHandle *mblk_handle,
    os_boolean resize_mblks);

/* Memory block is being deleted, remove any references to it from dynamic configuration.
 */
void ioc_droot_mblk_is_deleted(
    iocDynamicRoot *droot,
    iocMemoryBlock *mblk);

/* Calculate hash index for the key.
 */
os_uint ioc_hash(
    const os_char *key_str);

#endif
