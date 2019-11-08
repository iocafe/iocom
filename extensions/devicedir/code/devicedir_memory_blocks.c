/**

  @file    devicedir_memory_blocks.c
  @brief   List memory blocks.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.11.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/devicedir/devicedir.h"


/**
****************************************************************************************************

  @brief List memory blocks this root node.

  The devicedir_memory_blocks() function lists memory blocks found under this root object.

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write the list as JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_memory_blocks(
    iocRoot *root,
    osalStream list,
    os_short flags)
{
    iocMemoryBlock *mblk;
    os_short mflags;
    os_boolean isfirst;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');
    osal_debug_assert(list != OS_NULL);

    osal_stream_write_str(list, "{\"mblk\": [\n", 0);

    /* Synchronize.
     */
    ioc_lock(root);

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        osal_stream_write_str(list, "{", 0);
        devicedir_append_str_param(list, "dev_name", mblk->device_name, OS_TRUE);
        devicedir_append_int_param(list, "dev_nr", mblk->device_nr);
        devicedir_append_str_param(list, "net_name", mblk->network_name, OS_FALSE);

        devicedir_append_str_param(list, "mblk_name", mblk->mblk_name, OS_FALSE);
        devicedir_append_int_param(list, "mblk_nr", mblk->mblk_nr);
        devicedir_append_int_param(list, "mblk_id", mblk->mblk_id);
        devicedir_append_int_param(list, "size", mblk->nbytes);

        osal_stream_write_str(list, ", \"flags\":\"", 0);
        isfirst = OS_TRUE;
        mflags = mblk->flags;
        if (mflags & IOC_TARGET) devicedir_append_flag(list, "target", &isfirst);
        if (mflags & IOC_SOURCE) devicedir_append_flag(list, "source", &isfirst);
        if (mflags & IOC_AUTO_SYNC) devicedir_append_flag(list, "auto", &isfirst);
        if (mflags & IOC_ALLOW_RESIZE) devicedir_append_flag(list, "resize", &isfirst);
        if (mflags & IOC_STATIC) devicedir_append_flag(list, "static", &isfirst);
        osal_stream_write_str(list, "\"", 0);

        osal_stream_write_str(list, "}", 0);
        if (mblk->link.next)
        {
            osal_stream_write_str(list, ",", 0);
        }
        osal_stream_write_str(list, "\n", 0);
    }

    /* End synchronization.
     */
    ioc_unlock(root);

    osal_stream_write_str(list, "]}\n", 0);
}
