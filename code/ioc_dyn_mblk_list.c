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
iocDynMBlkListItem *ioc_initialize_mblk_list_item(
    iocDynamicNetwork *dnetwork,
    iocHandle *mblk_handle)
{
    iocDynMBlkListItem *item;

    item = (iocDynMBlkListItem*)os_malloc(sizeof(iocDynMBlkListItem), OS_NULL);
    os_memclear(item, sizeof(iocDynMBlkListItem));

    ioc_duplicate_handle(&item->mblk_handle, mblk_handle);

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
void ioc_release_mblk_list_item(
    iocDynamicNetwork *dnetwork,
    iocDynMBlkListItem *item)
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

    os_free(item, sizeof(iocDynMBlkListItem));
}


#endif
