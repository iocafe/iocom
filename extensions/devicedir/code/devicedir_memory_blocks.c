/**

  @file    devicedir_memory_blocks.c
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


/**
****************************************************************************************************

  @brief List memory blocks this root node.

  The devicedir_memory_blocks() function lists memory blocks found under this root object.

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write the list as JSON
  @param   flags Information to display, bit fields: IOC_DEVDIR_DEFAULT, IOC_DEVDIR_DATA,
           IOC_DEVDIR_BUFFERS.

  @return  None.

****************************************************************************************************
*/
void devicedir_memory_blocks(
    iocRoot *root,
    osalStream list,
    const os_char *iopath,
    os_short flags)
{
    iocMemoryBlock *mblk;
    iocIdentifiers ids;
    os_char *sep;
    os_short mflags;
    os_boolean isfirst;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');
    osal_debug_assert(list != OS_NULL);

    /* Split IO path
     */
    ioc_iopath_to_identifiers(&ids, iopath, IOC_EXPECT_MEMORY_BLOCK);

    osal_stream_print_str(list, "{\"mblk\": [\n", 0);
    sep = "{";

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

        osal_stream_print_str(list, sep, 0);
        devicedir_append_str_param(list, "dev_name", mblk->device_name, OS_TRUE);
        devicedir_append_int_param(list, "dev_nr", mblk->device_nr, OS_FALSE);
        devicedir_append_str_param(list, "net_name", mblk->network_name, OS_FALSE);

        devicedir_append_str_param(list, "mblk_name", mblk->mblk_name, OS_FALSE);
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
        if (mflags & IOC_DYNAMIC_MBLK) devicedir_append_flag(list, "dynamic", &isfirst);
        osal_stream_print_str(list, "\"", 0);

        if (flags & IOC_DEVDIR_BUFFERS)
        {
            devicedir_list_mblks_source_buffers(mblk, list, flags);
            devicedir_list_mblks_target_buffers(mblk, list, flags);
        }

        if (flags & IOC_DEVDIR_DATA)
        {
            devicedir_append_mblk_binary(mblk, list, flags);
        }

        osal_stream_print_str(list, "}", 0);

        sep = ",\n{";
    }
    osal_stream_print_str(list, "\n", 0);

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
        if (tbuf)
        {
            osal_stream_print_str(list, ",", 0);
        }
        osal_stream_print_str(list, "\n", 0);
    }

    osal_stream_print_str(list, "  ]", 0);
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
        if (sbuf)
        {
            osal_stream_print_str(list, ",", 0);
        }
        osal_stream_print_str(list, "\n", 0);
    }

    osal_stream_print_str(list, "  ]", 0);
}


/**
****************************************************************************************************

  @brief Append memory block content as binary.

  The devicedir_append_mblk_binary() function...

  Sync lock must be on when calling this function.

  @param   mblk Pointer to the memory buffer buffer structure.
  @param   list Steam handle into which to write the JSON output.
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_append_mblk_binary(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags)
{
    os_uchar *buf;
    os_char nbuf[OSAL_NBUF_SZ];
    os_int nbytes, i;

    buf = (os_uchar*)mblk->buf;
    nbytes = mblk->nbytes;

    osal_stream_print_str(list, ",\n  \"data\": [\n    ", 0);

    for (i = 0; i<nbytes; i++)
    {
        osal_int_to_str(nbuf, sizeof(nbuf), (os_uchar)buf[i]);
        osal_stream_print_str(list, nbuf, 0);

        if (i + 1 < nbytes)
        {
            if (((i + 1) % 32) == 0)
                osal_stream_print_str(list, ",\n    ", 0);
            else
                osal_stream_print_str(list, ", ", 0);
        }
    }

    osal_stream_print_str(list, "\n  ]\n", 0);
}


/**
****************************************************************************************************

  @brief Convert info memory block to plain JSON text.

  The devicedir_static_mblk_to_json() function converts memory block to JSON. This is typically
  used to get content of "info" memory block un understandable format.

  @param   mblk Pointer to the memory buffer buffer structure.
  @param   list Steam handle into which to write the JSON output.

  @return  OSAL_SUCCESS (0) is all is fine. Other values indicate an error.

****************************************************************************************************
*/
osalStatus devicedir_static_mblk_to_json(
    iocMemoryBlock *mblk,
    osalStream list)
{
    osalStatus s;

    ioc_lock(mblk->link.root);
    s = osal_uncompress_json(list, mblk->buf, mblk->nbytes, 0);
    ioc_unlock(mblk->link.root);
    return s;
}
