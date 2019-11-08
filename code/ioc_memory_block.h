/**

  @file    ioc_memory_block.h
  @brief   Memory block object.
  @author  Pekka Lehtikoski
  @version 1.1
  @date    23.6.2019

  Memory block class implementation. The communication is based on memory blocks. A memory block 
  is a byte array which is copied from a device to another. A memory block provides one directional 
  communication between two devices. To send data, application writes it to outgoing memory block.
  To receive data, it reads it from incoming memory block.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_MEMORY_BLOCK_INCLUDED
#define IOC_MEMORY_BLOCK_INCLUDED


/** Memory block numbers reserved for specific use.
 */
/*@{*/
#define IOC_DEVICE_INFO_MBLK 1
#define IOC_DEV_EXPORT_MBLK 2
#define IOC_DEV_IMPORT_MBLK 3
/*@}*/


/** Flags for ioc_initialize_memory_block() function. Bit fields.
    Note about IOC_AUTO_SYNC mode: If this flag is given to source memory buffer,
    ioc_send() is called every time ioc_write() is called. Similarly IOC_AUTO_SYNC
    causes ioc_receive() to be called every time ioc_read() is called. This is not
    really unefficient, and can be recommended in many cases.
    The IOC_ALLOW_RESIZE flag allows resizing memory block to match memory block
    information received from the other end (IO device, etc).
    The IOC_STATIC tells that memory block contains static data and doesn't need
    to be synchronized. If IOC_STATIC flag is given, also IOC_SOURCE flag must
    be set and application allocated static buffer (and may be constant) must be
    given.
 */
/*@{*/
#define IOC_TARGET 1
#define IOC_SOURCE 2
#define IOC_AUTO_SYNC 4
#define IOC_ALLOW_RESIZE 16
#define IOC_STATIC 32
/*@}*/


/* Flags for ioc_write_internal().
 */
#define IOC_SWAP_16 2 /* needs to be number of bytes */
#define IOC_SWAP_32 4 /* needs to be number of bytes */
#define IOC_SWAP_64 8 /* needs to be number of bytes */
#define IOC_SWAP_MASK 15
#define IOC_MBLK_STRING 16
#define IOC_CLEAR_MBLK_RANGE 32


/**
****************************************************************************************************
    Parameters for ioc_initialize_memory_block() function.
****************************************************************************************************
*/
typedef struct
{
    /** Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL". 
     */
    const os_char *device_name;

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in 
        context as device name, like "TEMPCTRL1".
     */
    os_short device_nr;

    /** Memory block identifier number. A communication typically has multiple
        memory blocks and this identifies the memory block among memory blocks
        of the device. 
     */
    os_short mblk_nr;

    /** Memory block name, max 15 characters.
     */
    const os_char *mblk_name;

    /** Network name.
     */
    const os_char *network_name;

    /** Buffer for memory block content. If dynamic memory allocation is supported,
        this argument can be OS_NULL, and the buffer will be allcated by the function.
        If buf argument is given, it must be pointer to buffer which can hold nro_bytes
        data (next structure member).
     */
    os_char *buf;

    /** Memory block size in bytes.
     */
    os_int nbytes;

    /** Flags, bit fields: IOC_TARGET, IOC_SOURCE, IOC_AUTO_SYNC.
     */
    os_short flags;
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
   IOC_MBLK_NR = 4,
   IOC_MBLK_NAME = 5,
   IOC_MBLK_AUTO_SYNC_FLAG = 6
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
    This can be a a worker thread is IOC_AUTO_SYNC is given. ioc_lock() is on when callback
    function is called.

****************************************************************************************************
*/
typedef void ioc_callback(
    struct iocHandle *handle,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context);

#ifndef IOC_MBLK_MAX_CALLBACK_FUNCS
#define IOC_MBLK_MAX_CALLBACK_FUNCS 2
#endif


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

    /** Pointer to data buffer.
     */
    os_char *buf;

    /** Memory block (data buffer) size in bytes.
     */
    os_int nbytes;

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
    os_short device_nr;

    /** Memory block number, identifies the memory block within device.
     */
    os_short mblk_nr;

    /** Unique memory block identifier (unique for this memory block among
        all memory blocks of this iocRoot).
     */
    os_short mblk_id;

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

    /** Buffer for communication status.
     */
    os_char status[IOC_STATUS_MEMORY_SZ];
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

/* Set memory block parameter.
 */
void ioc_memory_block_set_int_param(
    iocHandle *handle,
    iocMemoryBlockParamIx param_ix,
    os_int value);

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
    int addr,
    const os_char *buf,
    int n);

/* Write data to memory block (internal function for the iocom library).
 */
void ioc_write_internal(
    iocHandle *handle,
    int addr,
    const os_char *buf,
    int n,
    os_short flags);

/* Read data from memory block.
 */
void ioc_read(
    iocHandle *handle,
    int addr,
    os_char *buf,
    int n);

/* Read data from memory block (internal function for the iocom library).
 */
void ioc_read_internal(
    iocHandle *handle,
    int addr,
    os_char *buf,
    int n,
    os_short flags);

#ifndef IOC_SUPPORT_LOW_LEVEL_MBLK_FUNCTIONS
#define IOC_SUPPORT_LOW_LEVEL_MBLK_FUNCTIONS 1
#endif

#if IOC_SUPPORT_LOW_LEVEL_MBLK_FUNCTIONS

/* Write one bit to the memory block.
 */
void ioc_setp_bit(
    iocHandle *handle,
    int addr,
    int bit_nr,
    int value);

/* Read one bit from the memory block.
 */
char ioc_getp_bit(
    iocHandle *handle,
    int addr,
    int bit_nr);

/* Read one unsigned byte from the memory block.
 */
int ioc_getp_uchar(
    iocHandle *handle,
    int addr);

/* Write 16 bit integer to the memory block.
 */
void ioc_setp_short(
    iocHandle *handle,
    int addr,
    int value);

/* Read signed 16 bit integer from the memory block.
 */
int ioc_getp_short(
    iocHandle *handle,
    int addr);

/* Read unsigned 16 bit integer from the memory block.
 */
os_int ioc_getp_ushort(
    iocHandle *handle,
    int addr);

/* Write 32 bit integer (os_int) to the memory block.
 */
void ioc_setp_int(
    iocHandle *handle,
    int addr,
    os_int value);

/* Read 32 bit integer from the memory block.
 */
os_int ioc_getp_int(
    iocHandle *handle,
    int addr);

/* Write 32 bit floating point value to the memory block.
 */
void ioc_setp_float(
    iocHandle *handle,
    int addr,
    os_float value);

/* Read 32 bit floating point value from the memory block.
 */
os_float ioc_getp_float(
    iocHandle *handle,
    int addr);

/* Write string to the memory block.
 */
void ioc_setp_str(
    iocHandle *handle,
    int addr,
    const os_char *str,
    int n);

/* Read string from the memory block.
 */
void ioc_getp_str(
    iocHandle *handle,
    int addr,
    os_char *str,
    int n);

/* Store array of 16 bit integers to the memory block.
 */
void ioc_setp_short_array(
    iocHandle *handle,
    int addr,
    const os_short *arr,
    int n);

/* Read array of 16 bit integers from the memory block.
 */
void ioc_getp_short_array(
    iocHandle *handle,
    int addr,
    os_short *arr,
    int n);

/* Store array of 32 bit integers to the memory block.
 */
void ioc_setp_int_array(
    iocHandle *handle,
    int addr,
    const os_int *arr,
    int n);

/* Read array of 32 bit integers from the memory block.
 */
void ioc_getp_int_array(
    iocHandle *handle,
    int addr,
    os_int *arr,
    int n);

/* Store array of 32 bit floating point values to the memory block.
 */
void ioc_setp_float_array(
    iocHandle *handle,
    int addr,
    const os_float *arr,
    int n);

/* Read array of 32 bit floating point values from the memory block.
 */
void ioc_getp_float_array(
    iocHandle *handle,
    int addr,
    os_float *arr,
    int n);

#endif

/* Clear N bytes of memory block starting from specified address.
 */
void ioc_clear(
    iocHandle *handle,
    int addr,
    int n);

/* Send data synchronously.
 */
void ioc_send(
    iocHandle *handle);

/* Receive data synchronously.
 */
void ioc_receive(
    iocHandle *handle);

/* Add callback function.
 */
void ioc_add_callback(
    iocHandle *handle,
    ioc_callback func,
    void *context);

/* Remove callback function.
 */
/* void ioc_remove_callback(
    iocHandle *handle,
    ioc_callback func,
    void *context); */

/* Mark address range of changed values.
 */
void ioc_mblk_invalidate(
    iocMemoryBlock *mblk,
    int start_addr,
    int end_addr);

/* Copy data and swap byte order on big endian processors.
 */
void ioc_byte_ordered_copy(
    os_char *buf,
    const os_char *p,
    os_memsz total_sz,
    os_memsz type_sz);

/*@}*/

#endif
