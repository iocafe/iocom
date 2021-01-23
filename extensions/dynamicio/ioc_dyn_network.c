/**

  @file    ioc_dyn_network.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  The dynamic network structure organizes signals of one network. The dynamic configuration is
  used by server when IO devices are not known and must be accessed as plug and play.
  - Python API (iocompython) uses always dynamic approach to configuration.
  - server/controller based on C code can use either dynamic or static configuration can be used.
    Dynamic is more flexible to accept unknown IO device types, static implementation is faster
    and less resource intensive.
  - IO boards/devices should almost always use static approach. Exception is IO device implemented
    in Python. Python is already so resource intensive, that usin dynamic code doesn't change that.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/**
****************************************************************************************************

  @brief Allocate and clear dynamic network object.
  @anchor ioc_initialize_dynamic_network

  The ioc_initialize_dynamic_network() function just allocates clear memory for dynamic IO device
  network structure. This function is called by ioc_add_dynamic_network(), which is used to
  initiate storing information about new dynamic network.

  @return  Pointer to newly allocated dynamic network structure, or OS_NULL if memory allocation
           failed..

****************************************************************************************************
*/
iocDynamicNetwork *ioc_initialize_dynamic_network(
    void)
{
    iocDynamicNetwork *dnetwork;

    dnetwork = (iocDynamicNetwork*)os_malloc(sizeof(iocDynamicNetwork), OS_NULL);
    if (dnetwork)
    {
        os_memclear(dnetwork, sizeof(iocDynamicNetwork));
    }
    return dnetwork;
}


/**
****************************************************************************************************

  @brief Release dynamic network object.
  @anchor ioc_release_dynamic_network

  The ioc_release_dynamic_network() function releases memory allocated for dynamic IO network
  structure, and memory allocated for dynamic IO signal and memory block shortcuts.
  Synchronization ioc_lock() must be on when this function is called.

  @param   dnetwork Pointer to dynamic network structure to release. If OS_NULL, the function
           does nothing.
  @return  None.

****************************************************************************************************
*/
void ioc_release_dynamic_network(
    iocDynamicNetwork *dnetwork)
{
    iocDynamicSignal *dsignal, *next_dsignal;
    os_int i;

    if (dnetwork == OS_NULL) return;

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

    while (dnetwork->mlist_first)
    {
        ioc_release_mblk_shortcut(dnetwork, dnetwork->mlist_first);
    }

    os_free(dnetwork, sizeof(iocDynamicNetwork));
}


/**
****************************************************************************************************

  @brief Add a dynamic signal information.
  @anchor ioc_add_dynamic_signal

  The ioc_add_dynamic_signal() function adds information about a signal based on "info" memory
  block configuration for the IO device network. If signal already exists, function jsut returns
  pointer to it. Synchronization ioc_lock() must be on when this function is called.

  @param   dnetwork Pointer to dynamic network structure.
  @param   signal_name Signal name.
  @param   mblk_name Memory block name.
  @param   device_name Device name.
  @param   device_nr Device number, if there are several same kind of IO devices, they must
           have different numbers.
  @param   addr Starting address of the signal in memory block.

  @param   n For strings n can be number of bytes in memory block for the string. For arrays n is
           number of elements reserved in memory block. Use value 1 for single variables.
  @param   ncolumns If a matrix of data is stored as an array, number of matrix columns.
           Otherwise value 1.
  @param   flags: OS_BOOLEAN, OS_CHAR, OS_UCHAR, OS_SHORT, OS_USHORT, OS_INT, OS_UINT,
           OS_LONG, OS_FLOAT, OS_DOUBLE, or OS_STR.

  @return  Pointer to dynamic signal. OS_NULL if memory allocation failed.

****************************************************************************************************
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
    os_char flags)
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
            if (!os_strcmp(mblk_name, dsignal->mblk_name) &&
                !os_strcmp(device_name, dsignal->device_name) &&
                device_nr == dsignal->device_nr)
            {
                return dsignal;
            }
        }

        prev_dsignal = dsignal;
    }

    /* Allocate and initialize a new IO network object.
     */
    dsignal = ioc_initialize_dynamic_signal(signal_name);
    if (dsignal == OS_NULL) return OS_NULL;
    dsignal->dnetwork = dnetwork;
    os_strncpy(dsignal->mblk_name, mblk_name, IOC_NAME_SZ);
    os_strncpy(dsignal->device_name, device_name, IOC_NAME_SZ);
    dsignal->device_nr = device_nr;
    dsignal->addr = addr;
    dsignal->n = n;
    dsignal->ncolumns = ncolumns;
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


/**
****************************************************************************************************

  @brief Find matching dynamic signal.
  @anchor ioc_find_dynamic_signal

  The ioc_find_dynamic_signal() function finds first dynamic signal matching to given identifiers.
  Synchronization ioc_lock() must be on when this function is called.

  @param   dnetwork Pointer to dynamic network structure.
  @param   intentifiers Identifiers like signal name, memory block name, device name and number,
           network name.
  @return  None.

****************************************************************************************************
*/
iocDynamicSignal *ioc_find_dynamic_signal(
    iocDynamicNetwork *dnetwork,
    iocIdentifiers *identifiers)
{
    os_uint hash_ix;
    iocDynamicSignal *dsignal = OS_NULL;

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

    root = mblk->link.root;

    /* If this is info block, device is being disconnected.
     */
    if (!os_strcmp(mblk->mblk_name, "info") && root)
    {
        ioc_new_root_event(root, IOC_DEVICE_DISCONNECTED, dnetwork, mblk, root->callback_context);
    }

    for (i = 0; i < IOC_DNETWORK_HASH_TAB_SZ; i++)
    {
        prev_dsignal = OS_NULL;
        for (dsignal = dnetwork->hash[i];
             dsignal;
             dsignal = next_dsignal)
        {
            next_dsignal = dsignal->next;

#if IOC_MBLK_SPECIFIC_DEVICE_NAME
            if (!os_strcmp(dsignal->mblk_name, mblk->mblk_name) &&
                !os_strcmp(dsignal->device_name, mblk->device_name) &&
                dsignal->device_nr == mblk->device_nr)
#else
            if (!os_strcmp(dsignal->mblk_name, mblk->mblk_name) &&
                !os_strcmp(dsignal->device_name, root->device_name) &&
                dsignal->device_nr == root->device_nr)
#endif
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

    /* If this was the last memory block of the network.
     */
    if (dnetwork->mlist_first == OS_NULL)
    {
        if (root) if (root->droot)
        {
            ioc_remove_dynamic_network(root->droot, dnetwork);
        }
    }
}

#endif
