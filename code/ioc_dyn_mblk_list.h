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
typedef struct iocMblkShortcut
{
    iocHandle mblk_handle;

    struct iocMblkShortcut *next;
    struct iocMblkShortcut *prev;
}
iocMblkShortcut;


/* Allocate and initialize dynamic signal.
 */
iocMblkShortcut *ioc_add_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *mblk);

/* Release dynamic signal.
 */
void ioc_release_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    iocMblkShortcut *item);

/* Find a shortcut to memory block.
 */
iocHandle *ioc_find_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    const os_char *mblk_name,
    const os_char *device_name,
    os_short device_nr);

/* Remove memory block short cuts which are no longer needed.
 */
void ioc_clean_mblk_shortcuts(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *deleting_mblk);

#endif
#endif
