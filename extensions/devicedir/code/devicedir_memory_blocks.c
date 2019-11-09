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
#include "devicedir.h"


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

    osal_stream_print_str(list, "{\"mblk\": [\n", 0);

    /* Synchronize.
     */
    ioc_lock(root);

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
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
void devicedir_append_target_buffer(
    iocTargetBuffer *tbuf,
    osalStream list,
    os_short flags)
{
    /* Check that root object is valid pointer.
     */
    osal_debug_assert(tbuf->debug_id == 'T');
    osal_debug_assert(list != OS_NULL);

    osal_stream_print_str(list, "    {", 0);
    devicedir_append_int_param(list, "remote_mblk_id", tbuf->remote_mblk_id, OS_TRUE);
    devicedir_append_int_param(list, "nbytes", tbuf->syncbuf.nbytes, OS_FALSE);
    devicedir_append_int_param(list, "buf_start_addr", tbuf->syncbuf.buf_start_addr, OS_FALSE);
    devicedir_append_int_param(list, "buf_end_addr", tbuf->syncbuf.buf_end_addr, OS_FALSE);
    devicedir_append_int_param(list, "buf_used", tbuf->syncbuf.buf_used, OS_FALSE);
    devicedir_append_int_param(list, "has_new_data", tbuf->syncbuf.has_new_data, OS_FALSE);
    devicedir_append_int_param(list, "newdata_start_addr", tbuf->syncbuf.newdata_start_addr, OS_FALSE);
    devicedir_append_int_param(list, "newdata_end_addr", tbuf->syncbuf.newdata_end_addr, OS_FALSE);

    osal_stream_print_str(list, "}", 0);
}


/**
****************************************************************************************************

  @brief List target buffers connected to a memory block.

  The devicedir_list_mblks_target_buffers() function appends list of target buffers attached to
  a memory block to lists in JSON format.

  Sync lock must be on when calling this function.

  @param   mblk Pointer to the memory buffer buffer structure.
  @param   list Steam handle into which to write the list as JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_list_mblks_target_buffers(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags)
{
    iocTargetBuffer *tbuf;

    tbuf = mblk->tbuf.first;
    if (tbuf == OS_NULL) return;

    osal_stream_print_str(list, ",\n  \"tbuf\": [\n", 0);

    while (tbuf)
    {
        devicedir_append_target_buffer(tbuf, list, flags);

        tbuf = tbuf->mlink.next;
        osal_stream_print_str(list, "}", 0);
        if (tbuf)
        {
            osal_stream_print_str(list, ",", 0);
        }
        osal_stream_print_str(list, "\n", 0);
    }

    osal_stream_print_str(list, "  }\n", 0);
}


/**
****************************************************************************************************

  @brief Append information about one source buffer.

  The devicedir_append_source_buffer() function appends information about a source buffer to
  list (JSON).

  Sync lock must be on when calling this function.

  @param   sbuf Pointer to the source buffer structure.
  @param   list Steam handle into which to write the list as JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_append_source_buffer(
    iocSourceBuffer *sbuf,
    osalStream list,
    os_short flags)
{
    /* Check that root object is valid pointer.
     */
    osal_debug_assert(sbuf->debug_id == 'S');
    osal_debug_assert(list != OS_NULL);

    osal_stream_print_str(list, "    {", 0);
    devicedir_append_int_param(list, "remote_mblk_id", sbuf->remote_mblk_id, OS_TRUE);
    devicedir_append_int_param(list, "range_set", sbuf->changed.range_set, OS_FALSE);
    devicedir_append_int_param(list, "changed.start_addr", sbuf->changed.start_addr, OS_FALSE);
    devicedir_append_int_param(list, "changed.end_addr", sbuf->changed.end_addr, OS_FALSE);

    devicedir_append_int_param(list, "nbytes", sbuf->syncbuf.nbytes, OS_FALSE);
    devicedir_append_int_param(list, "buf_used", sbuf->syncbuf.used, OS_FALSE);

    devicedir_append_int_param(list, "make_keyframe", sbuf->syncbuf.make_keyframe, OS_FALSE);
    devicedir_append_int_param(list, "is_keyframe", sbuf->syncbuf.is_keyframe, OS_FALSE);
    devicedir_append_int_param(list, "start_addr", sbuf->syncbuf.start_addr, OS_FALSE);
    devicedir_append_int_param(list, "end_addr", sbuf->syncbuf.end_addr, OS_FALSE);

    osal_stream_print_str(list, "}", 0);
}


/**
****************************************************************************************************

  @brief List source buffers connected to a memory block.

  The devicedir_list_mblks_source_buffers() function appends list of source buffers attached to
  a memory block to lists in JSON format.

  Sync lock must be on when calling this function.

  @param   mblk Pointer to the memory buffer buffer structure.
  @param   list Steam handle into which to write the list as JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_list_mblks_source_buffers(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags)
{
    iocSourceBuffer *sbuf;

    sbuf = mblk->sbuf.first;
    if (sbuf == OS_NULL) return;

    osal_stream_print_str(list, ",\n  \"sbuf\": [\n", 0);

    while (sbuf)
    {
        devicedir_append_source_buffer(sbuf, list, flags);

        sbuf = sbuf->mlink.next;
        osal_stream_print_str(list, "}", 0);
        if (sbuf)
        {
            osal_stream_print_str(list, ",", 0);
        }
        osal_stream_print_str(list, "\n", 0);
    }

    osal_stream_print_str(list, "  }\n", 0);
}

