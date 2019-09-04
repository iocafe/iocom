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
#define IOC_INPUT_MBLK 2
#define IOC_OUTPUT_MBLK 3
/*@}*/


/** Flags for ioc_initialize_memory_block() function. Bit fields.
    Note about IOC_AUTO_SEND mode: If this flag is given to source memory buffer,
    ioc_send() is called every time ioc_write() is called. Similarly IOC_AUTO_RECEIVE
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
#define IOC_AUTO_RECEIVE 4
#define IOC_AUTO_SEND 8
#define IOC_ALLOW_RESIZE 16
#define IOC_STATIC 32
/*@}*/


/* Flags for ioc_write_internal().
 */
#define IOC_SWAP_16 1
#define IOC_SWAP_32 2
#define IOC_SWAP_64 3
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
    /** Device name, max 11 upper case characters from 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL". 
     */
    const os_char *device_name;

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in 
        context as device name, like "TEMPCTRL1".
     */
    int device_nr;

    /** Memory block identifier number. A communication typically has multiple
        memory blocks and this identifies the memory block among memory blocks
        of the device. 
     */
    int mblk_nr; 

    /** Memory block name, max 11 characters.
     */
    const os_char *mblk_name;

    /** Buffer for memory block content. If dynamic memory allocation is supported,
        this argument can be OS_NULL, and the buffer will be allcated by the function.
        If buf argument is given, it must be pointer to buffer which can hold nro_bytes
        data (next structure member).
     */
    os_uchar *buf;

    /** Memory block size in bytes.
     */
    int nbytes;

    /** Flags, bit fields: IOC_TARGET, IOC_SOURCE, IOC_AUTO_RECEIVE, IOC_AUTO_SEND.
     */
    int flags;
} 
iocMemoryBlockParams;


/**
****************************************************************************************************
    Enumeration of parameters for ioc_get_memory_block_param() function.
****************************************************************************************************
*/
typedef enum
{
   IOC_DEVICE_NAME = 1,
   IOC_DEVICE_NR = 2,
   IOC_MBLK_NR = 3,
   IOC_MBLK_NAME = 4
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

    If multithreading is used: callbacks is called by the thread which calls ioc_receive(),
    which is worker thread is IOC_AUTO_RECEIVE is given. ioc_lock() is on when callback
    function is called.

****************************************************************************************************
*/
typedef void ioc_callback(
    struct iocMemoryBlock *mblk,
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

    /** Flags as given to ioc_initialize_memory_block()
     */
    int flags;

    /** Pointer to data buffer.
     */
    os_uchar *buf;

    /** Memory block (data buffer) size in bytes.
     */
    int nbytes;

    /** Device name, max 11 upper case characters from 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL". 
     */
    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in 
        context as device name, like "TEMPCTRL1".
     */
    int device_nr;

    /** Memory block number, identifies the memory block within device.
     */
    int mblk_nr;

    /** Unique memory block identifier (unique for this memory block among
       all memory blocks of this iocRoot).
     */
    int mblk_id;

    /** Memory block name, max 11 characters
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

  @name Functions related to iocom root object

  The ioc_initialize_memory_block() function initializes or allocates new memory block object,
  and ioc_release_memory_block() releases resources associated with it. Memory allocated for the
  memory block is freed, if the memory was allocated by ioc_initialize_memory_block().

  The ioc_read() and ioc_write() functions are used to access data in memory block.

****************************************************************************************************
 */
/*@{*/

/* Initialize memory block object.
 */
iocMemoryBlock *ioc_initialize_memory_block(
    iocMemoryBlock *mblk,
    iocRoot *root,
    iocMemoryBlockParams *prm);

/* Release memory block object.
 */
void ioc_release_memory_block(
    iocMemoryBlock *mblk);

/* Modify memory block flag (IOC_AUTO_RECEIVE or IOC_AUTO_SEND)
 */
os_boolean ioc_set_flag(
    iocMemoryBlock *mblk,
    int flag,
    os_boolean set);

/* Get memory block parameter value.
 */
os_int ioc_get_memory_block_param(
    iocMemoryBlock *mblk,
    iocMemoryBlockParamIx param_ix,
    os_char *buf,
    os_memsz buf_sz);

/* Write data to memory block.
 */
void ioc_write(
    iocMemoryBlock *mblk,
    int addr,
    const os_uchar *buf,
    int n);

/* Write data to memory block (internal function for the iocom library).
 */
void ioc_write_internal(
    iocMemoryBlock *mblk,
    int addr,
    const os_uchar *buf,
    int n,
    int flags);

/* Read data from memory block.
 */
void ioc_read(
    iocMemoryBlock *mblk,
    int addr,
    os_uchar *buf,
    int n);

/* Read data from memory block (internal function for the iocom library).
 */
void ioc_read_internal(
    iocMemoryBlock *mblk,
    int addr,
    os_uchar *buf,
    int n,
    int flags);

/* Write one bit to the memory block.
 */
void ioc_set_bit(
    iocMemoryBlock *mblk,
    int addr,
    int bit_nr,
    int value);

/* Read one bit from the memory block.
 */
char ioc_get_bit(
    iocMemoryBlock *mblk,
    int addr,
    int bit_nr);

/* Write one byte to the memory block.
 */
void ioc_set8(
    iocMemoryBlock *mblk,
    int addr,
    int value);

/* Read one signed byte from the memory block.
 */
int ioc_get8(
    iocMemoryBlock *mblk,
    int addr);

/* Read one unsigned byte from the memory block.
 */
int ioc_get8u(
    iocMemoryBlock *mblk,
    int addr);

/* Write 16 bit integer to the memory block.
 */
void ioc_set16(
    iocMemoryBlock *mblk,
    int addr,
    int value);

/* Read signed 16 bit integer from the memory block.
 */
int ioc_get16(
    iocMemoryBlock *mblk,
    int addr);

/* Read unsigned 16 bit integer from the memory block.
 */
os_int ioc_get16u(
    iocMemoryBlock *mblk,
    int addr);

/* Write 32 bit integer (os_int) to the memory block.
 */
void ioc_set32(
    iocMemoryBlock *mblk,
    int addr,
    os_int value);

/* Read 32 bit integer from the memory block.
 */
os_int ioc_get32(
    iocMemoryBlock * mblk,
    int addr);

/* Write 64 bit integer (os_int64) to the memory block.
 */
void ioc_set64(
    iocMemoryBlock *mblk,
    int addr,
    os_int64 value);

/* Read 64 bit integer from the memory block.
 */
os_int64 ioc_get64(
    iocMemoryBlock *mblk,
    int addr);

/* Write 32 bit floating point value to the memory block.
 */
void ioc_setfloat(
    iocMemoryBlock *mblk,
    int addr,
    os_float value);

/* Read 32 bit floating point value from the memory block.
 */
os_float ioc_getfloat(
    iocMemoryBlock *mblk,
    int addr);

/* Write string to the memory block.
 */
void ioc_setstring(
    iocMemoryBlock *mblk,
    int addr,
    const os_char *str,
    int n);

/* Read string from the memory block.
 */
void ioc_getstring(
    iocMemoryBlock *mblk,
    int addr,
    os_char *str,
    int n);

/* Store array of 16 bit integers to the memory block.
 */
void ioc_setarray16(
    iocMemoryBlock *mblk,
    int addr,
    const os_short *arr,
    int n);

/* Read array of 16 bit integers from the memory block.
 */
void ioc_getarray16(
    iocMemoryBlock *mblk,
    int addr,
    os_short *arr,
    int n);

/* Store array of 32 bit integers to the memory block.
 */
void ioc_setarray32(
    iocMemoryBlock *mblk,
    int addr,
    const os_int *arr,
    int n);

/* Read array of 32 bit integers from the memory block.
 */
void ioc_getarray32(
    iocMemoryBlock *mblk,
    int addr,
    os_int *arr,
    int n);

/* Store array of 32 bit floating point values to the memory block.
 */
void ioc_setfloatarray(
    iocMemoryBlock *mblk,
    int addr,
    const os_float *arr,
    int n);

/* Read array of 32 bit floating point values from the memory block.
 */
void ioc_getfloatarray(
    iocMemoryBlock *mblk,
    int addr,
    os_float *arr,
    int n);

/* Clear N bytes of memory block starting from specified address.
 */
void ioc_clear(
    iocMemoryBlock *mblk,
    int addr,
    int n);

/* Send data synchronously.
 */
void ioc_send(
    iocMemoryBlock *mblk);

/* Receive data synchronously.
 */
void ioc_receive(
    iocMemoryBlock *mblk);

/* Add callback function.
 */
void ioc_add_callback(
    iocMemoryBlock *mblk,
    ioc_callback func,
    void *context);

/* Remove callback function.
 */
/* void ioc_remove_callback(
    iocMemoryBlock *mblk,
    ioc_callback func,
    void *context); */

/*@}*/

#endif
