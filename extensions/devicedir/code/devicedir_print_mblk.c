/**

  @file    devicedir_print_blocks.c
  @brief   List memory blocks.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.11.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"


static void devicedir_append_memory_block(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags);


/**
****************************************************************************************************

  @brief Print content of a memory block.

  The devicedir_print_memory_blocks() function prints what a memory block or memory block contain.

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write the list as JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_print_memory_blocks(
    iocRoot *root,
    osalStream list,
    const os_char *iopath,
    os_short flags)
{
    iocMemoryBlock *mblk;
    iocIdentifiers ids;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');
    osal_debug_assert(list != OS_NULL);

    /* Split IO path
     */
    ioc_iopath_to_identifiers(&ids, iopath, IOC_EXPECT_MEMORY_BLOCK);

    osal_stream_print_str(list, "{\"mblk\": [\n", 0);

    /* Synchronize.
     */
    ioc_lock(root);

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        if (ids.network_name[0] != '\0')
        {
            if (os_strcmp(ids.network_name, mblk->network_name)) continue;
        }
        if (ids.device_name[0] != '\0')
        {
            if (os_strcmp(ids.device_name, mblk->device_name)) continue;
        }
        if (ids.device_nr)
        {
            if (ids.device_nr != mblk->device_nr) continue;
        }
        if (ids.mblk_name[0] != '\0')
        {
            if (os_strcmp(ids.mblk_name, mblk->mblk_name)) continue;
        }

        devicedir_append_memory_block(mblk, list, flags);
    }

    /* End synchronization.
     */
    ioc_unlock(root);

    osal_stream_print_str(list, "]}\n", 0);
}


/**
****************************************************************************************************

  @brief Show data of one target buffer.

  The devicedir_append_target_buffer() function appends information about target buffer to
  list (JSON).

  Sync lock must be on when calling this function.

  @param   tbuf Pointer to the target buffer structure.
  @param   list Steam handle into which to write the list as JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
static void devicedir_append_memory_block(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags)
{
    os_short mflags;
    os_boolean isfirst;

    osal_stream_print_str(list, "{", 0);
    devicedir_append_str_param(list, "dev_name", mblk->device_name, OS_TRUE);
    devicedir_append_int_param(list, "dev_nr", mblk->device_nr, OS_FALSE);
    devicedir_append_str_param(list, "net_name", mblk->network_name, OS_FALSE);

    devicedir_append_str_param(list, "mblk_name", mblk->mblk_name, OS_FALSE);
    devicedir_append_int_param(list, "mblk_nr", mblk->mblk_nr, OS_FALSE);
    devicedir_append_int_param(list, "mblk_id", mblk->mblk_id, OS_FALSE);
    devicedir_append_int_param(list, "size", mblk->nbytes, OS_FALSE);

    osal_stream_print_str(list, ", \"flags\":\"", 0);
    isfirst = OS_TRUE;
    mflags = mblk->flags;
    if (mflags & IOC_TARGET) devicedir_append_flag(list, "target", &isfirst);
    if (mflags & IOC_SOURCE) devicedir_append_flag(list, "source", &isfirst);
    if (mflags & IOC_AUTO_SYNC) devicedir_append_flag(list, "auto", &isfirst);
    if (mflags & IOC_ALLOW_RESIZE) devicedir_append_flag(list, "resize", &isfirst);
    if (mflags & IOC_STATIC) devicedir_append_flag(list, "static", &isfirst);
    osal_stream_print_str(list, "\"", 0);

    devicedir_list_mblks_source_buffers(mblk, list, flags);
    devicedir_list_mblks_target_buffers(mblk, list, flags);

    osal_stream_print_str(list, "}", 0);
    if (mblk->link.next)
    {
        osal_stream_print_str(list, ",", 0);
    }
    osal_stream_print_str(list, "\n", 0);
}
