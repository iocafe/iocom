/**

  @file    ioc_target_buffer.h
  @brief   Target transfer buffers.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Transfer buffer binds a memory block and connection object together. It buffers changes
  to be sent through the connection.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/

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
    os_char *newdata;

    /** Synchronized bytes size in bytes
     */
    os_int nbytes;

    /** Synchronized buffer, first changed address.
     */
    os_int buf_start_addr;

    /** Synchronized buffer, last changed address.
     */
    os_int buf_end_addr;

    /** Synchronized buffer has data flag.
     */
    os_boolean buf_used;

    /** New data buffer has data.
     */
    os_boolean has_new_data;

    /** New data buffer, first changed address.
     */
    os_int newdata_start_addr;

    /** New data buffer, last changed address.
     */
    os_int newdata_end_addr;

#if IOC_BIDIRECTIONAL_MBLK_CODE
    /** IOC_BIDIRECTIONAL bit indicates bidirectional transfer.
     */
    os_short flags;

    /** Number of data bytes. If this is not bidirectional transfer, ndata equals nbytes.
        Otherwise nbytes = ndata + (ndata + 7)/8 (one "ivalidate" bit for each bit daya byte)
     */
    os_int ndata;

#endif
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
    os_short remote_mblk_id;

    /** Synchronized buffer.
     */
    iocSynchronizedTargetBuffer syncbuf;

    /** This target buffer in memory buffer's linked list of target buffers.
     */
    iocMemoryBlocksTargetBufferLink mlink;

    /** This target buffer in connections's linked list of target buffers.
     */
    iocConnectionsTargetBufferLink clink;
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
    iocConnection *con,
    iocMemoryBlock *mblk,
    os_short remote_mblk_id,
    os_short flags);

/* Release target buffer object.
 */
void ioc_release_target_buffer(
    iocTargetBuffer *tbuf);

/* Mark address range of changed values (internal).
 */
void ioc_tbuf_invalidate(
    iocTargetBuffer *sbuf,
    os_int start_addr,
    os_int end_addr);

/* Synchronize received data.
 */
void ioc_tbuf_synchronize(
    iocTargetBuffer *tbuf);

/* Clear OSAL_STATE_CONNECTED status bit of signals no longer connected.
 */
void ioc_tbuf_disconnect_signals(
    iocTargetBuffer *tbuf);

/*@}*/
