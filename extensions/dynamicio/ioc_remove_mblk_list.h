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
#if IOC_DYNAMIC_MBLK_CODE

/**
****************************************************************************************************
    Delete memory block request (item of linked list)
****************************************************************************************************
*/
typedef struct iocDeleteMblkRequest
{
    /** Identifier of memory block to remove (as top level memory block identifier)
     */
    os_int remote_mblk_id;

    /** Next request of the memory block request list. OS_NULL if this is the last item on list.
     */
    struct iocDeleteMblkRequest *next;

    /** Previous request of the memory block request list. OS_NULL if this is the first item on list.
     */
    struct iocDeleteMblkRequest *prev;
}
iocDeleteMblkRequest;


/**
****************************************************************************************************
    Delete memory block request list (root of the list)
****************************************************************************************************
*/
typedef struct iocDeleteMblkReqList
{
    /** First request of the "remove memory block request" list. OS_NULL if the list is empty.
     */
    iocDeleteMblkRequest *first;

    /** Last request of the "remove memory block request" list. OS_NULL if the list is empty.
     */
    iocDeleteMblkRequest *last;

#if OSAL_DEBUG
    /** Number of requests on list. Used to detect programming errors.
     */
    os_int count;
#endif
}
iocDeleteMblkReqList;


/** 
****************************************************************************************************
  Functions for managing memory block information
****************************************************************************************************
 */
/*@{*/

/* Initialize the "remove memory block request" list root structure to as empty list.
 */
void ioc_initialize_remove_mblk_req_list(
    iocDeleteMblkReqList *drl);

/* Release memory allocated for the "remove memory block request" list.
 */
void ioc_release_remove_mblk_req_list(
    iocDeleteMblkReqList *drl);

/* Add "remove memory block" request to list.
 */
void ioc_add_request_to_remove_mblk(
    iocDeleteMblkReqList *drl,
    os_int remote_mblk_id);

/* Get remote memory block id to remove, -1 if none.
 */
os_int ioc_get_remote_mblk_to_delete(
    iocDeleteMblkReqList *drl);

/* Called when remote memory block returned by ioc_get_remote_mblk_to_delete() request has
   been sent through connection. Removes first item from the request list.
 */
void ioc_remote_mblk_deleted(
    iocDeleteMblkReqList *drl);

/*@}*/

#endif
