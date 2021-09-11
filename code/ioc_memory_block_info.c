/**

  @file    ioc_memory_block_info.h
  @brief   Memory block information.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Functionality related to memory block information. Memory block information is sent trough
  connections to inform connected devices what data they can access, connect to, etc.

  Memory block information is sent through connection when a connection is established, or
  when new memory block is created while there are existing connections.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

/* Forward referred static functions.
 */
static iocMemoryBlock *ioc_get_mbinfo_to_send2(
    struct iocConnection *con);

static void ioc_mbinfo_new_sbuf(
    iocConnection *con,
    iocMemoryBlock *mblk,
    iocMemoryBlockInfo *info,
    os_short bdflags);

static void ioc_mbinfo_new_tbuf(
    iocConnection *con,
    iocMemoryBlock *mblk,
    iocMemoryBlockInfo *info,
    os_short bdflags);

#if IOC_DYNAMIC_MBLK_CODE
static void ioc_mbinfo_info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);
#endif

/**
****************************************************************************************************

  @brief Make sure that information about new memory block gets sent
  @anchor ioc_add_mblk_to_global_mbinfo

  The ioc_add_mblk_to_global_mbinfo() function sets the new memory block as current memory block
   whose info to send, to all upwards connections. If connection is sending other memory block
  information, nothing is done since new memory block is added as last memory block and thus it's
  info will get sent.

  ioc_lock() must be on when this function is called.

  @param   mblk Pointer to the memory block object.
  @return  None.

****************************************************************************************************
*/
void ioc_add_mblk_to_global_mbinfo(
    struct iocMemoryBlock *mblk)
{
    iocRoot *root;
    iocConnection *con;

    if (mblk == OS_NULL) return;
    root = mblk->link.root;

    for (con = root->con.first; con; con = con->link.next)
    {
#if IOC_SERVER2CLOUD_CODE
        if (con->flags & IOC_CLOUD_CONNECTION)
        {
            if (mblk->flags & IOC_NO_CLOUD) continue;
        }
        else
        {
            if (mblk->flags & IOC_CLOUD_ONLY) continue;
        }
#endif
        if (con->flags & IOC_CONNECT_UP)
        {
            if (con->sinfo.current_mblk == OS_NULL)
            {
                con->sinfo.current_mblk = mblk;
            }
        }

#if IOC_SERVER2CLOUD_CODE
        else if ((con->flags & IOC_CLOUD_CONNECTION) &&
            (mblk->flags & (IOC_CLOUD_ONLY|IOC_NO_CLOUD)) == IOC_CLOUD_ONLY)
        {
             con->sinfo.current_cloud_mblk = mblk;
        }
#endif
    }
}


/**
****************************************************************************************************

  @brief Make sure that information about all memory blocks is sent trough a specific connection.
  @anchor ioc_add_con_to_global_mbinfo

  The ioc_add_con_to_global_mbinfo() function is called when a connection to another device is
  established. If connection is upwaeds, it sets that information about all memory blocks needs
  to be sent through the new connection.

  ioc_lock() must be on when this function is called.

  @param   con Pointer to the connection object (established connection).
  @return  None.

****************************************************************************************************
*/
void ioc_add_con_to_global_mbinfo(
    struct iocConnection *con)
{
    iocRoot *root;
    root = con->link.root;

    /* Be sure to ignore previous value of current_mblk.
     */
    if (con->flags & IOC_CONNECT_UP)
    {
        con->sinfo.current_mblk = root->mblk.first;
    }
#if IOC_SERVER2CLOUD_CODE
    else if (con->flags & IOC_CLOUD_CONNECTION)
    {
        con->sinfo.current_cloud_mblk = root->mblk.first;
    }
#endif
}


/**
****************************************************************************************************

  @brief Mark that memory block information is not needed for now.
  @anchor ioc_add_con_to_global_mbinfo

  The ioc_mbinfo_con_is_closed() function is called when connection is dropped. It marks that
  memory block information for the connection is not needed for now.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_mbinfo_con_is_closed(
    struct iocConnection *con)
{
    con->sinfo.current_mblk = OS_NULL;
#if IOC_SERVER2CLOUD_CODE
    con->sinfo.current_cloud_mblk = OS_NULL;
#endif
}


/**
****************************************************************************************************

  @brief Check if we have information to send trough the connection.
  @anchor ioc_get_mbinfo_to_send

  The ioc_get_mbinfo_to_send() function checks if there is memory block information which
  needs to be sent through the connection before any data can be sent.

  Moves on

  ioc_lock() must be on when this function is called.

  @param   con Pointer to the connection object.
  @return  Pointer to memory block whose info to send to this connection, or OS NULL if nont.

****************************************************************************************************
*/
struct iocMemoryBlock *ioc_get_mbinfo_to_send(
    struct iocConnection *con)
{
    iocMemoryBlock *mblk;

    while ((mblk = ioc_get_mbinfo_to_send2(con)))
    {
#if IOC_SERVER2CLOUD_CODE
        if (con->flags & IOC_CLOUD_CONNECTION)
        {
            if (mblk->flags & IOC_NO_CLOUD) goto skipit;
        }
        else
        {
            if (mblk->flags & IOC_CLOUD_ONLY) goto skipit;
        }
#endif

#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
        /* If network is not authorized, just drop skip the memory block.
         */
        if (!ioc_is_network_authorized(con, mblk->network_name, 0))
        {
            goto skipit;
        }
#endif
#endif

        if (con->flags & IOC_CONNECT_UP) break;
        if ((mblk->flags & IOC_FLOOR) == 0) break;
#if IOC_SERVER2CLOUD_CODE
skipit:
#endif
        ioc_mbinfo_sent(con, mblk);
    }

    return mblk;
}



/**
****************************************************************************************************

  @brief Check if we have something which might be sent trough the connection.
  @anchor ioc_get_mbinfo_to_send2

  The ioc_get_mbinfo_to_send2() ...

  ioc_lock() must be on when this function is called.

  @param   con Pointer to the connection object.
  @return  Pointer to memory block whose info to send to this connection, or OS NULL if nont.

****************************************************************************************************
*/
static iocMemoryBlock *ioc_get_mbinfo_to_send2(
    struct iocConnection *con)
{
    if (con->flags & IOC_CONNECT_UP)
    {
        return con->sinfo.current_mblk;
    }
    else
    {
#if IOC_SERVER2CLOUD_CODE
        if (con->sinfo.current_cloud_mblk)
        {
            return con->sinfo.current_cloud_mblk;
        }
#endif
        if (con->sbuf.mbinfo_down)
        {
            return con->sbuf.mbinfo_down->mlink.mblk;
        }
        if (con->tbuf.mbinfo_down)
        {
            return con->tbuf.mbinfo_down->mlink.mblk;
        }
    }
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Memory block information was successfully sent, move on to the next one.
  @anchor ioc_mbinfo_sent

  The ioc_mbinfo_sent() function is called once memory block information has been sent trough
  a connection. It moves current memory block whose information to send to next one. If no
  more memory block, the current block is set to NULL.

  ioc_lock() must be on when this function is called.

  @param   con Pointer to the connection object.
  @param   mblk Pointer to the memory block object.
  @return  None.

****************************************************************************************************
*/
void ioc_mbinfo_sent(
    struct iocConnection *con,
    struct iocMemoryBlock *mblk)
{
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf;

    if (con->flags & IOC_CONNECT_UP)
    {
        con->sinfo.current_mblk = OS_NULL;
        if (mblk == OS_NULL) return;

        con->sinfo.current_mblk = mblk->link.next;
    }
    else
    {
#if IOC_SERVER2CLOUD_CODE
        if (mblk == con->sinfo.current_cloud_mblk)
        {
            con->sinfo.current_cloud_mblk = mblk->link.next;
        }
#endif

        tbuf = con->tbuf.mbinfo_down;
        if (tbuf)
        {
            if (mblk == tbuf->mlink.mblk)
            {
                con->tbuf.mbinfo_down = tbuf->clink.next;
            }
        }

        sbuf = con->sbuf.mbinfo_down;
        if (sbuf)
        {
            if (mblk == sbuf->mlink.mblk)
            {
                con->sbuf.mbinfo_down = sbuf->clink.next;
            }
        }
    }
}


/**
****************************************************************************************************

  @brief Memory block is being deleted, make sure that it is not referred by any connection
  @anchor ioc_mbinfo_mblk_is_deleted

  The ioc_mbinfo_mblk_is_deleted() function is called when memory block is deleted to ensure
  that there is no pointers left around to point it.

  ioc_lock() must be on when this function is called.

  @param   mblk Pointer to the memory block object.
  @return  None.

****************************************************************************************************
*/
void ioc_mbinfo_mblk_is_deleted(
    struct iocMemoryBlock *mblk)
{
    iocRoot *root;
    iocConnection *con;

    if (mblk == OS_NULL) return;
    root = mblk->link.root;

    for (con = root->con.first; con; con = con->link.next)
    {
        if (con->sinfo.current_mblk == mblk)
        {
            con->sinfo.current_mblk = mblk->link.next;
        }

#if IOC_SERVER2CLOUD_CODE
        if (con->sinfo.current_cloud_mblk == mblk)
        {
            con->sinfo.current_cloud_mblk = mblk->link.next;
        }
#endif
    }
}


/**
****************************************************************************************************

  @brief Process complete memory block information frame received from socket or serial port.
  @anchor ioc_process_received_system_frame

  The ioc_process_received_mbinfo_frame() function is called once a complete system frame
  containing memory block information is received.

  ioc_lock() must be on before calling this function.

  @param   con Pointer to the connection object.
  @param   mblk_id Memory block identifier in this end.
  @param   data Received data, can be compressed and delta encoded, check flags.

  @return  OSAL_SUCCESS if successfull. Other values indicate a corrupted frame.

****************************************************************************************************
*/
osalStatus ioc_process_received_mbinfo_frame(
    struct iocConnection *con,
    os_uint mblk_id,
    os_char *data)
{
    iocRoot *root;
    iocMemoryBlockInfo mbinfo;
    os_uchar
        iflags,
        *p; /* keep as unsigned */

    p = (os_uchar*)data + 1; /* Skip system frame IOC_SYSFRAME_MBLK_INFO byte. */
    iflags = (os_uchar)*(p++);
    os_memclear(&mbinfo, sizeof(mbinfo));
    mbinfo.device_nr = ioc_msg_get_uint(&p,
        iflags & IOC_INFO_D_2BYTES,
        iflags & IOC_INFO_D_4BYTES);

    mbinfo.mblk_id = mblk_id;
    mbinfo.nbytes = ioc_msg_get_uint(&p,
        iflags & IOC_INFO_N_2BYTES,
        iflags & IOC_INFO_N_4BYTES);
    mbinfo.flags = ioc_msg_get_ushort(&p, iflags & IOC_INFO_F_2BYTES);
    if (iflags & IOC_INFO_HAS_DEVICE_NAME)
    {
        if (ioc_msg_getstr(mbinfo.device_name, IOC_NAME_SZ, &p))
            return OSAL_STATUS_FAILED;
        if (ioc_msg_getstr(mbinfo.network_name, IOC_NETWORK_NAME_SZ, &p))
            return OSAL_STATUS_FAILED;
    }
    if (iflags & IOC_INFO_HAS_MBLK_NAME)
    {
        if (ioc_msg_getstr(mbinfo.mblk_name, IOC_NAME_SZ, &p))
            return OSAL_STATUS_FAILED;
    }

    /* If we received message from device which requires automatically given
       device number in controller end (not auto eumerated device) device,
       give the number now.
     */
    root = con->link.root;
    if (mbinfo.device_nr == IOC_AUTO_DEVICE_NR)
    {
        /* If we do not have automatic device number, reserve one now
         */
        if (!con->auto_device_nr)
        {
#if IOC_AUTO_DEVICE_NR_SUPPORT
            con->auto_device_nr = ioc_get_automatic_device_nr(root, con->unique_id_bin);
#else
            con->auto_device_nr = ioc_get_automatic_device_nr(root, OS_NULL);
#endif
        }
        mbinfo.device_nr = con->auto_device_nr;
        mbinfo.local_flags = IOC_MBLK_LOCAL_AUTO_ID;
    }

    /* If this is message to device with automatic device number and this device has
       automatic device number
     */
    else if (mbinfo.device_nr == IOC_TO_AUTO_DEVICE_NR &&
        root->device_nr == IOC_AUTO_DEVICE_NR)
    {
        mbinfo.device_nr = IOC_AUTO_DEVICE_NR;
    }

    ioc_mbinfo_received(con, &mbinfo);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Memory block information received, create source and target buffers.
  @anchor ioc_mbinfo_received

  The ioc_mbinfo_received() function is called when memory block information is received.
  The function created source and target buffers according to received information and thus
  binds memory blocks with same device name, number and memory block number together.

  Function can create memory blocks dynamically. Define IOC_DYNAMIC_MBLK_CODE controls if code
  for dynamic memory blocks is supported for platform. In addition flag IOC_DYNAMIC_MBLKS
  needs to be given to connection or end point to enable this feature.

  Function can resize memory blocks if memory block in other end is large than in this end.
  Define IOC_RESIZE_MBLK_CODE controls if code support for this included for the platform.
  Flag IOC_ALLOW_RESIZE controls weather resizing is allowed for a specific memory block.

  Code support for both dynamic memory blocks and resizing memory blocks is normally
  enabled if underlyin platform supports dynamic memory allocation through eosal. This
  can be overridden by compiler define.

  ioc_lock() must be on when this function is called.

  @param   mblk Pointer to the memory block object.
  @return  None.

****************************************************************************************************
*/
void ioc_mbinfo_received(
    struct iocConnection *con,
    iocMemoryBlockInfo *info)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_short source_flag, target_flag;
    os_short bdflag = 0;

#if IOC_DYNAMIC_MBLK_CODE
    iocHandle handle;
    iocMemoryBlockParams mbprm;
    iocDynamicNetwork *dnetwork;
#endif

    root = con->link.root;

#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
    /* If network is not authorized, just drop the received information.
     */
    if (!ioc_is_network_authorized(con, info->network_name, 0))
    {
        return;
    }
#endif

    /* Find if we have memory block with device name, number and memory block
       number. If not, do nothing.
     */
    mblk = root->mblk.first;
    while (OS_TRUE)
    {
        if (mblk == OS_NULL)
        {
            osal_trace_str("~MBINFO received, dev name=", info->device_name);
            osal_trace_int("~, dev nr=", info->device_nr);
            osal_trace_str("~, net name=", info->network_name);
            osal_trace_str(", mblk name=", info->mblk_name);

#if IOC_DYNAMIC_MBLK_CODE
            /* If we can allocate memory blocks dynamically.
             */
            if (con->flags & IOC_DYNAMIC_MBLKS)
            {
                os_memclear(&mbprm, sizeof(mbprm));
                mbprm.network_name = info->network_name;
                mbprm.device_name = info->device_name;
                mbprm.device_nr = info->device_nr;
                mbprm.flags = (info->flags & (IOC_MBLK_DOWN|IOC_MBLK_UP))
                    | (IOC_ALLOW_RESIZE|IOC_DYNAMIC);
                mbprm.local_flags = info->local_flags;
                mbprm.mblk_name = info->mblk_name;
                mbprm.nbytes = info->nbytes;

                if (ioc_initialize_memory_block(&handle, OS_NULL, root, &mbprm)) return;
                mblk = handle.mblk;

                /* If we are maintaining dynamic information, create dynamic information
                   structures for network and memory block already now.
                 */
                if (root->droot)
                {
                    dnetwork = ioc_add_dynamic_network(root->droot, mbprm.network_name);

                    if (ioc_find_mblk_shortcut(dnetwork, mbprm.mblk_name,
                        mbprm.device_name, mbprm.device_nr) == OS_NULL)
                    {
                        ioc_add_mblk_shortcut(dnetwork, mblk);
                    }

                    /* If this is info memory block, add callback to receive dynamic info.
                     */
                    if (!os_strcmp(mblk->mblk_name, "info"))
                    {
                        ioc_add_callback(&mblk->handle, ioc_mbinfo_info_callback, OS_NULL);
                    }
                }

                ioc_new_root_event(root, IOC_NEW_MEMORY_BLOCK, OS_NULL, mblk,
                    root->callback_context);

                ioc_release_handle(&handle);
                break;
            }
#endif
            osal_trace2("No matching memory block");
            return;
        }

        /* Compare memory block, device and network names. If same the memory blocks
           do match. If one end has an empty name, accept it (execpt memory block name).
           Notice that accepting empty names needs to be treated the same way for
           both ends.
         */
        if (os_strcmp(info->mblk_name, mblk->mblk_name)) goto goon;
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
        if (info->device_nr != mblk->device_nr) goto goon;
        if (os_strcmp(info->device_name, mblk->device_name)) goto goon;
        if (os_strcmp(info->network_name, mblk->network_name)) goto goon;
#else
        if (info->device_nr != root->device_nr) goto goon;
        if (os_strcmp(info->device_name, root->device_name)) goto goon;
        if (os_strcmp(info->network_name, root->network_name)) goto goon;
#endif

        osal_trace_str("~MBinfo matched, dev name=", info->device_name);
        osal_trace_int("~, dev nr=", info->device_nr);
        osal_trace_str(", mblk name=", info->mblk_name);
        break;
goon:
        mblk = mblk->link.next;
    }

#if IOC_RESIZE_MBLK_CODE
    /* If block in other end is larger and we can resize the block at this end?
     */
    ioc_resize_mblk(mblk, info->nbytes, 0);
#endif

#if IOC_BIDIRECTIONAL_MBLK_CODE
    if (con->flags & IOC_BIDIRECTIONAL_MBLKS)
    {
        bdflag = (mblk->flags & info->flags & IOC_BIDIRECTIONAL);
    }
#endif

    if (con->flags & IOC_CONNECT_UP)
    {
        source_flag = IOC_MBLK_UP;
        target_flag = IOC_MBLK_DOWN;
    }
    else
    {
        target_flag = IOC_MBLK_UP;
        source_flag = IOC_MBLK_DOWN;
    }

    /* If we have a memory block which can be a source and the other end matches?
     */
    if ((mblk->flags & source_flag) && (info->flags & source_flag))
    {
#if IOC_BIDIRECTIONAL_MBLK_CODE
        if (con->flags & IOC_CONNECT_UP) bdflag = 0;
        ioc_mbinfo_new_sbuf(con, mblk, info, bdflag);
        if (bdflag)
        {
            ioc_mbinfo_new_tbuf(con, mblk, info, bdflag);
        }
#else
        ioc_mbinfo_new_sbuf(con, mblk, info, 0);
#endif
    }

    /* If we have memory block which can be a target and the other end matches?
     */
    if ((mblk->flags & target_flag) && (info->flags & target_flag))
    {
#if IOC_BIDIRECTIONAL_MBLK_CODE
        if ((con->flags & IOC_CONNECT_UP) == 0) bdflag = 0;
        if (bdflag)
        {
            ioc_mbinfo_new_sbuf(con, mblk, info, bdflag);
        }
#endif
        ioc_mbinfo_new_tbuf(con, mblk, info, bdflag);
    }
}


/**
****************************************************************************************************

  @brief Setup new source buffer for the memory block.
  @anchor ioc_mbinfo_new_sbuf

  The ioc_mbinfo_new_sbuf() function adds new source buffer according to received memory block
  information.

  ioc_lock() must be on when this function is called.

  @param   con Connection from which memory block info was received.
  @param   mblk Pointer to the memory block object.
  @param   info Memory block information.
  @param   bdflags Nonzero (IOC_BIDIRECTIONAL) to set up for bidirectional transfer.
  @return  None.

****************************************************************************************************
*/static void ioc_mbinfo_new_sbuf(
    iocConnection *con,
    iocMemoryBlock *mblk,
    iocMemoryBlockInfo *info,
    os_short bdflags)
{
    iocSourceBuffer *sbuf;

#if IOC_ROOT_CALLBACK_SUPPORT
    iocRoot *root;
    root = con->link.root;
#endif
    /* Check if we already have subscription for this connection.
     */
    for (sbuf = con->sbuf.first;
         sbuf;
         sbuf = sbuf->clink.next)
    {
        if (mblk == sbuf->mlink.mblk)
        {
            osal_trace2("Memory block already has source buffer for the connection");
            return;
        }
    }

    /* Create source buffer to link the connection and memory block together.
     */
    sbuf = ioc_initialize_source_buffer(con, mblk, info->mblk_id, bdflags & IOC_BIDIRECTIONAL);

    /* Do initial synchronization for all memory blocks.
     */
    ioc_sbuf_synchronize(sbuf);

    /* Application may want to know that the memory block has been connected.
     */
#if IOC_ROOT_CALLBACK_SUPPORT
    ioc_new_root_event(root, IOC_MBLK_CONNECTED_AS_SOURCE, OS_NULL, mblk,
        root->callback_context);
#endif

    /* Mark that we need to send memory block info back. If pointer is
       set, do nothing because the source buffer was added to last in list.
     */
    if ((con->flags & IOC_CONNECT_UP) == 0)
    {
        if (con->sbuf.mbinfo_down == OS_NULL)
        {
            con->sbuf.mbinfo_down = sbuf;
        }
    }
#if IOC_SERVER2CLOUD_CODE
    else if ((con->flags & IOC_CLOUD_CONNECTION) &&
        (mblk->flags & (IOC_CLOUD_ONLY|IOC_NO_CLOUD)) == IOC_CLOUD_ONLY)
    {
        if (con->sinfo.current_cloud_mblk == OS_NULL)
        {
            con->sinfo.current_cloud_mblk = mblk;
        }
    }
#endif
}


/**
****************************************************************************************************

  @brief Setup new target buffer for the memory block.
  @anchor ioc_mbinfo_new_tbuf

  The ioc_mbinfo_new_tbuf() function adds new target buffer according to received block
  information.

  ioc_lock() must be on when this function is called.

  @param   con Connection from which memory block info was received.
  @param   mblk Pointer to the memory block object.
  @param   info Memory block information.
  @param   bdflags Nonzero (IOC_BIDIRECTIONAL) to set up for bidirectional transfer.
  @return  None.

****************************************************************************************************
*/
static void ioc_mbinfo_new_tbuf(
    iocConnection *con,
    iocMemoryBlock *mblk,
    iocMemoryBlockInfo *info,
    os_short bdflags)
{
    iocTargetBuffer *tbuf, *next_tbuf;

#if IOC_ROOT_CALLBACK_SUPPORT
    iocRoot *root;
    root = con->link.root;
#endif

    /* Check if we already have target buffer for this connection.
     */
    for (tbuf = con->tbuf.first;
         tbuf;
         tbuf = tbuf->clink.next)
    {
        if (mblk == tbuf->mlink.mblk)
        {
            osal_trace2("Memory block already target buffer for this connection");
            return;
        }
    }

    /* If there is memory block is already one directional data transfer, delete it.
     */
    if (bdflags == 0)
    {
        for (tbuf = mblk->tbuf.first;
             tbuf;
             tbuf = next_tbuf)
        {
            next_tbuf = tbuf->mlink.next;
#if IOC_BIDIRECTIONAL_MBLK_CODE
            if ((tbuf->syncbuf.flags & IOC_BIDIRECTIONAL) == 0)
            {
                osal_trace("Existing target buffer for memory block dropped");
                ioc_release_target_buffer(tbuf);
            }
#else
            osal_trace("Existing target buffer for memory block dropped");
            ioc_release_target_buffer(tbuf);
#endif
        }
    }

    /* Create source buffer to link the connection and memory block together.
     */
    tbuf = ioc_initialize_target_buffer(con, mblk, info->mblk_id,
        bdflags & IOC_BIDIRECTIONAL);

    /* Application may want to know that the memory block has been connected.
     */
#if IOC_ROOT_CALLBACK_SUPPORT
    ioc_new_root_event(root, IOC_MBLK_CONNECTED_AS_TARGET, OS_NULL, mblk,
        root->callback_context);
#endif

    /* Mark that we need to send memory block info back. If pointer is
       set, do nothing because the source buffer was added to last in list.
     */
    if ((con->flags & IOC_CONNECT_UP) == 0)
    {
        if (con->tbuf.mbinfo_down == OS_NULL)
        {
            con->tbuf.mbinfo_down = tbuf;
        }
    }
#if IOC_SERVER2CLOUD_CODE
    else if ((con->flags & IOC_CLOUD_CONNECTION) &&
        (mblk->flags & (IOC_CLOUD_ONLY|IOC_NO_CLOUD)) == IOC_CLOUD_ONLY)
    {
        if (con->sinfo.current_cloud_mblk == OS_NULL)
        {
            con->sinfo.current_cloud_mblk = mblk;
        }
    }
#endif
}

#if IOC_DYNAMIC_MBLK_CODE
/**
****************************************************************************************************

  @brief Callback function to add dynamic device information.

  The ioc_mbinfo_info_callback() function is called when device information data is received from
  connection or when connection status changes.

  @param   handle Memory block handle.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void ioc_mbinfo_info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    OSAL_UNUSED(start_addr);
    OSAL_UNUSED(flags);
    OSAL_UNUSED(context);

    /* If actual data received (not connection status change).
     */
    if (end_addr >= 0)
    {
        ioc_add_dynamic_info(handle, OS_FALSE);
    }
}
#endif
