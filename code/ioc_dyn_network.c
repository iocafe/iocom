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

    ioc_free_dynamic_mblk_list(dnetwork);


    os_free(dnetwork, sizeof(iocDynamicNetwork));
}


/* Free list of memory blocks in this network
 */
void ioc_free_dynamic_mblk_list(
    iocDynamicNetwork *dnetwork)
{
    while (dnetwork->mlist_first)
    {
        ioc_release_mblk_shortcut(dnetwork, dnetwork->mlist_first);
    }
}


/* Add a dynamic signal.
   Notice that there can be multiple dynamic signals with same name. !!!!!!!!!!!!!!!!!!!!!!!!!!!! MISSING
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
    const os_char *mblk_name,
    const os_char *device_name,
    short device_nr,
    os_int addr,
    os_ushort n,
    os_char flags,
    iocHandle *mblk_handle)
{
    iocDynamicSignal *dsignal, *prev_dsignal;
    os_uint hash_ix;

    /* If we have existing IO network with this name,
       just return pointer to it.
     */
    hash_ix = ioc_hash(signal_name) % IOC_DNETWORK_HASH_TAB_SZ;
    prev_dsignal = OS_NULL;
    for (dsignal = dnetwork->hash[hash_ix];
         dsignal;
         dsignal = dsignal->next)
    {
        if (!os_strcmp(signal_name, dsignal->signal_name))
        {
            return dsignal;
        }
        prev_dsignal = dsignal;
    }

    /* Allocate and initialize a new IO network object.
     */
    dsignal = ioc_initialize_dynamic_signal(signal_name);
    dsignal->dnetwork = dnetwork;
    os_strncpy(dsignal->mblk_name, mblk_name, IOC_NAME_SZ);
    os_strncpy(dsignal->device_name, device_name, IOC_NAME_SZ);
    dsignal->device_nr = device_nr;
    dsignal->addr = addr;
    dsignal->n = n;
    dsignal->flags = flags;

    /* Join it as last to linked list for the hash index.
     */
    if (prev_dsignal)
    {
        prev_dsignal->next = dsignal;
    }
    else
    {
        dnetwork->hash[hash_ix] = dsignal;
    }

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
   Notice that there can be multiple signals with same set of identifiers.
 */
iocDynamicSignal *ioc_find_first_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocIdentifiers *identifiers)
{
    return ioc_find_next_dynamic_signal(dnetwork, OS_NULL, identifiers);
}


/* Find next matching dynamic signal.
   Notice that there can be multiple signals with same set of identifiers.
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
            if (identifiers->mblk_name[0] != '\0')
            {
                if (os_strcmp(identifiers->mblk_name, dsignal->mblk_name)) continue;
            }
            if (identifiers->device_name[0] != '\0')
            {
                if (os_strcmp(identifiers->device_name, dsignal->device_name)) continue;
            }
            if (identifiers->device_nr)
            {
                if (identifiers->device_nr != dsignal->device_nr) continue;
            }

            return dsignal;
        }
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Delete all dynamic signal information related to a memory block.
  @anchor ioc_network_mblk_is_deleted

  The ioc_network_mblk_is_deleted() is called when a memory block is about to be deleted from
  the IO device network. It deletes all dynamic signal information for signals in that
  memory block.

  Root lock must be on when calling this function.

  @param   dnetwork Pointer to dynamic network object, to which memory block belongs to.
  @param   mblk Pointer to memory block object being deleted.
  @return  None.

****************************************************************************************************
*/
void ioc_network_mblk_is_deleted(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *mblk)
{
    iocRoot *root;
    iocDynamicSignal *dsignal, *prev_dsignal, *next_dsignal;
    os_int i;

    for (i = 0; i < IOC_DNETWORK_HASH_TAB_SZ; i++)
    {
        prev_dsignal = OS_NULL;
        for (dsignal = dnetwork->hash[i];
             dsignal;
             dsignal = next_dsignal)
        {
            next_dsignal = dsignal->next;

            if (!os_strcmp(dsignal->mblk_name, mblk->mblk_name) &&
                !os_strcmp(dsignal->device_name, mblk->device_name) &&
                dsignal->device_nr == mblk->device_nr)
            {
                if (prev_dsignal) prev_dsignal->next = dsignal->next;
                else dnetwork->hash[i] = dsignal->next;

                ioc_release_dynamic_signal(dsignal);
            }
            else
            {
                prev_dsignal = dsignal;
            }
        }
    }

    /* Remove memory block short cuts which are no longer needed.
     */
    ioc_clean_mblk_shortcuts(dnetwork, mblk);

osal_debug_error("HERE1");
    /* If this was the last memory block of the network.
     */
    if (dnetwork->mlist_first == OS_NULL)
    {
        root = mblk->link.root;
        if (root) if (root->droot)
        {
            ioc_remove_dynamic_network(root->droot, dnetwork);
osal_debug_error("HERE2");
        }
    }
}

#endif
