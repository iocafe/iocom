/**

  @file    ioc_target_buffer.h
  @brief   Target transfer buffers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.8.2018

  Transfer buffer binds a memory block and connection object together. It buffers changes
  to be sent through the connection.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_TARGET_BUFFER_INCLUDED
#define IOC_TARGET_BUFFER_INCLUDED


/**
****************************************************************************************************
    Member variables for synchronized buffer.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to synchronized buffer.
     */
    os_uchar *buf;

    /** Pointer to delta buffer.
     */
    os_uchar *newdata;

    /** Synchronized bytes size in bytes
     */
    int nbytes;

    /** Synchronized buffer, first changed address.
     */
    int buf_start_addr;

    /** Synchronized buffer, last changed address.
     */
    int buf_end_addr;

    /** Synchronized buffer has data flag.
     */
    os_boolean buf_used;

    /** New data buffer has data.
     */
    os_boolean has_new_data;

    /** New data buffer, first changed address.
     */
    int newdata_start_addr;

    /** New data buffer, last changed address.
     */
    int newdata_end_addr;

    /** Flag indicating that the synchronized buffer structure was allocated.
     */
    os_boolean allocated;
}
iocSynchronizedTargetBuffer;


/**
****************************************************************************************************
    This target buffer in connections's linked list of target buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the connection object.
     */
    iocConnection *con;

    /** Pointer to connection's next target buffer in linked list.
     */
    struct iocTargetBuffer *next;

    /** Pointer to connection's previous target buffer in linked list.
     */
    struct iocTargetBuffer *prev;
}
iocConnectionsTargetBufferLink;


/**
****************************************************************************************************
    This target buffer in memory block's linked list of target buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the memory block object.
     */
    iocMemoryBlock *mblk;

    /** Pointer to memory block's next target buffer in linked list.
     */
    struct iocTargetBuffer *next;

    /** Pointer to memory block's previous target buffer in linked list.
     */
    struct iocTargetBuffer *prev;
}
iocMemoryBlocksTargetBufferLink;


/**
****************************************************************************************************

  @name Target transfer buffer object structure.

  X,..

****************************************************************************************************
*/
typedef struct iocTargetBuffer
{
    /** Debug identifier must be first item in the object structure. It is used to verify
        that a function argument is pointer to correct initialized object.
     */
    IOC_DEBUG_ID

    /** Memory block identifier on remote end of connection. Identifies the memory
        block within the iocRoot.
     */
    int remote_mblk_id;

    /** Target buffers at client side need to request for data to start receiving it.
        This flag indicates if data has been requested. This is cleared at connection reset.
     */
    // os_boolean is_linked;

    /** Synchronized buffer.
     */
    iocSynchronizedTargetBuffer syncbuf;

    /** This target buffer in memory buffer's linked list of target buffers.
     */
    iocMemoryBlocksTargetBufferLink mlink;

    /** This target buffer in connections's linked list of target buffers.
     */
    iocConnectionsTargetBufferLink clink;

    /** Flag indicating that the target buffer structure was allocated.
     */
    os_boolean allocated;
}
iocTargetBuffer;


/** 
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_target_buffer() function initializes or allocates new target buffer object,
  and ioc_release_target_buffer() releases retargets associated with it. Memory allocated for the
  target buffer is freed, if the memory was allocated by ioc_initialize_target_buffer().

  The ioc_read() and ioc_write() functions are used to access data in target buffer.

****************************************************************************************************
 */
/*@{*/

/* Initialize target buffer object.
 */
iocTargetBuffer *ioc_initialize_target_buffer(
    iocTargetBuffer *tbuf,
    iocConnection *con,
    iocMemoryBlock *mblk,
    int remote_mblk_id,
    ioc_tbuf_item *itembuf,
    int nitems);

/* Release target buffer object.
 */
void ioc_release_target_buffer(
    iocTargetBuffer *tbuf);

/* Mark address range of changed values (internal).
 */
void ioc_tbuf_invalidate(
    iocTargetBuffer *sbuf,
    int start_addr,
    int end_addr);

/* Synchronize received data.
 */
void ioc_tbuf_synchronize(
    iocTargetBuffer *tbuf);

/*@}*/

#endif
