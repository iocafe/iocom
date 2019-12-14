/**

  @file    ioc_dyn_mblk_list.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    3.12.2019

  Dynamic list of memory block handles belonging to an IO device network. This list is used
  to seach for memory blocks without going trough all memory blocks belonging to the root.
  This is important in cloud server environment where one service may handle many thousands
  of memory blocks.

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

  @brief Allocate and initialize memory block list item.
  @anchor ioc_add_mblk_shortcut

  The ioc_add_mblk_shortcut() function adds a short cut to IO device network's quick search list.
  The added list item contains memory block handle, which links with memory block in such
  way that link becomes NULL is memory block is deleted. Synchronization ioc_lock() must be on
  when calling this function.

  @param   dnetwork Pointer to dynamic network structure, and root of the list.
  @return  Pointer to newly created shortcut, or OS_NULL if memory allocation failed.

****************************************************************************************************
*/
iocMblkShortcut *ioc_add_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *mblk)
{
    iocMblkShortcut *item;

    item = (iocMblkShortcut*)os_malloc(sizeof(iocMblkShortcut), OS_NULL);
    if (item == OS_NULL) return OS_NULL;
    os_memclear(item, sizeof(iocMblkShortcut));

    ioc_setup_handle(&item->mblk_handle, mblk->link.root, mblk);

    item->prev = dnetwork->mlist_last;
    if (item->prev)
    {
        item->prev->next = item;
    }
    else
    {
        dnetwork->mlist_first = item;
    }
    dnetwork->mlist_last = item;

    return item;
}


/**
****************************************************************************************************

  @brief Release memory block list item.
  @anchor ioc_release_mblk_shortcut
`
  The ioc_release_mblk_shortcut() function detaches shortcut from memory block and from memory
  block shortcut list and frees memory allocated for it. Synchronization ioc_lock() must be on
  when calling this function.

  @param   dnetwork Pointer to dynamic network structure, and root of the list.
  @param   item Pointer to memory block search shortcut to delete.
  @return  None

****************************************************************************************************
*/
void ioc_release_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    iocMblkShortcut *item)
{
    ioc_release_handle(&item->mblk_handle);

    if (item->prev)
    {
        item->prev->next = item->next;
    }
    else
    {
        dnetwork->mlist_first = item->next;
    }

    if (item->next)
    {
        item->next->prev = item->prev;
    }
    else
    {
        dnetwork->mlist_last = item->prev;
    }

    os_free(item, sizeof(iocMblkShortcut));
}


/**
****************************************************************************************************

  @brief Find memory block using shortcut list.
  @anchor ioc_find_mblk_shortcut

  The ioc_find_mblk_shortcut() function seaches for matching memory block using shortcut list.
  The memory block name, device name and device number must match exactly. Synchronization
  ioc_lock() must be on when calling this function.

  @param   dnetwork Pointer to dynamic network structure, holds the root of the shortcut list.
  @param   mblk_name Memory block name to search for.
  @param   device_name IO device name to search for.
  @param   device_nr IO device number to seach for.
  @return  Pointer to memory block handle within shortcut, or OS_NULL if there was no
           shortcut to matching memory block.

****************************************************************************************************
*/
iocHandle *ioc_find_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    const os_char *mblk_name,
    const os_char *device_name,
    os_short device_nr)
{
    iocMblkShortcut *item, *next_item;
    iocMemoryBlock *mblk;

    if (dnetwork == OS_NULL) return OS_NULL;

    for (item = dnetwork->mlist_first;
         item;
         item = next_item)
    {
        next_item = item->next;

        /* Delete shortcuts which no longer point to memory block while searching.
         */
        mblk = item->mblk_handle.mblk;
        if (mblk == OS_NULL)
        {
            ioc_release_mblk_shortcut(dnetwork, item);
        }
        else
        {
            if (device_nr == mblk->device_nr)
            {
                if (!os_strcmp(mblk_name, mblk->mblk_name) &&
                    !os_strcmp(device_name, mblk->device_name))
                {
                    return &item->mblk_handle;
                }
            }
        }
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Clean NULL shortcut items.
  @anchor ioc_find_mblk_shortcut

  The ioc_clean_mblk_shortcuts() function goes trough the shortcut list and removes shortcuts
  which do not any more point to a memory block (memory block has been deleted). Synchronization
  ioc_lock() must be on when calling this function.

  @param   dnetwork Pointer to dynamic network structure, holds the root of the shortcut list.
  @param   deleting_mblk Pointer to memory block currently being deleted. We can remove
           short cut for this one as well.
  @return  None.

****************************************************************************************************
*/
void ioc_clean_mblk_shortcuts(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *deleting_mblk)
{
    iocMblkShortcut *item, *next_item;
    iocMemoryBlock *mblk;

    for (item = dnetwork->mlist_first;
         item;
         item = next_item)
    {
        next_item = item->next;
        mblk = item->mblk_handle.mblk;
       if (mblk == OS_NULL || mblk == deleting_mblk)
        {
            ioc_release_mblk_shortcut(dnetwork, item);
        }
    }
}

#endif
