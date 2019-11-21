/**

  @file    ioc_dyn_network.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic network organizes signals of one network.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/* Allocate and initialize dynamic network object.
 */
iocDynamicNetwork *ioc_initialize_dynamic_network(
    void)
{
    iocDynamicNetwork *dnetwork;

    dnetwork = (iocDynamicNetwork*)os_malloc(sizeof(iocDynamicNetwork), OS_NULL);
    os_memclear(dnetwork, sizeof(iocDynamicNetwork));
    return dnetwork;
}


/* Release dynamic network object.
 */
void ioc_release_dynamic_network(
    iocDynamicNetwork *dnetwork)
{
    iocDynamicSignal *dsignal, *next_dsignal;
    os_int i;

    for (i = 0; i < IOC_DNETWORK_HASH_TAB_SZ; i++)
    {
        for (dsignal = dnetwork->hash[i];
             dsignal;
             dsignal = next_dsignal)
        {
            next_dsignal = dsignal->next;
            ioc_release_dynamic_signal(dsignal);
        }
    }

    os_free(dnetwork, sizeof(iocDynamicNetwork));
}


/* Add a dynamic signal.
   Notice that there can be multiple dynamic signals with same name.
    @param addr Starting address of the signal in memory block.

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
    iocHandle *mblk_handle)
{
    iocDynamicSignal *dsignal;
    os_uint hash_ix;

    /* Allocate and initialize a new IO network object.
     */
    dsignal = ioc_initialize_dynamic_signal(signal_name);

    /* Join it as last to linked list for the hash index.
     */
    hash_ix = ioc_hash(signal_name) % IOC_DNETWORK_HASH_TAB_SZ;
    dsignal->next = dnetwork->hash[hash_ix];
    dnetwork->hash[hash_ix] = dsignal;

    return dsignal;
}


/* Remove a dynamic signal.
 */
void ioc_remove_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocDynamicSignal *dsignal)
{
    iocDynamicSignal *ds, *prev_ds;
    os_uint hash_ix;

    /* Fond out who has pointer to dnetwork in prev_dn.
     * If none, dnetwork is first in list and prev_dn is OS_NULL.
     */
    hash_ix = ioc_hash(dsignal->signal_name) % IOC_DNETWORK_HASH_TAB_SZ;
    prev_ds = OS_NULL;
    for (ds = dnetwork->hash[hash_ix];
         ds && ds != dsignal;
         ds = ds->next)
    {
        prev_ds = ds;
    }

    /* Remove from linked list.
     */
    if (prev_ds)
    {
        prev_ds->next = dsignal->next;
    }
    else
    {
        dnetwork->hash[hash_ix] = dsignal->next;
    }

    /* Release the dynamic network object.
     */
    ioc_release_dynamic_signal(dsignal);
}


/* Find first matching dynamic signal.
   Notice that there can be multiple dynamic signals with same name.
 */
iocDynamicSignal *ioc_find_first_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocIdentifiers *identifiers)
{
    return ioc_find_next_dynamic_signal(dnetwork, OS_NULL, identifiers);
}


/* Find next matching dynamic signal.
   Notice that there can be multiple dynamic signals with same name.
 */
iocDynamicSignal *ioc_find_next_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocDynamicSignal *dsignal,
    iocIdentifiers *identifiers)
{
    os_uint hash_ix;

    while (OS_TRUE)
    {
        if (dsignal)
        {
            dsignal = dsignal->next;
        }
        else
        {
            hash_ix = ioc_hash(identifiers->signal_name) % IOC_DNETWORK_HASH_TAB_SZ;
            dsignal = dnetwork->hash[hash_ix];
        }
        if (dsignal == OS_NULL) break;

        if (!os_strcmp(identifiers->signal_name, dsignal->signal_name))
        {
            return dsignal;
        }
    }

    return OS_NULL;
}

/* Called by ioc_release_memory_block(): memory block is being deleted, remove any references
   to it from dynamic configuration.
 */
void ioc_network_mblk_is_deleted(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *mblk)
{
    iocDynamicSignal *dsignal;
    os_int i;

    for (i = 0; i < IOC_DNETWORK_HASH_TAB_SZ; i++)
    {
        for (dsignal = dnetwork->hash[i];
             dsignal;
             dsignal = dsignal->next)
        {
            if (dsignal->x.handle->mblk == mblk)
            {
                dsignal->x.handle = OS_NULL;
            }
        }
    }
}

#endif
