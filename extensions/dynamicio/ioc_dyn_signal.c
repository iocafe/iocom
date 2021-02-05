/**

  @file    ioc_dyn_signal.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

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


/**
****************************************************************************************************

  @brief Allocate and initialize dynamic signal structure.
  @anchor ioc_initialize_dynamic_signal

  The ioc_initialize_dynamic_signal() function allocates new dynamic signal structure and
  stores signal name for it. This function doesn't join allocated structure to any top
  level structure.

  @param   signal_name Name of the new signal.
  @return  Pointer to dynamic signal structure, or OS_NULL if memory allocation failed.

****************************************************************************************************
*/
iocDynamicSignal *ioc_initialize_dynamic_signal(
    const os_char *signal_name)
{
    iocDynamicSignal *dsignal;
    os_memsz sz;

    dsignal = (iocDynamicSignal*)os_malloc(sizeof(iocDynamicSignal), OS_NULL);
    if (dsignal == OS_NULL) return OS_NULL;
    os_memclear(dsignal, sizeof(iocDynamicSignal));

    sz = os_strlen(signal_name);
    dsignal->signal_name = os_malloc(sz, OS_NULL);
    if (dsignal->signal_name == OS_NULL)
    {
        os_free(dsignal, sizeof(iocDynamicSignal));
        return OS_NULL;
    }
    os_memcpy(dsignal->signal_name, signal_name, sz);

    return dsignal;
}


/**
****************************************************************************************************

  @brief Release dynamic signal structure.
  @anchor ioc_release_dynamic_signal

  The ioc_release_dynamic_signal() function frees memory allocated for dynamic signal structure
  and signal name.

  @param   dsignal Pointer to dynamic signal structure to release.
  @return  None.

****************************************************************************************************
*/
void ioc_release_dynamic_signal(
    iocDynamicSignal *dsignal)
{
    os_memsz sz;

    sz = os_strlen(dsignal->signal_name);
    os_free(dsignal->signal_name, sz);
    os_free(dsignal, sizeof(iocDynamicSignal));
}


/**
****************************************************************************************************

  @brief Maintain or allocate signal structure.
  @anchor ioc_maintain_signal

  The ioc_maintain_signal() function allocates signal structure and container memory block handle
  structre, unless already allocated. The function fills these by by data (memory block handle,
  address, n, type, etc) by searching the dynamic information.

  If dynamic information for the signal is not (yet) available, the signal structure is
  left uninitialized and signal->handle->mblk will be OS_NULL.

  @param   root IOCOM root structure.
  @param   iopath IO path to the signal, like "lighton.imp.mydev.cafenet", etc.
  @param   network_name Network name can also be given separate from IO path, for example
           "cafenet". Sometimes this is convinient.
  @param   psignal Pointer to signal pointer. At entry, the signal pointer can be either
           OS_NULL and signal structure will be allocated, or pointer to already allocated
           signal structure. At exit this is set to pointer to signal structure, unless
           memory allocation fails.
  @return  None.

****************************************************************************************************
*/
void ioc_maintain_signal(
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


/**
****************************************************************************************************

  @brief Release signal structure allocated by ioc_maintain_signal().
  @anchor ioc_delete_signal

  The ioc_delete_signal() function frees signal and contained handle structures allocated
  by ioc_maintain_signal() function.

  @param   signal Pointer to signal structure.
  @return  None.

****************************************************************************************************
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


/**
****************************************************************************************************

  @brief Set up a signal.
  @anchor ioc_setup_signal

  The ioc_delete_signal() function is helper function for ioc_maintain_signal() to fill in
  the signal structure.

  LOCK must be on when calling this function.

  @param   signal Pointer to signal structure.
  @return  None.

****************************************************************************************************
*/
static void ioc_setup_signal(
    iocRoot *root,
    const os_char *iopath,
    const os_char *network_name,
    iocSignal *signal)
{
    iocIdentifiers identifiers;
    const os_char *topnet, *req_topnet;

    ioc_iopath_to_identifiers(root, &identifiers, iopath, IOC_EXPECT_SIGNAL);

    /* We do allow access between device networks, as long as these are subnets of the same
       top level network. This is useful to allow subnets in large IO networks. Care must be
       taken because here this could become a security vunerability.
     */
    if (network_name) if (*network_name != '\0')
    {
        topnet = os_strchr((os_char*)network_name, '.');
        topnet = topnet ? topnet + 1 : network_name;
        req_topnet = os_strchr(identifiers.network_name, '.');
        req_topnet = req_topnet ? req_topnet + 1 : req_topnet;
        if (os_strcmp(topnet, req_topnet))
        {
            os_strncpy(identifiers.network_name, network_name, IOC_NETWORK_NAME_SZ);
        }
    }

    ioc_setup_signal_by_identifiers(root, &identifiers, signal);
}


/**
****************************************************************************************************

  @brief Set up a signal structure, if we have dynamic information for it.
  @anchor ioc_setup_signal_by_identifiers

  The ioc_setup_signal_by_identifiers() function searces for signal from dynamic information
  with given identifiers. If one is found, data is stored in signal and contained handle stuctures.

  If dynamic information for the signal is not (yet) available, the signal structure is
  left uninitialized and signal->handle->mblk will be OS_NULL.

  LOCK must be on when calling this function.

  @param   root IOCOM root structure.
  @param   identifiers Identifiers from IO path, specify which signal.
  @param   signal Pointer to signal signal structure to set up. At entry, the signal pointer can be either
           OS_NULL and signal structure will be allocated, or pointer to already allocated
           signal structure. At exit this is set to pointer to signal structure, unless
           memory allocation fails.
  @return  None.

****************************************************************************************************
*/
iocDynamicSignal *ioc_setup_signal_by_identifiers(
    iocRoot *root,
    iocIdentifiers *identifiers,
    iocSignal *signal)
{
    iocDynamicNetwork *dnetwork;
    iocDynamicSignal *dsignal;
    iocMemoryBlock *mblk;
    iocHandle *handle;

    if (root->droot == OS_NULL)
    {
        osal_debug_error("The application is not using dynamic network structure, root->dapp is NULL");
        return OS_NULL;
    }

    dnetwork = ioc_find_dynamic_network(root->droot, identifiers->network_name);
    if (dnetwork == OS_NULL) return OS_NULL;

    dsignal = ioc_find_dynamic_signal(dnetwork, identifiers);
    if (dsignal == OS_NULL) return OS_NULL;

    signal->addr = dsignal->addr;
    signal->n = dsignal->n;
    signal->flags = dsignal->flags;

    /* If we already got handle resolved by another signal using same handle, no need to redo.
     */
    if (signal->handle->mblk) return dsignal;

    /* Check if we already have shortcut. This is much faster than
     * doing tough all memory blocks if there is many networks.
     */
    handle = ioc_find_mblk_shortcut(dnetwork, dsignal->mblk_name,
        dsignal->device_name, dsignal->device_nr);
    if (handle)
    {
        ioc_release_handle(signal->handle);
        ioc_setup_handle(signal->handle, root, handle->mblk);
        return dsignal;
    }

    /* Search trough all memory blocks. This will be slow if there are very many IO device networks,
     * that is why the shortcuts are in memory block list.
     */
    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
        if (os_strcmp(mblk->network_name, identifiers->network_name)) continue;
        if (os_strcmp(mblk->device_name, dsignal->device_name)) continue;
        if (mblk->device_nr != dsignal->device_nr) continue;
#endif
        if (os_strcmp(mblk->mblk_name, dsignal->mblk_name)) continue;

        ioc_release_handle(signal->handle);
        ioc_setup_handle(signal->handle, root, mblk);

        /* Add shortcut to memory block list for faster search
         */
        ioc_add_mblk_shortcut(dnetwork, mblk);
        return dsignal;
    }

    return OS_NULL;
}


#endif
