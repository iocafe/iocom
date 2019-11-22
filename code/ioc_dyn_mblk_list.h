/**

  @file    ioc_dyn_mblk_list.h
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
#ifndef IOC_DYN_MBLK_LIST_INCLUDED
#define IOC_DYN_MBLK_LIST_INCLUDED
#if IOC_DYNAMIC_MBLK_CODE

/* Item in memory bloc list
 */
typedef struct iocDynMBlkListItem
{
    iocHandle mblk_handle;

    struct iocDynMBlkListItem *next;
    struct iocDynMBlkListItem *prev;
}
iocDynMBlkListItem;


/* Allocate and initialize dynamic signal.
 */
iocDynMBlkListItem *ioc_initialize_mblk_list_item(
    iocDynamicNetwork *dnetwork,
    iocHandle *mblk_handle);

/* Release dynamic signal.
 */
void ioc_release_mblk_list_item(
    iocDynamicNetwork *dnetwork,
    iocDynMBlkListItem *item);


#endif
#endif
