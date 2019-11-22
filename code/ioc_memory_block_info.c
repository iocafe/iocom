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
  @anchor ioc_add_mblk_to_mbinfo

  The ioc_add_mblk_to_mbinfo() function sets the new memory block as current memory block whose
  info to send, to all connections. If connection is sending other memory block information,
  nothing is done since new memory block is added as last memory block and thus it's info will
  get sent.

  ioc_lock() must be on when this function is called.

  @param   mblk Pointer to the memory block object.
  @return  None.

****************************************************************************************************
*/
void ioc_add_mblk_to_mbinfo(
    struct iocMemoryBlock *mblk)
{
    iocRoot *root;
    iocConnection *con;

    if (mblk == OS_NULL) return;
    root = mblk->link.root;

    for (con = root->con.first; con; con = con->link.next)
    {
        if (con->sinfo.current_mblk == OS_NULL)
        {
            con->sinfo.current_mblk = mblk;
        }
    }    
}


/**
****************************************************************************************************

  @brief Make sure that information about all memory blocks is sent trough a specific connection.
  @anchor ioc_add_con_to_mbinfo

  The ioc_add_con_to_mbinfo() function is called when a connection to another device is
  established. It sets that information about all memory blocks needs to be sent through the
  new connection.

  ioc_lock() must be on when this function is called.

  @param   con Pointer to the connection object (established connection).
  @return  None.

****************************************************************************************************
*/
void ioc_add_con_to_mbinfo(
    struct iocConnection *con)
{
    iocRoot *root;

    /* Be sure to ignore precious value of current_mblk.
     */
    root = con->link.root;
    con->sinfo.current_mblk = root->mblk.first;
}


/**
****************************************************************************************************

  @brief Mark that memory block information is not needed for now.
  @anchor ioc_add_con_to_mbinfo

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
  @return  None.

****************************************************************************************************
*/
struct iocMemoryBlock *ioc_get_mbinfo_to_send(
    struct iocConnection *con)
{
    return con->sinfo.current_mblk;
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
    con->sinfo.current_mblk = OS_NULL;
    if (mblk == OS_NULL) return;

    con->sinfo.current_mblk = mblk->link.next;
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
    iocHandle handle;
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf, *next_tbuf;
    os_boolean device_name_matches, device_nr_matches;
    os_boolean network_matches, mblk_matches;

#if IOC_DYNAMIC_MBLK_CODE
    iocMemoryBlockParams mbprm;
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
            osal_trace_str("~, mblk name=", info->mblk_name);

#if IOC_DYNAMIC_MBLK_CODE
            /* If we can allocate memory blocks dynamically.
             */
            if (con->flags & IOC_DYNAMIC_MBLKS)
            {
                os_memclear(&mbprm, sizeof(mbprm));
                mbprm.device_name = info->device_name;
                mbprm.device_nr = info->device_nr;
                mbprm.flags = (info->flags & IOC_SOURCE)
                    ? (IOC_TARGET|IOC_SOURCE|IOC_ALLOW_RESIZE|IOC_AUTO_SYNC)
                    : (IOC_SOURCE|IOC_ALLOW_RESIZE|IOC_AUTO_SYNC);
                mbprm.mblk_name = info->mblk_name;
                mbprm.nbytes = info->nbytes;

                if (ioc_initialize_memory_block(&handle, OS_NULL, root, &mbprm)) return;
                mblk = handle.mblk;

                /* If we have callback function, application may want to know about
                   the new memory block, so call it.
                 */
                if (root->callback_func)
                {
                    root->callback_func(root, con, &handle,
                        IOC_NEW_DYNAMIC_MBLK, root->callback_context);
                }
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
        mblk_matches = !os_strcmp(info->mblk_name, mblk->mblk_name);
        device_name_matches = !os_strcmp(info->device_name, mblk->device_name)
            || mblk->device_name[0] =='\0' || info->device_name[0] =='\0';
        device_nr_matches = info->device_nr == mblk->device_nr
            || mblk->device_nr == 0 || info->device_nr == 0;
        network_matches = !os_strcmp(info->network_name, mblk->network_name)  /* HERE WE ARE RELAXED ABOUT NETWORK NAME MATCH, THIS IS A SECURITY CONCERN. WE MAY NOT WANT TO EXECPT EMPTY BUT IN TESTERS, ETC */
            || mblk->network_name[0] =='\0' || info->network_name[0] =='\0';

        if (device_name_matches &&
            device_nr_matches &&
            network_matches &&
            mblk_matches)
        {
            osal_trace3("Memory block matched");
            break;
        }

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

    /* If we have a memory block which can be a source?
     */
    if (mblk->flags & IOC_SOURCE)
    {
        /* If the other memory block cannot be a target, do nothing more
         */
        if ((info->flags & IOC_TARGET) == 0)
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

        /* If memory block has no name, copy name from info (if there is one).
         */
        if (mblk->mblk_name[0] == '\0')
        {
            os_strncpy(mblk->mblk_name, info->mblk_name, IOC_NAME_SZ);
        }

        /* Create source buffer to link the connection and memory block together.
         */
        sbuf = ioc_initialize_source_buffer(con, mblk, info->mblk_id, OS_NULL, 0);

        /* Do initial synchronization for static memory blocks.
         */
        if (mblk->flags & IOC_STATIC)
        {
            ioc_sbuf_synchronize(sbuf);
        }

        /* If we have callback function, application may want to know that the
           memory block has been connected.
         */
        /* if (root->callback_func)
        {
            root->callback_func(root, con, mblk,
                IOC_MBLK_CONNECTED, root->callback_context);
        } */
    }
skip1:

    /* If we have memory block which can be a target
     */
    if (mblk->flags & IOC_TARGET)
    {
        /* If the other memory block cannot be a source
         */
        if ((info->flags & IOC_SOURCE) == 0)
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

        /* If memory block has no name, copy name from info (if there is one).
         */
        if (mblk->mblk_name[0] == '\0')
        {
            os_strncpy(mblk->mblk_name, info->mblk_name, IOC_NAME_SZ);
        }

        /* Create source buffer to link the connection and memory block together.
         */
        ioc_initialize_target_buffer(con, mblk, info->mblk_id, OS_NULL, 0);

        /* If we have callback function, application may want to know that the
           memory block has been connected.
         */
        /* if (root->callback_func)
        {
            root->callback_func(root, con, mblk,
                IOC_MBLK_CONNECTED, root->callback_context);
        } */

        return;
    }
skip2:;

}

