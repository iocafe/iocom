/**

  @file    ioc_dyn_mblk_list.h
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
#if IOC_DYNAMIC_MBLK_CODE

/**
****************************************************************************************************
  Item in memory block shortcut list
****************************************************************************************************
*/
typedef struct iocMblkShortcut
{
    iocHandle mblk_handle;

    struct iocMblkShortcut *next;
    struct iocMblkShortcut *prev;
}
iocMblkShortcut;


/**
****************************************************************************************************
  Memory block shortcut functions
****************************************************************************************************
*/
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

/* Find memory block.
 */
iocHandle *ioc_find_mblk_shortcut(
    iocDynamicNetwork *dnetwork,
    const os_char *mblk_name,
    const os_char *device_name,
    os_uint device_nr);

/* Find memory block using shortcut list, start from IOCOM root.
 */
osalStatus ioc_find_mblk(
    iocRoot *root,
    iocHandle *handle,
    const os_char *mblk_name,
    const os_char *device_name,
    os_uint device_nr,
    const os_char *network_name);

/* Remove memory block short cuts which are no longer needed.
 */
void ioc_clean_mblk_shortcuts(
    iocDynamicNetwork *dnetwork,
    iocMemoryBlock *deleting_mblk);

#endif
