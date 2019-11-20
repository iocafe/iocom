/**

  @file    ioc_dyn_root.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic root holds data structure to managet information about IO networks and signals.
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
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/* Allocate and initialize dynamic root object.
 */
iocDynamicRoot *ioc_initialize_dynamic_root(
    void)
{
    iocDynamicRoot *droot;

    droot = (iocDynamicRoot*)os_malloc(sizeof(iocDynamicRoot), OS_NULL);
    os_memclear(droot, sizeof(iocDynamicRoot));
    return droot;
}

/* Release dynamic root structure.
   LOCK MUST BE ON.
 */
void ioc_release_dynamic_root(
    iocDynamicRoot *droot)
{
    iocDynamicNetwork *dnetwork, *next_dnetwork;
    os_int i;

    for (i = 0; i < IOC_DROOT_HASH_TAB_SZ; i++)
    {
        for (dnetwork = droot->hash[i];
             dnetwork;
             dnetwork = next_dnetwork)
        {
            next_dnetwork = dnetwork->next;
            ioc_release_dynamic_network(dnetwork);
        }
    }

    os_free(droot, sizeof(iocDynamicRoot));
}

/* Add a dynamic network.
 * LOCK must be on.
 */
iocDynamicNetwork *ioc_add_dynamic_network(
    iocDynamicRoot *droot,
    const os_char *network_name)
{
    iocDynamicNetwork *dnetwork, *prev_dnetwork;
    os_uint hash_ix;

    /* If we have existing IO network with this name,
       just return pointer to it.
     */
    hash_ix = ioc_hash(network_name) % IOC_DROOT_HASH_TAB_SZ;
    prev_dnetwork = OS_NULL;
    for (dnetwork = droot->hash[hash_ix];
         dnetwork;
         dnetwork = dnetwork->next)
    {
        if (!os_strcmp(network_name, dnetwork->network_name))
        {
            return dnetwork;
        }
        prev_dnetwork = dnetwork;
    }

    /* Allocate and initialize a new IO network object.
     */
    dnetwork = ioc_initialize_dynamic_network();
    os_strncpy(dnetwork->network_name, network_name, IOC_NETWORK_NAME_SZ);

    /* Join it as last to linked list for the hash index.
     */
    if (prev_dnetwork)
    {
        prev_dnetwork->next = dnetwork;
    }
    else
    {
        droot->hash[hash_ix] = dnetwork;
    }

    return dnetwork;
}

/* Remove a dynamic network.
   LOCK MUST BE ON.
 */
void ioc_remove_dynamic_network(
    iocDynamicRoot *droot,
    iocDynamicNetwork *dnetwork)
{
    iocDynamicNetwork *dn, *prev_dn;
    os_uint hash_ix;

    /* Fond out who has pointer to dnetwork in prev_dn.
     * If none, dnetwork is first in list and prev_dn is OS_NULL.
     */
    hash_ix = ioc_hash(dnetwork->network_name) % IOC_DROOT_HASH_TAB_SZ;
    prev_dn = OS_NULL;
    for (dn = droot->hash[hash_ix];
         dn && dn != dnetwork;
         dn = dn->next)
    {
        prev_dn = dn;
    }

    /* Remove from linked list.
     */
    if (prev_dn)
    {
        prev_dn->next = dnetwork->next;
    }
    else
    {
        droot->hash[hash_ix] = dnetwork->next;
    }

    /* Release the dynamic network object.
     */
    ioc_release_dynamic_network(dnetwork);
}


/* Find a dynamic network.
   LOCK MUST BE ON.
 */
iocDynamicNetwork *ioc_find_network(
    iocDynamicRoot *droot,
    const os_char *network_name)
{
    iocDynamicNetwork *dnetwork;
    os_uint hash_ix;

    hash_ix = ioc_hash(network_name) % IOC_DROOT_HASH_TAB_SZ;
    for (dnetwork = droot->hash[hash_ix];
         dnetwork;
         dnetwork = dnetwork->next)
    {
        if (!os_strcmp(network_name, dnetwork->network_name))
        {
            return dnetwork;
        }
    }

    return OS_NULL;
}


/* Called by ioc_release_memory_block(): memory block is being deleted, remove any references
   to it from dynamic configuration.
 */
void ioc_droot_mblk_is_deleted(
    iocDynamicRoot *droot,
    iocMemoryBlock *mblk)
{
    /* iocDynamicNetwork *dnetwork;
    dnetwork = ioc_find_network(iocDynamicRoot *droot, const os_char *network_name) */
}

/* Calculate hash index for the key
 */
os_uint ioc_hash(
    const os_char *key_str)
{
    const os_uint primes[] = {47, 2, 43, 3, 41, 5, 37, 7, 31, 11, 29, 13, 23, 17, 19};
    const os_int n_primes = sizeof(primes) / sizeof(os_uint);
    os_ulong hash_sum;
    os_int prime_ix;
    os_uchar c;

    hash_sum = 0;
    prime_ix = 0;
    while ((c = *(key_str++)))
    {
        hash_sum += c * primes[prime_ix];
        if (++prime_ix >= n_primes) prime_ix = 0;
    }

    return hash_sum;
}


#endif
