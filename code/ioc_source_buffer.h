/**

  @file    ioc_source_buffer.h
  @brief   Source transfer buffers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.8.2018

  Transfer buffer binds a memory block and connection object together. It buffers changes
  to be sent through the connection.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_SOURCE_BUFFER_INCLUDED
#define IOC_SOURCE_BUFFER_INCLUDED


/**
****************************************************************************************************
    Member variables for invalidated range
****************************************************************************************************
*/
typedef struct
{
    /** Range start_addr ... end_addr has been set.
     */
    os_boolean range_set;

    /** First invalidated address
     */
    os_int start_addr;

    /** Last invalidated address
     */
    os_int end_addr;
}
iocInvalidatedRange;


/**
****************************************************************************************************
    Member variables for synchronized buffer.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to synchronized buffer.
     */
    os_char *buf;

    /** Pointer to delta buffer.
     */
    os_char *delta;

    /** Synchronized bytes size in bytes
     */
    os_int nbytes;

    /** Synchronized buffer used flag.
     */
    os_boolean used;

    /** Make key frame in next ioc_sbuf_synchronize() call.
     */
    os_boolean make_keyframe;

    /** Key frame in synchronized buffer.
     */
    os_boolean is_keyframe;

    /** Synchronization buffer, start of modifications.
     */
    os_int start_addr;

    /** Synchronization buffer end address.
     */
    os_int end_addr;

    /** Flag indicating that the synchronized buffer structure was allocated.
     */
    os_boolean allocated;
}
iocSynchronizedSourceBuffer;


/**
****************************************************************************************************
    This source buffer in connections's linked list of source buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the connection object.
     */
    iocConnection *con;

    /** Pointer to connection's next source buffer in linked list.
     */
    struct iocSourceBuffer *next;

    /** Pointer to connection's previous source buffer in linked list.
     */
    struct iocSourceBuffer *prev;
}
iocConnectionsSourceBufferLink;


/**
****************************************************************************************************
    This source buffer in memory block's linked list of source buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the memory block.
     */
    iocMemoryBlock *mblk;

    /** Pointer to memory block's next source buffer in linked list.
     */
    struct iocSourceBuffer *next;

    /** Pointer to memory block's previous source buffer in linked list.
     */
    struct iocSourceBuffer *prev;
}
iocMemoryBlocksSourceBufferLink;


/**
****************************************************************************************************

  @name Source transfer buffer object structure.

  X,..

****************************************************************************************************
*/
typedef struct iocSourceBuffer
{
    /** Debug identifier must be first item in the object structure. It is used to verify
        that a function argument is pointer to correct initialized object.
     */
    IOC_DEBUG_ID

    /** Memory block identifier on remote end of connection. Identifies the memory
        block within the device.
     */
    os_short remote_mblk_id;

    /** Invalidated (changed) range.
     */
    iocInvalidatedRange changed;

    /** Synchronized buffer.
     */
    iocSynchronizedSourceBuffer syncbuf;

    /** This source buffer in memory block's linked list of source buffers.
     */
    iocMemoryBlocksSourceBufferLink mlink;

    /** This source buffer in connections's linked list of source buffers.
     */
    iocConnectionsSourceBufferLink clink;
}
iocSourceBuffer;


/** 
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_source_buffer() function initializes or allocates new source buffer object,
  and ioc_release_source_buffer() releases resources associated with it. Memory allocated for the
  source buffer is freed, if the memory was allocated by ioc_initialize_source_buffer().

  The ioc_read() and ioc_write() functions are used to access data in source buffer.

****************************************************************************************************
 */
/*@{*/

/* Initialize source buffer object.
 */
iocSourceBuffer *ioc_initialize_source_buffer(
    iocConnection *con,
    iocMemoryBlock *mblk,
    os_short remote_mblk_id,
    ioc_sbuf_item *itembuf,
    os_int nitems);

/* Release source buffer object.
 */
void ioc_release_source_buffer(
    iocSourceBuffer *sbuf);

/* Mark address range of changed values (internal).
 */
void ioc_sbuf_invalidate(
    iocSourceBuffer *sbuf,
    os_int start_addr,
    os_int end_addr);

/* Synchronize data for sending.
 */
void ioc_sbuf_synchronize(
    iocSourceBuffer *sbuf);

/*@}*/

#endif
