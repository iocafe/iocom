/**

  @file    ioc_memory_block.h
  @brief   Memory block object.
  @author  Pekka Lehtikoski
  @version 1.1
  @date    8.1.2020

  Memory block class implementation. The communication is based on memory blocks. A memory block
  is a byte array which is copied from a device to another. A memory block provides one directional
  communication between two devices. To send data, application writes it to outgoing memory block.
  To receive data, it reads it from incoming memory block.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_MEMORY_BLOCK_H_
#define IOC_MEMORY_BLOCK_H_
#include "iocom.h"

/** Flags for ioc_initialize_memory_block() function. Bit fields.
    - The IOC_ALLOW_RESIZE flag allows resizing memory block to match memory block
      information received from the other end (IO device, etc).
    - The IOC_STATIC tells that memory block contains static data and doesn't need
      to be synchronized. If IOC_STATIC flag is given, also IOC_MBLK_UP flag must
      be set and application allocated static buffer (and may be constant) must be
      given.
    - Flag IOC_DYNAMIC indicates that the memory block was dynamically
      allocated, and needs to be deleted when last source or target buffer attached
      to it is deleted.
    - As memory block flag, IOC_BIDIRECTIONAL means that memory block can support
      bidirectional transfers, not that it is used. As source/target buffer
      initialization flag this means actual use.
    - The IOC_CLOUD_ONLY flag indicates that memory block is transferred only
      between cloud server and server in local network, from local server to
      cloud server. This is used for "data" blocks containing user account information.
      The flag must be serialized in memory block info.
    - The IOC_NO_CLOUD flag indicates that this memory block is not transferred
      to between cloud server and server in local net.
    - IOC_FLOOR flag: This memory block is floor level in memory block hierarchy.
      Meaning that memory block information send to below of this level is never
      initiated.
 */
/*@{*/
#define IOC_DEFAULT 0
#define IOC_MBLK_DOWN 1
#define IOC_MBLK_UP 2
#define IOC_MBLK_RESERVED  4 /* was AUTO_SYNC */
#if IOC_BIDIRECTIONAL_MBLK_CODE
  #define IOC_BIDIRECTIONAL 8
#else
  #define IOC_BIDIRECTIONAL 8
#endif
#define IOC_DYNAMIC 16
#define IOC_ALLOW_RESIZE 32
#define IOC_STATIC 64
#define IOC_CLOUD_ONLY 128
#define IOC_NO_CLOUD 256
#define IOC_FLOOR 512

/*@}*/

/* Local memory block flags (not serialized)
 */
#define IOC_MBLK_LOCAL_AUTO_ID 1

/* Flags for ioc_write_internal().
 */
#define IOC_SWAP_16 2 /* needs to be number of bytes */
#define IOC_SWAP_32 4 /* needs to be number of bytes */
#define IOC_SWAP_64 8 /* needs to be number of bytes */
#define IOC_SWAP_MASK 15
#define IOC_MBLK_STRING 16
#define IOC_CLEAR_MBLK_RANGE 32
#define IOC_MBLK_NO_THREAD_SYNC 0x80 /* Same value used as for IOC_SIGNAL_NO_THREAD_SYNC */

/* Minimum memory block size in bytes.
 */
#define IOC_MIN_MBLK_SZ 24

/* Minumin value for unique memory block identifier.
 */
#define IOC_MIN_UNIQUE_ID 8

struct iocConnection;
struct iocMblkSignalHdr;
struct iocSourceBuffer;

/**
****************************************************************************************************
    Parameters for ioc_initialize_memory_block() function.
****************************************************************************************************
*/
typedef struct
{
#if IOC_MBLK_SPECIFIC_DEVICE_NAME

    /** Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL".
     */
    const os_char *device_name;

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in
        context as device name, like "TEMPCTRL1".
     */
    os_uint device_nr;

    /** Network name.
     */
    const os_char *network_name;
#endif

    /** Memory block name, max 15 characters.
     */
    const os_char *mblk_name;

    /** Buffer for memory block content. If dynamic memory allocation is supported,
        this argument can be OS_NULL, and the buffer will be allcated by the function.
        If buf argument is given, it must be pointer to buffer which can hold nro_bytes
        data (next structure member).
     */
    os_char *buf;

    /** Memory block size in bytes.
     */
    ioc_addr nbytes;

    /** Flags, bit fields: IOC_MBLK_DOWN, IOC_MBLK_UP.
     */
    os_short flags;

    /** Local memory block flags (not serialized).
        Flag IOC_MBLK_LOCAL_AUTO_ID: Device number was automatically generated by this process.
     */
    os_char local_flags;
}
iocMemoryBlockParams;


/**
****************************************************************************************************
    Enumeration of parameters for ioc_memory_block_get_*_param() ioc_memory_block_set_*_param()
    functions.
****************************************************************************************************
*/
typedef enum
{
   IOC_NETWORK_NAME = 1,
   IOC_DEVICE_NAME = 2,
   IOC_DEVICE_NR = 3,
   IOC_MBLK_NAME = 4,
   IOC_MBLK_SZ = 6
}
iocMemoryBlockParamIx;


/**
****************************************************************************************************
    Linked list of memory block's source buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to memory block's first source buffer in linked list.
     */
    struct iocSourceBuffer *first;

    /** Pointer to memory block's last source buffer in linked list.
     */
    struct iocSourceBuffer *last;
}
iocMemoryBlocksSourceBufferList;


/**
****************************************************************************************************
    Linked list of memory block's target buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to memory block's first target buffer in linked list.
     */
    struct iocTargetBuffer *first;

    /** Pointer to memory block's last target buffer in linked list.
     */
    struct iocTargetBuffer *last;
}
iocMemoryBlocksTargetBufferList;


/**
****************************************************************************************************
    This memory block in root's linked list of memory blocks.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the root object.
     */
    iocRoot *root;

    /** Pointer to the next memory block in linked list.
     */
    struct iocMemoryBlock *next;

    /** Pointer to the previous memory block in linked list.
     */
    struct iocMemoryBlock *prev;
}
iocMemoryBlockLink;


/**
****************************************************************************************************

    Memory block callback function type.

    If multithreading is used: callbacks is called by the thread which calls ioc_receive().
    ioc_lock() is on when callback function is called.

****************************************************************************************************
*/

/* Flags
 */
#define IOC_MBLK_CALLBACK_WRITE 1   /* Changed by local write */
#define IOC_MBLK_CALLBACK_RECEIVE 2 /* Changed by received data */

typedef void ioc_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

#ifndef IOC_MBLK_MAX_CALLBACK_FUNCS
    #if OSAL_MINIMALISTIC
        #define IOC_MBLK_MAX_CALLBACK_FUNCS 1
    #else
        #define IOC_MBLK_MAX_CALLBACK_FUNCS 2
    #endif
#endif

/* Flag for ioc_resize_mblk() function.
 */
#define IOC_DISCONNECT_MBLK_ON_RESIZE 1


/**
****************************************************************************************************

  @name Memory block object structure.

  This API presents IO as set of memory blocks which are copied trough communication from
  one device to another, for example IO board blocks could be "binary inputs", "analog inputs"
  and "binary outputs".

****************************************************************************************************
*/
typedef struct iocMemoryBlock
{
    /** Debug identifier must be first item in the object structure. It is used to verify
        that a function argument is pointer to correct initialized object.
     */
    IOC_DEBUG_ID

    /** Memory block handle structure.
     */
    iocHandle handle;

    /** Flags as given to ioc_initialize_memory_block()
     */
    os_short flags;

    /** Local memory block flags (not serialized).
        Flag IOC_MBLK_LOCAL_AUTO_ID: Device number was automatically generated by this process.
     */
    os_char local_flags;

    /** Pointer to data buffer.
     */
    os_char *buf;

    /** Memory block (data buffer) size in bytes.
     */
    ioc_addr nbytes;

#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    /** Network name.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL".
     */
    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in
        context as device name, like "TEMPCTRL1".
     */
    os_uint device_nr;
#endif

    /** Unique memory block identifier (unique for this memory block among
        all memory blocks of this iocRoot).
     */
    os_uint mblk_id;

    /** Memory block name, max 15 characters
     */
    os_char mblk_name[IOC_NAME_SZ];

    /** Callback function pointers. OS_NULL if not used.
     */
    ioc_callback *func[IOC_MBLK_MAX_CALLBACK_FUNCS];

    /** Callback context for callback functions. OS_NULL if not used.
     */
    void *context[IOC_MBLK_MAX_CALLBACK_FUNCS];

    /** Linked list of memory block's source buffers.
     */
    iocMemoryBlocksSourceBufferList sbuf;

    /** Linked list of memory block's target buffers.
     */
    iocMemoryBlocksTargetBufferList tbuf;

    /** This memory block in root's linked list of memory blocks.
     */
    iocMemoryBlockLink link;

    /** Flag indicating that the memory block structure was dynamically allocated.
     */
    os_boolean allocated;

    /** Flag indicating that the data buffer was dynamically allocated.
     */
    os_boolean buf_allocated;

#if IOC_DYNAMIC_MBLK_CODE
    /** Memory block being deleted flag, ioc_release_dynamic_mblk_if_not_attached()
        and ioc_generate_del_mblk_request() functions.
     */
    os_boolean to_be_deleted;

#endif

#if IOC_SIGNAL_RANGE_SUPPORT
    /** Pointer to fixed signal header for the memory block. OS_NULL if none.
     */
    const struct iocMblkSignalHdr *signal_hdr;
#endif
}
iocMemoryBlock;


/**
****************************************************************************************************

  @name Functions related to memory blocks

  The ioc_initialize_memory_block() function initializes or allocates new memory block object,
  and ioc_release_memory_block() releases resources associated with it. Memory allocated for the
  memory block is freed, if the memory was allocated by ioc_initialize_memory_block().

  The ioc_read() and ioc_write() functions are used to access data in memory block.

****************************************************************************************************
 */
/*@{*/

/* Initialize memory block object.
 */
osalStatus ioc_initialize_memory_block(
    iocHandle *handle,
    iocMemoryBlock *static_mblk,
    iocRoot *root,
    iocMemoryBlockParams *prm);

/* Release memory block object.
 */
void ioc_release_memory_block(
    iocHandle *handle);

#if IOC_DYNAMIC_MBLK_CODE
/* Release dynamic memory block if it is no longer attached.
 */
void ioc_release_dynamic_mblk_if_not_attached(
    iocMemoryBlock *mblk,
    struct iocConnection *deleting_con,
    os_boolean really_delete);

/* Generate "remove memory block" requests.
 */
void ioc_generate_del_mblk_request(
    iocMemoryBlock *mblk,
    struct iocConnection *deleting_con);

/* Save pointer to signal header.
 */
void ioc_mblk_set_signal_header(
    iocHandle *handle,
    const struct iocMblkSignalHdr *hdr);
#endif

/* Get memory block parameter value as integer.
 */
os_int ioc_memory_block_get_int_param(
    iocHandle *handle,
    iocMemoryBlockParamIx param_ix);

/* Get memory block parameter as string.
 */
void ioc_memory_block_get_string_param(
    iocHandle *handle,
    iocMemoryBlockParamIx param_ix,
    os_char *buf,
    os_memsz buf_sz);

/* Write data to memory block.
 */
void ioc_write(
    iocHandle *handle,
    os_int addr,
    const os_char *buf,
    os_int n,
    os_short flags);

/* Read data from memory block.
 */
void ioc_read(
    iocHandle *handle,
    os_int addr,
    os_char *buf,
    os_int n,
    os_short flags);

/* Clear N bytes of memory block starting from specified address.
 */
void ioc_clear(
    iocHandle *handle,
    os_int addr,
    os_int n);

/* Send data synchronously.
 */
void ioc_send(
    iocHandle *handle);

/* Receive data synchronously.
 */
void ioc_receive(
    iocHandle *handle);

/* Receive data synchronously (no lock).
 */
void ioc_receive_nolock(
    iocMemoryBlock *mblk);

/* Add callback function.
 */
void ioc_add_callback(
    iocHandle *handle,
    ioc_callback func,
    void *context);

#if OSAL_MINIMALISTIC==0
/* Remove a callback function.
 */
void ioc_remove_callback(
    iocHandle *handle,
    ioc_callback func,
    void *context);
#endif

/* Mark address range of changed values.
 */
void ioc_mblk_invalidate(
    iocMemoryBlock *mblk,
    os_int start_addr,
    os_int end_addr);

/* Make sure that memory block can hold N bytes.
 */
osalStatus ioc_resize_mblk(
    iocMemoryBlock *mblk,
    os_int nbytes,
    os_short flags);

/* Trigger sending changes immediately.
 */
void ioc_mblk_auto_sync(
    struct iocSourceBuffer *sbuf);

/* Copy data and swap byte order on big endian processors.
 */
void ioc_byte_ordered_copy(
    os_char *buf,
    const os_char *p,
    os_memsz total_sz,
    os_memsz type_sz);

/*@}*/

#endif
