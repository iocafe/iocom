/**

  @file    ioc_dyn_mblk_list.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic list of memory block handles belonging to the IO network.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE


/* Allocate and initialize memory block list item.
 */
iocMblkShortcut *ioc_add_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *mblk)
{
    iocMblkShortcut *item;

    item = (iocMblkShortcut*)os_malloc(sizeof(iocMblkShortcut), OS_NULL);
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

/* Release memory block list item.
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

/* Find a shortcut to memory block.
 * LOCK MUST BE ON
 */
iocMemoryBlock *ioc_find_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    os_char *mblk_name,
    os_char *device_name,
    os_short device_nr)
{
    iocMblkShortcut *item, *next_item;
    iocMemoryBlock *mblk;

    for (item = dnetwork->mlist_first;
         item;
         item = next_item)
    {
        next_item = item->next;

        /* Clean up memory while searching
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
                    return mblk;
                }
            }
        }
    }

    return OS_NULL;
}

/* Remove memory block short cuts which are no longer needed.
 * LOCK MUST BE ON
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
