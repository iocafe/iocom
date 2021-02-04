/**

  @file    ioc_remove_mblk_list.h
  @brief   Keep track of memory block removes.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    16.2.2020

  This is for more complex network where multiple IO devices are connected to server with
  dynamic configuration, and server is further connected to higher level server or tool
  like i-spy.

  When a device is disconnected from dynamic server, server knows to delete related memory
  blocks when the device disconnects. But since the connection between intermediate server
  and higher level server or tool is used for multiple devices, thus top level will not
  know about deleted memory blocks.

  This information is passed in "remove memory block request" from intermediate server to
  higher level. The remove memory block lists in intermediate server keeps track of memory
  blocks to remove from higher level. The list is kept for each connection and records
  memory block identifiers (of the top level software) to remove.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/* Limit for number of requests. Used to detect programming errors.
 */
#ifndef IOC_MAX_REMOVE_MBLK_REQS
#define IOC_MAX_REMOVE_MBLK_REQS 1000
#endif

/* Forward referred static functions.
 */
static void ioc_remove_mblk_req_processed(
    struct iocConnection *con);

static void ioc_remove_mblk_by_request(
    struct  iocConnection *con,
    iocMemoryBlock *mblk);


/**
****************************************************************************************************

  @brief Initialize the "remove memory block request" list root structure to as empty list.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_remove_mblk_req_list(
    struct iocConnection *con)
{
    os_memclear(&con->del_mlk_req_list, sizeof(iocDeleteMblkReqList));
}


/**
****************************************************************************************************

  @brief Release memory allocated for the "remove memory block request" list.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_remove_mblk_req_list(
    struct iocConnection *con)
{
    iocDeleteMblkRequest *r, *next_r;

    for (r = con->del_mlk_req_list.first; r; r = next_r)
    {
        next_r = r->next;
        os_free(r, sizeof(iocDeleteMblkRequest));
    }
    os_memclear(&con->del_mlk_req_list, sizeof(iocDeleteMblkReqList));
}


/**
****************************************************************************************************

  @brief Add "remove memory block" request to list.

  @param   con Pointer to the connection object.
  @param   remote_mblk_id Remote memory block identifier.
  @return  None.

****************************************************************************************************
*/
void ioc_add_request_to_remove_mblk(
    struct iocConnection *con,
    os_int remote_mblk_id)
{
    iocDeleteMblkRequest *r;

    /* See if we can merge this to the last one
     */
    r = con->del_mlk_req_list.last;
    if (r) if (r->n_requests < IOC_PACK_N_REQUESTS)
    {
        r->remote_mblk_id[r->n_requests++] = remote_mblk_id;
        return;
    }

#if OSAL_DEBUG
    if (con->del_mlk_req_list.count >= IOC_MAX_REMOVE_MBLK_REQS)
    {
        osal_debug_error_int("ioc_add_request_to_remove_mblk: Too many items on list: ",
            con->del_mlk_req_list.count);
        return;
    }
#endif

    r = (iocDeleteMblkRequest*)os_malloc(sizeof(iocDeleteMblkRequest), OS_NULL);
    if (r == OS_NULL) return;
    os_memclear(r, sizeof(iocDeleteMblkRequest));
    r->remote_mblk_id[0] = remote_mblk_id;
    r->n_requests = 1;

    r->prev = con->del_mlk_req_list.last;
    if (con->del_mlk_req_list.last) {
        con->del_mlk_req_list.last->next = r;
    }
    else {
        con->del_mlk_req_list.first = r;
    }
    con->del_mlk_req_list.last = r;

#if OSAL_DEBUG
    con->del_mlk_req_list.count++;
#endif
}


/**
****************************************************************************************************

  @brief The first item on request list has been sent trogh the connection, remove it from list.

  Called when renice memory block request block request has been sent to the connection.
  Removes the first item from the request list.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
static void ioc_remove_mblk_req_processed(
    struct iocConnection *con)
{
    iocDeleteMblkRequest *r;

    r = con->del_mlk_req_list.first;
    if (r == OS_NULL) {
        osal_debug_error("ioc_remove_mblk_req_processed() called on empty list");
        return;
    }

    if (r->next) {
        r->next->prev = OS_NULL;
    }
    else {
        con->del_mlk_req_list.last = OS_NULL;
    }
    con->del_mlk_req_list.first = r->next;

    os_free(r, sizeof(iocDeleteMblkRequest));

#if OSAL_DEBUG
    con->del_mlk_req_list.count--;
#endif
}


/**
****************************************************************************************************

  @brief Make remove memory block request frame.
  @anchor ioc_make_remove_mblk_req_frame

  The ioc_make_remove_mblk_req_frame() generates outgoing data frame which lists IDs of memory
  blocks to delete at remote end.

  @param   con Pointer to the connection object.
  @return  OSAL_COMPLETED = All done, no more remove requests to send.
           OSAL_SUCCESS = frame was placed in outgoing data buffer
           OSAL_PENDING = Send delayed by flow control.

****************************************************************************************************
*/
osalStatus ioc_make_remove_mblk_req_frame(
    struct iocConnection *con)
{
    iocDeleteMblkRequest *r;
    iocSendHeaderPtrs ptrs;
    os_uchar *p, *start;
    os_int i, n, bytes;

    /* If nothing to do, return that completed to indicate that all remove requests have been sent.
     */
    r = con->del_mlk_req_list.first;
    if (r == OS_NULL)
    {
        return OSAL_COMPLETED;
    }

    /* Set frame header (set number of items as mblk id field).
     */
    n = r->n_requests;
    ioc_generate_header(con, con->frame_out.buf, &ptrs, n, 0);

    /* Generate frame content. Here we do not check for buffer overflow,
       we know (and trust) that it fits within one frame.
     */
    p = start = (os_uchar*)con->frame_out.buf + ptrs.header_sz;
    *(p++) = IOC_REMOVE_MBLK_REQUEST;

    for (i = 0; i<n; i++)
    {
        bytes = osal_intser_writer((os_char*)p, r->remote_mblk_id[i]);
        p += bytes;
    }

    /* Finish outgoing frame with data size, frame number, and optional checksum. Quit here
       if transmission is blocked by flow control.
     */
    if (ioc_finish_frame(con, &ptrs, start, p))
    {
        return OSAL_PENDING;
    }

    /* We have processed this remove request block, remove from queue.
     */
    ioc_remove_mblk_req_processed(con);

    osal_trace("remove mblk request sent");
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Process "remove memory block" request frame received from socket or serial port.
  @anchor ioc_ioc_process_remove_mblk_req_frame

  The ioc_process_remove_mblk_req_frame() function is called once a system frame
  containing remove memory block request list is received.

  ioc_lock() must be on before calling this function.

  @param   con Pointer to the connection object.
  @param   mblk_id Memory block identifier in this end.
  @param   data Received data, can be compressed and delta encoded, check flags.

  @return  OSAL_SUCCESS if succesfull. Other values indicate a corrupted frame.

****************************************************************************************************
*/
osalStatus ioc_process_remove_mblk_req_frame(
    struct iocConnection *con,
    os_uint n_requests,
    os_char *data)
{
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf;
    os_char *p;
    os_long mblk_id;
    os_int i, bytes;

    if (n_requests < 1 || n_requests > IOC_PACK_ABS_MAX_REQUESTS)
        return OSAL_STATUS_FAILED;

    p = data + 1; /* Skip system frame IOC_REMOVE_MBLK_REQUEST byte. */

    for (i = 0; i<(os_int)n_requests; i++)
    {
        bytes = osal_intser_reader(p, &mblk_id);
        p += bytes;

        for (tbuf = con->tbuf.first;
             tbuf;
             tbuf = tbuf->clink.next)
        {
            if (mblk_id == tbuf->mlink.mblk->mblk_id)
            {
                ioc_remove_mblk_by_request(con, tbuf->mlink.mblk);
                goto next_req;
            }
        }

        for (sbuf = con->sbuf.first;
             sbuf;
             sbuf = sbuf->clink.next)
        {
            if (mblk_id == sbuf->mlink.mblk->mblk_id)
            {
                ioc_remove_mblk_by_request(con, sbuf->mlink.mblk);
                goto next_req;
            }
        }

next_req:;
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Delete a memory block by request and forward the request upwards.
  @anchor ioc_remove_mblk_by_request

  The ioc_remove_mblk_by_request() function...

  ioc_lock() must be on before calling this function.

  @param   mblk Pointer to memory block to delete.
  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
static void ioc_remove_mblk_by_request(
    struct iocConnection *con,
    iocMemoryBlock *mblk)
{
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    /* If network is not authorized, report error. This may be intrusion attempt.
       IS THIS AN UNNECESSARY DOUBLE CHECK? WHEN WE ASSIGN TRANSFER BUFFER TO CONNECTION DO
       WE CHECK THIS ALREADY. MAKE SURE BEFORE REMOVING THIS. NO HARM IF HERE, MAY BE
       SECURITY BREACH IF REMOVED.
     */
    if (!ioc_is_network_authorized(con, mblk->network_name, 0))
    {
        osal_error(OSAL_WARNING, iocom_mod, OSAL_STATUS_NOT_AUTOHORIZED,
            "attempt to remove memory block in unauthorized network");
        return;
    }
#endif
#endif

    /* Send delete request to upper levels of hierarcy and delete the memory block.
     */
    mblk->to_be_deleted = OS_TRUE;
    ioc_generate_del_mblk_request(mblk, OS_NULL);
    ioc_release_memory_block(&mblk->handle);
}

#endif
