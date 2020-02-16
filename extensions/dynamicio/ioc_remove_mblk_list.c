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

/**
****************************************************************************************************

  @brief Initialize the "remove memory block request" list root structure to as empty list.

  @param   dlr Pointer to root structure of the "remove memory block request" list.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_remove_mblk_req_list(
    iocDeleteMblkReqList *drl)
{
    os_memclear(drl, sizeof(iocDeleteMblkReqList));
}


/**
****************************************************************************************************

  @brief Release memory allocated for the "remove memory block request" list.

  @param   dlr Pointer to root structure of the "remove memory block request" list. Memory
           allocated for the root structure itself is not freed.
  @return  None.

****************************************************************************************************
*/
void ioc_release_remove_mblk_req_list(
    iocDeleteMblkReqList *drl)
{
    iocDeleteMblkRequest *r, *next_r;

    for (r = drl->first; r; r = next_r)
    {
        next_r = r->next;
        os_free(r, sizeof(iocDeleteMblkRequest));
    }
    os_memclear(drl, sizeof(iocDeleteMblkReqList));
}


/**
****************************************************************************************************

  @brief Add "remove memory block" request to list.

  @param   dlr Pointer to root structure of the "remove memory block request" list. Memory
           allocated for the root structure itself is not freed.
  @return  None.

****************************************************************************************************
*/
void ioc_add_request_to_remove_mblk(
    iocDeleteMblkReqList *drl,
    os_int remote_mblk_id)
{
    iocDeleteMblkRequest *r;

#if OSAL_DEBUG
    if (drl->count >= IOC_MAX_REMOVE_MBLK_REQS)
    {
        osal_debug_error_int("ioc_add_request_to_remove_mblk: Too many items on list ", drl->count);
        return;
    }
#endif

    r = (iocDeleteMblkRequest*)os_malloc(sizeof(iocDeleteMblkRequest), OS_NULL);
    if (r == OS_NULL) return;
    os_memclear(r, sizeof(iocDeleteMblkRequest));
    r->remote_mblk_id = remote_mblk_id;

    r->prev = drl->last;
    if (drl->last) {
        drl->last->next = r;
    }
    else {
        drl->first = r;
    }

#if OSAL_DEBUG
    drl->count++;
#endif
}


/**
****************************************************************************************************

  @brief Get remote memory block id to remove, -1 if none.

  @param   dlr Pointer to root structure of the "remove memory block request" list. Memory
           allocated for the root structure itself is not freed.
  @return  Remote memory block ID, or -1 if the list is empty.

****************************************************************************************************
*/
os_int ioc_get_remote_mblk_to_delete(
    iocDeleteMblkReqList *drl)
{
    if (drl->first) {
        return drl->first->remote_mblk_id;
    }

    return -1;
}


/**
****************************************************************************************************

  @brief Request has been sent, remove from list.

  Called when remote memory block returned by ioc_get_remote_mblk_to_delete() request has
  been sent through connection. Removes first item from the request list.

  @param   dlr Pointer to root structure of the "remove memory block request" list. Memory
           allocated for the root structure itself is not freed.
  @return  None.

****************************************************************************************************
*/
void ioc_remote_mblk_deleted(
    iocDeleteMblkReqList *drl)
{
    iocDeleteMblkRequest *r;

    r = drl->first;
    if (r == OS_NULL) {
        osal_debug_error("ioc_remote_mblk_deleted() called on empty list");
        return;
    }

    if (r->next) {
        r->next->prev = OS_NULL;
    }
    else {
        drl->last = OS_NULL;
    }
    drl->first = r->next;

    os_free(r, sizeof(iocDeleteMblkRequest));

#if OSAL_DEBUG
    drl->count--;
#endif
}


/**
****************************************************************************************************

  @brief Make authentication data frame.
  @anchor ioc_make_authentication_frame

  The ioc_make_authentication_frame() generates outgoing data frame which contains information
  to authenticate this IO device, etc.

  @param   con Pointer to the connection object.
  @return  OSAL_COMPLETED = All done, no more remove requests to send.
           OSAL_SUCCESS = frame was placed in outgoing data buffer
           OSAL_PENDING = Send delayed by flow control.
        ?None.

****************************************************************************************************
*/
osalStatus ioc_make_remove_mblk_req_frame(
    iocConnection *con,
    os_int remote_mblk_id)
{
    iocSendHeaderPtrs
        ptrs;

    os_uchar
        *p,
        *start;

    // root = con->link.root;

    // PACK MULTIPLE REMOVES IN ONE FRAME

    /* Set frame header.
     */
    ioc_generate_header(con, con->frame_out.buf, &ptrs, remote_mblk_id, 0);

    /* Generate frame content. Here we do not check for buffer overflow,
       we know (and trust) that it fits within one frame.
     */
    p = start = (os_uchar*)con->frame_out.buf + ptrs.header_sz;
    *(p++) = IOC_REMOVE_MBLK_REQUEST;

    /* Frame not rejected by flow control, increment frame number.
     */
    if (con->frame_out.frame_nr++ >= IOC_MAX_FRAME_NR)
    {
        con->frame_out.frame_nr = 1;
    }

    /* Finish outgoing frame with data size, frame number, and optional checksum. Quit here
       if transmission is blocked by flow control.
     */
    if (ioc_finish_frame(con, &ptrs, start, p))
        return;

    // con->authentication_sent = OS_TRUE;
osal_debug_error("HERE REMOVE REQ SENT");
return OSAL_COMPLETED;
}


#endif
