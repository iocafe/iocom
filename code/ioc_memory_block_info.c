/**

  @file    ioc_memory_block_info.h
  @brief   Memory block information.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    24.6.2019

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
        if (con->flags & IOC_CONNECT_UP)
        {
            if (con->sinfo.current_mblk == OS_NULL)
            {
                con->sinfo.current_mblk = mblk;
            }
        }
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

    /* Be sure to ignore previous value of current_mblk.
     */
    if (con->flags & IOC_CONNECT_UP)
    {
        root = con->link.root;
        con->sinfo.current_mblk = root->mblk.first;
    }
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
}


/**
****************************************************************************************************

  @brief Check if we have information to send trough the connection.
  @anchor ioc_get_mbinfo_to_send

  The ioc_get_mbinfo_to_send() function checks if there is memory block information which
  needs to be sent through the connection before any data can be sent.

  ioc_lock() must be on when this function is called.

  @param   con Pointer to the connection object.
  @return  Pointer to memory block whose info to send to this connection, or OS NULL if nont.

****************************************************************************************************
*/
struct iocMemoryBlock *ioc_get_mbinfo_to_send(
    struct iocConnection *con)
{
    if (con->flags & IOC_CONNECT_UP)
    {
        return con->sinfo.current_mblk;
    }
    else
    {
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

  @return  OSAL_SUCCESS if succesfull. Other values indicate a corrupted frame.

****************************************************************************************************
*/
osalStatus ioc_process_received_mbinfo_frame(
    struct iocConnection *con,
    os_uint mblk_id,
    os_char *data)
{
    iocMemoryBlockInfo
        mbinfo;

    os_uchar
        iflags,
        *p; /* keep as unsigned */

    p = (os_uchar*)data + 1; /* Skip system frame IOC_SYSRAME_MBLK_INFO byte. */
    iflags = (os_uchar)*(p++);
    os_memclear(&mbinfo, sizeof(mbinfo));
    mbinfo.device_nr = ioc_msg_get_uint(&p,
        iflags & IOC_INFO_D_2BYTES,
        iflags & IOC_INFO_D_4BYTES);

    /* If we received message from device which requires automatically given
       device number in controller end (not auto eumerated device) device,
       give the number now.
     */
    if (mbinfo.device_nr == IOC_AUTO_DEVICE_NR)
    {
        if (con->link.root->device_nr != IOC_AUTO_DEVICE_NR)
        {
            /* If we do not have automatic device number, reserve one now
             */
            if (!con->auto_device_nr)
            {
                con->auto_device_nr = ioc_get_unique_device_id(con->link.root);
            }
            mbinfo.device_nr = con->auto_device_nr;
        }
    }

    mbinfo.mblk_id = mblk_id;
    mbinfo.nbytes = ioc_msg_get_ushort(&p, iflags & IOC_INFO_N_2BYTES);
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
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf, *next_tbuf;
    os_short source_flag, target_flag;

#if IOC_DYNAMIC_MBLK_CODE
    iocHandle handle;
    iocMemoryBlockParams mbprm;
    iocDynamicNetwork *dnetwork;
#endif

#if IOC_RESIZE_MBLK_CODE
    os_char *newbuf;
#endif

    /* Find if we have memory block with device name, number and memory block 
       number. If not, do nothing.
     */
    root = con->link.root;
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
                    | (IOC_ALLOW_RESIZE|IOC_AUTO_SYNC|IOC_DYNAMIC_MBLK);
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
        if (info->device_nr != mblk->device_nr) goto goon;
        if (os_strcmp(info->mblk_name, mblk->mblk_name)) goto goon;
        if (os_strcmp(info->device_name, mblk->device_name)) goto goon;
        if (os_strcmp(info->network_name, mblk->network_name)) goto goon;

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
    if (info->nbytes > mblk->nbytes &&
        mblk->flags & IOC_ALLOW_RESIZE)
    {
        if (mblk->buf_allocated)
        {
            newbuf = ioc_malloc(root, info->nbytes, OS_NULL);
            os_memcpy(newbuf, mblk->buf, mblk->nbytes);
            ioc_free(root, mblk->buf, mblk->nbytes);
            mblk->buf = newbuf;
            mblk->nbytes = info->nbytes;
        }
#if OSAL_DEBUG
        else
        {
            osal_debug_error("Attempt to resize static memory block");
        }
#endif
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

    /* If we have a memory block which can be a source?
     */
    if (mblk->flags & source_flag)
    {
        /* If the other memory block cannot be a target, do nothing more
         */
        if ((info->flags & source_flag) == 0)
        {
            osal_trace3("source - source skipped");
            goto skip1;
        }

        /* Check if we already have subscription for this connection.
         */
        for (sbuf = con->sbuf.first;
             sbuf;
             sbuf = sbuf->clink.next)
        {
            if (mblk == sbuf->mlink.mblk)
            {
                osal_trace2("Memory block already subscribed for the connection");
                goto skip1;
            }
        }

        /* Create source buffer to link the connection and memory block together.
         */
        sbuf = ioc_initialize_source_buffer(con, mblk, info->mblk_id, info->flags & IOC_BIDIRECTIONAL);

        /* Do initial synchronization for all memory blocks.
         */
        ioc_sbuf_synchronize(sbuf);

        /* Application may want to know that the memory block has been connected.
         */
        ioc_new_root_event(root, IOC_MBLK_CONNECTED_AS_SOURCE, OS_NULL, mblk,
            root->callback_context);

        /* Mark that we need to send memory block info back. If pointer is
           set, do nothing because the source buffer was added to last in list.
         */
        if ((con->flags & IOC_CONNECT_UP) == 0 &&
            con->sbuf.mbinfo_down == OS_NULL && sbuf)
        {
            con->sbuf.mbinfo_down = sbuf;
        }
    }
skip1:

    /* If we have memory block which can be a target
     */
    if (mblk->flags & target_flag)
    {
        /* If the other memory block cannot be a source
         */
        if ((info->flags & target_flag) == 0)
        {
            osal_trace3("target - target skippes");
            goto skip2;
        }

        /* Check if we already have target buffer for this connection.
         */
        for (tbuf = con->tbuf.first;
             tbuf;
             tbuf = tbuf->clink.next)
        {
            if (mblk == tbuf->mlink.mblk)
            {
                osal_trace2("Memory block already targeted for the connection");
                goto skip2;
            }
        }

        /* If we have target buffer for other connection, delete it.
         */
        for (tbuf = mblk->tbuf.first;
             tbuf;
             tbuf = next_tbuf)
        {
            next_tbuf = tbuf->mlink.next;
            ioc_release_target_buffer(tbuf);
        }

        /* Create source buffer to link the connection and memory block together.
         */
        tbuf = ioc_initialize_target_buffer(con, mblk, info->mblk_id, info->flags & IOC_BIDIRECTIONAL);

        /* Application may want to know that the memory block has been connected.
         */
        ioc_new_root_event(root, IOC_MBLK_CONNECTED_AS_TARGET, OS_NULL, mblk,
            root->callback_context);

        /* Mark that we need to send memory block info back. If pointer is
           set, do nothing because the source buffer was added to last in list.
         */
        if ((con->flags & IOC_CONNECT_UP) == 0 &&
            con->tbuf.mbinfo_down == OS_NULL)
        {
            con->tbuf.mbinfo_down = tbuf;
        }
    }
skip2:;

}

