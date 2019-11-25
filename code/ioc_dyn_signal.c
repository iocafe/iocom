/**

  @file    ioc_dyn_signal.c
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
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/* Forward referred static functions.
 */
static void ioc_setup_signal(
    iocRoot *root,
    const os_char *iopath,
    const os_char *network_name,
    iocSignal *signal);


/* Allocate and initialize dynamic signal.
 */
iocDynamicSignal *ioc_initialize_dynamic_signal(
    const os_char *signal_name)
{
    iocDynamicSignal *dsignal;
    os_memsz sz;

    dsignal = (iocDynamicSignal*)os_malloc(sizeof(iocDynamicSignal), OS_NULL);
    os_memclear(dsignal, sizeof(iocDynamicSignal));

    sz = os_strlen(signal_name);
    dsignal->signal_name = os_malloc(sz, OS_NULL);
    os_memcpy(dsignal->signal_name, signal_name, sz);

    return dsignal;
}


/* Release dynamic signal.
 */
void ioc_release_dynamic_signal(
    iocDynamicSignal *dsignal)
{
    os_memsz sz;
    sz = os_strlen(dsignal->signal_name);
    os_free(dsignal->signal_name, sz);
    os_free(dsignal, sizeof(iocDynamicSignal));
}


/* Allocate or maintain dynamic signal structure.
 */
void ioc_new_signal(
    iocRoot *root,
    const os_char *iopath,
    const os_char *network_name,
    iocSignal **psignal)
{
    iocSignal *signal;
    iocHandle *handle;

    /* If we do not have signal structure, allocate it.
     */
    signal = *psignal;
    if (signal == OS_NULL)
    {
        signal = (iocSignal*)os_malloc(sizeof(iocSignal), OS_NULL);
        if (signal == OS_NULL) return;
        os_memclear(signal, sizeof(iocSignal));
        *psignal = signal;
    }

    /* If we do not have handle structure, allocate it.
     */
    handle = signal->handle;
    if (handle == OS_NULL)
    {
        handle = (iocHandle*)os_malloc(sizeof(iocHandle), OS_NULL);
        if (handle == OS_NULL) return;
        os_memclear(handle, sizeof(iocHandle));
        signal->handle = handle;
    }

    /* If we have already memoty block handle, we are good to go.
       We do not need to synchronize here. If the memory block was
       to be deleted between this point and actual read/write,
       the read/write will just fail.
     */
    if (handle->mblk) return;

    ioc_lock(root);
    ioc_setup_signal(root, iopath, network_name, signal);
    ioc_unlock(root);
}


/* Free signal allocated by ioc_new_signal() function.
   This function takes care about synchronization.
 */
void ioc_delete_signal(
    iocSignal *signal)
{
    /* Calling ioc_delete_signal() with NULL argument is fine, just nothing happens.
     */
    if (signal == OS_NULL) return;

    /* Release signal handle and free memory allocated for it. Notice that
       ioc_release_hanle() function takes care about synchronization.
     */
    if (signal->handle)
    {
        ioc_release_handle(signal->handle);
        os_free(signal->handle, sizeof(iocHandle));
    }

#if OSAL_DEBUG
    os_memclear(signal, sizeof(iocSignal));
#endif

    os_free(signal, sizeof(iocSignal));
}


/* Set up a dynamic signal.
 * LOCK must be on when calling this function.
 */
static void ioc_setup_signal(
    iocRoot *root,
    const os_char *iopath,
    const os_char *network_name,
    iocSignal *signal)
{
    iocIdentifiers identifiers;
    const os_char *topnet, *req_topnet;

    ioc_iopath_to_identifiers(&identifiers, iopath, IOC_EXPECT_SIGNAL);

    /* We do allow access between device networks, as long as these are subnets of the same
       top level network. This is useful to allow subnets in large IO networks. Care must be
       taken because here this could become a security vunerability.
     */
    topnet = os_strchr((os_char*)network_name, '.');
    topnet = topnet ? topnet + 1 : network_name;
    req_topnet = os_strchr(identifiers.network_name, '.');
    req_topnet = req_topnet ? req_topnet + 1 : req_topnet;
    if (os_strcmp(topnet, req_topnet))
    {
        os_strncpy(identifiers.network_name, network_name, IOC_NETWORK_NAME_SZ);
    }

    ioc_setup_signal_by_identifiers(root, &identifiers, signal);
}


/* Set up a dynamic signal.
 * LOCK must be on when calling this function.
 */
void ioc_setup_signal_by_identifiers(
    iocRoot *root,
    iocIdentifiers *identifiers,
    iocSignal *signal)
{
    iocDynamicNetwork *dnetwork;
    iocDynamicSignal *dsignal;
    iocMemoryBlock *mblk;

    if (root->droot == OS_NULL)
    {
        osal_debug_error("The application is not using dynamic network structure, root->dapp is NULL");
        return;
    }

    dnetwork = ioc_find_dynamic_network(root->droot, identifiers->network_name);
    if (dnetwork == OS_NULL) return;

    dsignal = ioc_find_first_dynamic_signal(dnetwork, identifiers);
    if (dsignal == OS_NULL) return;

    signal->addr = dsignal->addr;
    signal->n = dsignal->n;
    signal->flags = dsignal->flags;

    /* Check if we already have shortcut. This is much faster than
     * doing tough all memory blocks if there is many networks.
     */
    mblk = ioc_find_mblk_shortcut(dnetwork, dsignal->mblk_name,
        dsignal->device_name, dsignal->device_nr);
    if (mblk)
    {
        ioc_setup_handle(signal->handle, root, mblk);
        return;
    }

    /* Search trough all memory blocks. This will be slow if there are very many IO device networks,
     * that is why the shortcuts are in memory block list.
     */
    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        if (os_strcmp(mblk->network_name, identifiers->network_name)) continue;
        if (os_strcmp(mblk->device_name, dsignal->device_name)) continue;
        if (mblk->device_nr != dsignal->device_nr) continue;
        if (os_strcmp(mblk->mblk_name, dsignal->mblk_name)) continue;

        ioc_setup_handle(signal->handle, root, mblk);

        /* Add shortcut to memory block list for faster search
         */
        ioc_add_mblk_shortcut(dnetwork, mblk);
        break;
    }
}


#endif
