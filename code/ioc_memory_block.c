/**

  @file    ioc_memory_block.c
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
#include "iocom.h"

/* Forward referred static functions.
 */
static void ioc_mblk_invalidate(
    iocMemoryBlock *mblk,
    int start_addr,
    int end_addr);

static int ioc_get_unique_mblk_id(
    iocRoot *root);


/**
****************************************************************************************************

  @brief Initialize memory block.
  @anchor ioc_initialize_memory_block

  The ioc_initialize_memory_block() function initializes a memory block. A memory block can
  be either allocated by application or by the iocom library.

  @param   handle Pointer to memory block handle to set up.
  @param   static_mblk Pointer to memory block structure allocated by application, or OS_NULL
           to let iocom to allocate the memory block.
  @param   root Pointer to initialized root object.

  @param   prm Parameter structure. Clear parameter structure using os_memclear() and
           set the members needed. Members:
           - device_name Device name, max 15 upper case characters from 'A' - 'Z'. This
             identifies IO device type, like "TEMPCTRL".
           - device_nr If there are multiple devices of same type (same device name),
             this identifies the device. This number is often written in
             context as device name, like "TEMPCTRL1".
           - mblk_nr Memory block identifier number. A communication typically has
             multiple memory blocks and this identifies the memory block within device.
           - buf Buffer for memory block content. If dynamic memory allocation is supported,
             this argument can be OS_NULL, and the buffer will be allcated by the function.
             If buf argument is given, it must be pointer to buffer which can hold nro_bytes
             data.
           - nbytes. Memory block size in bytes (data size).
           - flags IOC_TARGET, IOC_SOURCE, IOC_AUTO_SYNC.

  @return  OSAL_SUCCESS to indicate success, other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_initialize_memory_block(
    iocHandle *handle,
    iocMemoryBlock *static_mblk,
    iocRoot *root,
    iocMemoryBlockParams *prm)
{
    iocMemoryBlock *mblk;
    os_uchar *buf;
    int nbytes;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    /* Synchronize.
     */
    ioc_lock(root);

    /* In case of errors (malloc fails)
     */
    ioc_setup_handle(handle, root, OS_NULL);

    /* Allocate memory block structure, unless allocated by application.
     */
    if (static_mblk == OS_NULL)
    {
        mblk = (iocMemoryBlock*)ioc_malloc(root, sizeof(iocMemoryBlock), OS_NULL);
        if (mblk == OS_NULL)
        {
            ioc_unlock(root);
            return OSAL_STATUS_FAILED;
        }

        os_memclear(mblk, sizeof(iocMemoryBlock));
        mblk->allocated = OS_TRUE;
    }
    else
    {
        mblk = static_mblk;
        os_memclear(mblk, sizeof(iocMemoryBlock));
    }

    /* Allocate buffer for memory block content, unless allocated by application.
     */
    buf = prm->buf;
    nbytes = prm->nbytes;
    if (buf == OS_NULL)
    {
        buf = ioc_malloc(root, nbytes, OS_NULL);
        mblk->buf_allocated = OS_TRUE;
    }

    /* Setup handle within memory block structure and one given as argument.
     */
    ioc_setup_handle(&mblk->handle, root, mblk);
    ioc_setup_handle(handle, root, mblk);

    /* Set up memory block structure.
     */
    mblk->buf = (os_uchar*)buf;
    mblk->nbytes = nbytes;
    if ((prm->flags & IOC_STATIC) == 0)
    {
        os_memclear(buf, nbytes);
    }
    mblk->mblk_nr = prm->mblk_nr;
    mblk->flags = prm->flags;
    os_strncpy(mblk->device_name, prm->device_name, IOC_NAME_SZ);
    mblk->device_nr = prm->device_nr;
    os_strncpy(mblk->mblk_name, prm->mblk_name, IOC_NAME_SZ);

    /* Generate unique memory block id within the root
     */
    mblk->mblk_id = ioc_get_unique_mblk_id(root);

    /* Save pointer to root object and join to linked list of memory blocks.
     */
    mblk->link.root = root;
    mblk->link.prev = root->mblk.last;
    if (root->mblk.last)
    {
        root->mblk.last->link.next = mblk;
    }
    else
    {
        root->mblk.first = mblk;
    }
    root->mblk.last = mblk;

    /* Mark memory block structure as initialized memory block object for debugging.
     */
    IOC_SET_DEBUG_ID(mblk, 'M')

    /* If this memory block is created after connections are established,
       mark to transfer it's info.
     */
    ioc_add_mblk_to_mbinfo(mblk);

    /* End synchronization.
     */
    ioc_unlock(root);

    /* Return pointer to initialized memory block.
     */
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Release memory block.
  @anchor ioc_release_memory_block

  The ioc_release_memory_block() function releases resources allocated for the memory block
  object. Memory allocated for the memory block object is freed, if it was allocated by
  ioc_initialize_memory_block().

  @param   handle Memory block handle.
  @return  None.

****************************************************************************************************
*/
void ioc_release_memory_block(
    iocHandle *handle)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_boolean allocated;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* Memory block is being deleted, remove it from all send info.
     */
    ioc_mbinfo_mblk_is_deleted(mblk);

    /* Terminate all handles to this memory block including the contained one.
     */
    ioc_terminate_handles(&mblk->handle);

    /* Release all source buffers.
     */
    while (mblk->sbuf.first)
    {
        ioc_release_source_buffer(mblk->sbuf.first);
    }

    /* Release all terget buffers.
     */
    while (mblk->tbuf.first)
    {
        ioc_release_target_buffer(mblk->tbuf.first);
    }

    /* Remove memory block from linked list.
     */
    if (mblk->link.prev)
    {
        mblk->link.prev->link.next = mblk->link.next;
    }
    else
    {
        mblk->link.root->mblk.first = mblk->link.next;
    }
    if (mblk->link.next)
    {
        mblk->link.next->link.prev = mblk->link.prev;
    }
    else
    {
        mblk->link.root->mblk.last = mblk->link.prev;
    }

    /* Free memory if allocated.
     */
    if (mblk->buf_allocated)
    {
        ioc_free(root, mblk->buf, mblk->nbytes);
    }

    /* Clear allocated memory indicate that is no longer initialized (for debugging and
       for primitive static allocation schema).
     */
    allocated = mblk->allocated;
    os_memclear(mblk, sizeof(iocMemoryBlock));

    if (allocated)
    {
        ioc_free(root, mblk, sizeof(iocMemoryBlock));
    }

    /* End syncronization.
     */
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Set memory block parameter at run time.
  @anchor ioc_memory_block_set_int_param

  The ioc_memory_block_set_int_param() function modifies memory block parameter. At the moment,
  the only supported parameter is IOC_MBLK_AUTO_SYNC_FLAG.

  IOC_MBLK_AUTO_SYNC_FLAG: Set or clear memory block's IOC_AUTO_SYNC flag. The auto sync is
  enabled, then either ioc_send() or ioc_receive() will be called when reading or writing data
  and is called by this function.

  @param   handle Memory block handle.
  @param   param_ix Parameter index, for IOC_MBLK_AUTO_SYNC_FLAG.
  @param   value If flag, zero to disable or nonzero to enable.
  @return  None.

****************************************************************************************************
*/
void ioc_memory_block_set_int_param(
    iocHandle *handle,
    iocMemoryBlockParamIx param_ix,
    os_int value)
{
    iocRoot *root;
    iocMemoryBlock *mblk;

    /* If parameter cannot be set, do nothing
     */
    if (param_ix != IOC_MBLK_AUTO_SYNC_FLAG) return;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* Modify the flag.
     */
    if (value) mblk->flags |= IOC_AUTO_SYNC;
    else mblk->flags &= ~IOC_AUTO_SYNC;

    /* Synchronize once immediately
     */
    if (value)
    {
        if (mblk->flags & IOC_SOURCE)
        {
            ioc_send(handle);
        }
        else
        {
            ioc_receive(handle);
        }
    }

    /* End syncronization.
     */
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Get memory block parameter value as integer.
  @anchor ioc_memory_block_get_int_param

  The ioc_memory_block_get_int_param() function gets a memory block parameter value.

  @param   handle Memory block handle.
  @param   param_ix Parameter index. Selects which parameter to get, one of:
           IOC_DEVICE_NR, IOC_MBLK_NR or IOC_MBLK_AUTO_SYNC_FLAG.
  @return  Parameter value as integer. -1 if cannot be converted to integer.

****************************************************************************************************
*/
os_int ioc_memory_block_get_int_param(
    iocHandle *handle,
    iocMemoryBlockParamIx param_ix)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_int value;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return -1;

    switch (param_ix)
    {
        case IOC_DEVICE_NR:
            value = mblk->device_nr;
            break;

        case IOC_MBLK_NR:
            value = mblk->mblk_nr;
            break;

        case IOC_MBLK_AUTO_SYNC_FLAG:
            value = (mblk->flags & IOC_AUTO_SYNC) ? OS_TRUE : OS_FALSE;
            break;

        default:
            value = -1;
            break;
    }

    /* End syncronization.
     */
    ioc_unlock(root);
    return value;
}


/**
****************************************************************************************************

  @brief Get memory block parameter value as string.
  @anchor ioc_memory_block_get_string_param

  The ioc_memory_block_get_string_param() function gets a memory block parameter value, either as
  string or as integer.

  @param   handle Memory block handle.
  @param   param_ix Parameter index. Selects which parameter to get, one of:
           IOC_NETWORK_NAME, IOC_DEVICE_NAME, IOC_DEVICE_NR, IOC_MBLK_NR, IOC_MBLK_NAME or
           IOC_MBLK_AUTO_SYNC_FLAG.
  @param   buf Pointer to buffer where to store parameter value as string. Empty string if
           no value.
  @param   buf_sz Buffer size in bytes.
  @return  None.

****************************************************************************************************
*/
void ioc_memory_block_get_string_param(
    iocHandle *handle,
    iocMemoryBlockParamIx param_ix,
    os_char *buf,
    os_memsz buf_sz)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_int value;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    *buf = '\0';
    value = -1;

    switch (param_ix)
    {
        case IOC_DEVICE_NAME:
            os_strncpy(buf, mblk->device_name, buf_sz);
            break;

        case IOC_MBLK_NAME:
            os_strncpy(buf, mblk->mblk_name, buf_sz);
            break;

        case IOC_NETWORK_NAME:
            os_strncpy(buf, mblk->network_name, buf_sz);
            break;

        case IOC_DEVICE_NR:
            value = mblk->device_nr;
            break;

        case IOC_MBLK_NR:
            value = mblk->mblk_nr;
            break;

        case IOC_MBLK_AUTO_SYNC_FLAG:
            value = (mblk->flags & IOC_AUTO_SYNC) ? OS_TRUE : OS_FALSE;
            break;
    }

    if (value != -1)
    {
        osal_int_to_string(buf, buf_sz, value);
    }

    /* End syncronization.
     */
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Write data to memory block.
  @anchor ioc_write

  The ioc_write() function writes data to memory block.

  Within memory block, byte order for typed numeric data, like integers, floats, etc. should
  be always least significant byte first. If we are running on big endian processor, we need
  to swap the byte order.
  Strings in memory block should be always UTF-8 encoded and '\0' terminated.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   buf Pointer to source data.
  @param   n Number of bytes to write.
  @return  None.

****************************************************************************************************
*/
void ioc_write(
    iocHandle *handle,
    int addr,
    const os_uchar *buf,
    int n)
{
    ioc_write_internal(handle, addr, buf, n, 0);
}


/**
****************************************************************************************************

  @brief Write data to memory block (internal function).
  @anchor ioc_write_internal

  The ioc_write_internal() function writes data to memory block. This function can also be
  used to write strings to specific reserved space, clear range of memory within block
  and to do typed byte swapping. The flags argument selects operation.

  Within memory block, byte order for typed numeric data, like integers, floats, etc. should
  be always least significant byte first. If we are running on big endian processor, we need
  to swap the byte order.
  Strings in memory block should be always UTF-8 encoded and '\0' terminated.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   buf Pointer to source data.
  @param   n Number of bytes to write.
  @param   flags: Bit fields.
           - IOC_MBLK_STRING buf is string, determine length with oe_strlen and copy only
             string. If string is shorter than n, the rest is filled with zeroes.
             If string is longer than n, the string is truncated. The resulting string
             in memory block always terminated with '\0'.
           - IOC_CLEAR_MBLK_RANGE. Clear n bytes of memory block starting from addr.
             Argument buf content is ignored, can be OS_NULL.
           - IOC_SWAP_16 If big endian processor, every second byte is swapped. For all
             swap modes number of bytes n must be divisible by swapped bytes.
           - IOC_SWAP_32 If big endian processor, every four byte group is swapped.
           - IOC_SWAP_64 If big endian processor, every eight byte group is swapped.

  @return  None.

****************************************************************************************************
*/
void ioc_write_internal(
    iocHandle *handle,
    int addr,
    const os_uchar *buf,
    int n,
    int flags)
{
    iocRoot *root;
    iocMemoryBlock *mblk;

    os_uchar
        *p;

    int
        max_n,
        nstat,
        count;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* Check function arguments.
     */
    osal_debug_assert(buf != OS_NULL || (flags & IOC_CLEAR_MBLK_RANGE));
    osal_debug_assert(n > 0);

    /* Handle status data writes.
     */
    if (addr < 0)
    {
        nstat = (n > -addr) ? -addr : n;
        if (buf)
        {
            ioc_status_write(mblk, addr, buf, nstat);
            buf += nstat;
        }
        if (nstat == n) return;
        addr = 0;
        n -= nstat;
    }

    /* Clip address and nuber of bytes to write within internal buffer.
     */
    max_n = mblk->nbytes - addr;
    if (max_n <= 0) return;
    if (n > max_n) n = max_n;

    /* Store the data.
     */
    p = mblk->buf + addr;
    if (flags & IOC_MBLK_STRING)
    {
        count = (int)os_strlen((os_char*)buf) - 1;
        if (count > n) count = n;
        if (count > 0) os_memcpy(p, buf, count);
        if (n > count) os_memclear(p + count, n - count);
    }
    else if (flags & IOC_CLEAR_MBLK_RANGE)
    {
        os_memclear(p, n);
    }
    else
    {
#if OSAL_SMALL_ENDIAN == 0
    switch (flags & IOC_SWAP_MASK)
    {
        default:
        case 0:
            os_memcpy(p, buf, n);
            break;

        case IOC_SWAP_16:
            count = n;
            while (count > 0)
            {
                *(p++) = buf[1]; *(p++) = buf[0];
                buf += 2;
                count -= 2;
            }
            break;

        case IOC_SWAP_32:
            count = n;
            while (count > 0)
            {
                *(p++) = buf[3]; *(p++) = buf[2];
                *(p++) = buf[1]; *(p++) = buf[0];
                buf += 4;
                count -= 4;
            }
            break;

        case IOC_SWAP_64:
            count = n;
            while (count > 0)
            {
                *(p++) = buf[7]; *(p++) = buf[6];
                *(p++) = buf[5]; *(p++) = buf[4];
                *(p++) = buf[3]; *(p++) = buf[2];
                *(p++) = buf[1]; *(p++) = buf[0];
                buf += 8;
                count -= 8;
            }
            break;
    }
#else
        os_memcpy(p, buf, n);
#endif
    }
    ioc_mblk_invalidate(mblk, addr, addr + n - 1);

    if (mblk->flags & IOC_AUTO_SYNC)
    {
        ioc_send(handle);
    }

    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Read data from memory block.
  @anchor ioc_read

  The ioc_read() function reads data from memory block.

  Within memory block, byte order for typed numeric data, like integers, floats, etc. should
  be always least significant byte first. If we are running on big endian processor, we need
  to swap the byte order after reading.
  Strings in memory block should be always UTF-8 encoded and '\0' terminated.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @param   buf Pointer to buffer where to place data.
  @param   n Number of bytes to read.
  @return  None.

****************************************************************************************************
*/
void ioc_read(
    iocHandle *handle,
    int addr,
    os_uchar *buf,
    int n)
{
    ioc_read_internal(handle, addr, buf, n, 0);
}


/**
****************************************************************************************************

  @brief Read data from memory block (internal function).
  @anchor ioc_read_internal

  The ioc_read_internal() function reads data from memory block.

  Within memory block, byte order for typed numeric data, like integers, floats, etc. should
  be always least significant byte first. If we are running on big endian processor, we need
  to swap the byte order after reading.
  Strings in memory block should be always UTF-8 encoded and '\0' terminated.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @param   buf Pointer to buffer where to place data.
  @param   n Number of bytes to read.
  @param   flags: Bit fields.
           - IOC_MBLK_STRING buf is string. n is maximum number of bytes to store
             into str buffer. The string stored in buf is terminated with '\0'.
           - IOC_SWAP_16 If big endian processor, every second byte is swapped. For all swap modes
             number of bytes n must be divisible by swapped bytes.
           - IOC_SWAP_32 If big endian processor, every four byte group is swapped.
           - IOC_SWAP_64 If big endian processor, every eight byte group is swapped.
  @return  None.

****************************************************************************************************
*/
void ioc_read_internal(
    iocHandle *handle,
    int addr,
    os_uchar *buf,
    int n,
    int flags)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_uchar *p;
    int max_n, nstat, count;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);

    /* Check function arguments.
     */
    osal_debug_assert(buf != OS_NULL);
    osal_debug_assert(n > 0);

    /* Handle status data reads.
     */
    if (addr < 0)
    {
        nstat = (n > -addr) ? -addr : n;
        ioc_status_read(mblk, addr, buf, nstat);
        if (nstat == n) return;
        addr = 0;
        buf += nstat;
        n -= nstat;
    }

    osal_debug_assert(addr >= 0);

    /* Clip address and nuber of bytes to write within internal buffer.
     */
    max_n = mblk->nbytes - addr;
    if (max_n <= 0) return;
    if (n > max_n)
    {
        os_memclear(buf + max_n, (os_memsz)n - (os_memsz)max_n);
        n = max_n;
    }

    /* Copy the data.
     */
    p = mblk->buf + addr;

    if (flags & IOC_MBLK_STRING)
    {
        count = n - 1;
        while (count > 0 && *p != '\0')
        {
            *(buf++) = *(p++);
            count--;
        }
        if (n) *buf = '\0';
    }
    else
    {
#if OSAL_SMALL_ENDIAN == 0
    switch (flags & IOC_SWAP_MASK)
    {
        default:
        case 0:
            os_memcpy(buf, p, n);
            break;

        case IOC_SWAP_16:
            count = n;
            while (count > 0)
            {
                buf[1] = *(p++);
                buf[0] = *(p++);
                buf += 2;
                count -= 2;
            }
            break;

        case IOC_SWAP_32:
            count = n;
            while (count > 0)
            {
                buf[3] = *(p++); buf[2] = *(p++);
                buf[1] = *(p++); buf[0] = *(p++);
                buf += 4;
                count -= 4;
            }
            break;

        case IOC_SWAP_64:
            count = n;
            while (count > 0)
            {
                buf[7] = *(p++); buf[6] = *(p++);
                buf[5] = *(p++); buf[4] = *(p++);
                buf[3] = *(p++); buf[2] = *(p++);
                buf[1] = *(p++); buf[0] = *(p++);
                buf += 8;
                count -= 8;
            }
            break;
    }
#else
        os_memcpy(buf, p, n);
#endif
    }

    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Write one bit to the memory block.
  @anchor ioc_set_bit

  The ioc_set_bit() function writes a single bit into the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   bit_nr Bit number 0 ... 7. Zero is the least significant bit.
  @param   Bit value to write. Can be either signed -128 ... 127, or unsigned 0 ... 255.
           At this point we do not need to care.
  @return  None.

****************************************************************************************************
*/
void ioc_set_bit(
    iocHandle *handle,
    int addr,
    int bit_nr,
    int value)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_uchar *p;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    if (addr < 0 || addr >= mblk->nbytes) return;

    /* Copy the data.
     */
    p = mblk->buf + addr;
    if (value)
    {
        *p |= (1 << bit_nr);
    }
    else
    {
        *p &= ~(1 << bit_nr);
    }

    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Read one bit from the memory block.
  @anchor ioc_get_bit

  The ioc_get_bit() function reads a single bit from memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read frin.
  @param   bit_nr Bit number 0 ... 7. Zeri is the least significant bit.
  @return  Bit value, either OS_TRUE (1) or OS_FALSE (0).

****************************************************************************************************
*/
char ioc_get_bit(
    iocHandle *handle,
    int addr,
    int bit_nr)
{
    os_uchar buf;

    ioc_read_internal(handle, addr, &buf, 1, 0);
    return (buf | (1 << bit_nr)) ? OS_TRUE : OS_FALSE;
}


/**
****************************************************************************************************

  @brief Write one byte to the memory block.
  @anchor ioc_set8

  The ioc_set8() function writes one byte of data into the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   value Byte value to write. Can be either signed -128 ... 127, or unsigned 0 ... 255.
           At this point we do not need to care.
  @return  None.

****************************************************************************************************
*/
void ioc_set8(
    iocHandle *handle,
    int addr,
    int value)
{
    os_uchar buf[1];
    buf[0] = (os_uchar)value;
    ioc_write_internal(handle, addr, buf, sizeof(buf), 0);
}


/**
****************************************************************************************************

  @brief Read one signed byte from the memory block.
  @anchor ioc_get8

  The ioc_get8() function reads one byte of data from the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @return  Byte value -128 ... 127.

****************************************************************************************************
*/
int ioc_get8(
    iocHandle *handle,
    int addr)
{
    os_char s;
    ioc_read_internal(handle, addr, (os_uchar*)&s, sizeof(os_char), 0);
    return s;
}


/**
****************************************************************************************************

  @brief Read one unsigned byte from the memory block.
  @anchor ioc_get8u

  The ioc_get8u() function reads one byte of data from the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @return  Byte value 0 ... 255.

****************************************************************************************************
*/
int ioc_get8u(
    iocHandle *handle,
    int addr)
{
    os_uchar u;
    ioc_read_internal(handle, addr, (os_uchar*)&u, sizeof(os_uchar), 0);
    return u;
}


/**
****************************************************************************************************

  @brief Write 16 bit integer (os_short) to the memory block.
  @anchor ioc_set16

  The ioc_set16() function writes 16 bit integer into the memory block. 16 bit integer will
  take two bytes space.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   value Integer value to write. Can be either signed -32768 ... 32767, or
           unsigned 0 ... 65535. At this point we do not need to care.
  @return  None.

****************************************************************************************************
*/
void ioc_set16(
    iocHandle *handle,
    int addr,
    int value)
{
    os_ushort u;
    u = (os_ushort)value;
    ioc_write_internal(handle, addr, (os_uchar*)&u, sizeof(os_ushort), IOC_SWAP_16);
}


/**
****************************************************************************************************

  @brief Read signed 16 bit integer from the memory block.
  @anchor ioc_get16

  The ioc_get16() function reads 16 bit integer from the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @return  Integer value -32768 ... 32767.

****************************************************************************************************
*/
int ioc_get16(
    iocHandle *handle,
    int addr)
{
    os_short s;
    ioc_read_internal(handle, addr, (os_uchar*)&s, sizeof(os_short), IOC_SWAP_16);
    return s;
}


/**
****************************************************************************************************

  @brief Read unsigned 16 bit integer from the memory block.
  @anchor ioc_get16u

  The ioc_get16u() function reads 16 bit integer from the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @return  Integer value 0 ... 65535.

****************************************************************************************************
*/
os_int ioc_get16u(
    iocHandle *handle,
    int addr)
{
    os_ushort u;
    ioc_read_internal(handle, addr, (os_uchar*)&u, sizeof(os_ushort), IOC_SWAP_16);
    return u;
}


/**
****************************************************************************************************

  @brief Write 32 bit integer (os_int) to the memory block.
  @anchor ioc_set32

  The ioc_set32() function writes 32 bit integer into the memory block. 32 bit integer will take
  4 bytes space.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   value Integer value to write.
  @return  None.

****************************************************************************************************
*/
void ioc_set32(
    iocHandle *handle,
    int addr,
    os_int value)
{
    ioc_write_internal(handle, addr, (os_uchar*)&value, sizeof(os_int), IOC_SWAP_32);
}


/**
****************************************************************************************************

  @brief Read 32 bit integer from the memory block.
  @anchor ioc_get32

  The ioc_get32() function reads 32 bit integer from the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @return  Integer value.

****************************************************************************************************
*/
os_int ioc_get32(
    iocHandle *handle,
    int addr)
{
    os_int s;
    ioc_read_internal(handle, addr, (os_uchar*)&s, sizeof(os_int), IOC_SWAP_32);
    return s;
}


/**
****************************************************************************************************

  @brief Write 64 bit integer (os_int64) to the memory block.
  @anchor ioc_set64

  The ioc_set64() function writes 64 bit integer into the memory block. 64 bit integer will take
  eight bytes space.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   value Integer value to write.
  @return  None.

****************************************************************************************************
*/
void ioc_set64(
    iocHandle *handle,
    int addr,
    os_int64 value)
{
    ioc_write_internal(handle, addr, (os_uchar*)&value, sizeof(os_int64), IOC_SWAP_64);
}


/**
****************************************************************************************************

  @brief Read 64 bit integer from the memory block.
  @anchor ioc_get64

  The ioc_get64() function reads 64 bit integer from the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @return  Integer value.

****************************************************************************************************
*/
os_int64 ioc_get64(
    iocHandle *handle,
    int addr)
{
    os_int64 value;
    ioc_read_internal(handle, addr, (os_uchar*)&value, sizeof(os_int64), IOC_SWAP_64);
    return value;
}


/**
****************************************************************************************************

  @brief Write 32 bit floating point value (os_float) to the memory block.
  @anchor ioc_setfloat

  The ioc_setfloat() function writes 4 byte floating pointr value into the memory block.
  This may not work on all systems, requires that 32 bit IEEE float is available. So far
  all systems I tested do support 32 bit IEEE float.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   value Integer value to write.
  @return  None.

****************************************************************************************************
*/
void ioc_setfloat(
    iocHandle *handle,
    int addr,
    os_float value)
{
    ioc_write_internal(handle, addr, (os_uchar*)&value, sizeof(os_float), IOC_SWAP_32);
}


/**
****************************************************************************************************

  @brief Read 32 bit floating point value from the memory block.
  @anchor ioc_getfloat

  The ioc_getfloat() function reads 4 byte floating point value from the memory block.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @return  Integer value.

****************************************************************************************************
*/
os_float ioc_getfloat(
    iocHandle *handle,
    int addr)
{
    os_float value;
    ioc_read_internal(handle, addr, (os_uchar*)&value, sizeof(os_float), IOC_SWAP_32);
    return value;
}


/**
****************************************************************************************************

  @brief Write string to the memory block.
  @anchor ioc_setstring

  The ioc_setstring() function writes a string to the memory block. If string is shorter than
  maximum size n, extra space is filled with '\0' characters. If string is longer than maximum
  size n minus one, string is truncated. Resulting string will always be terminated with '\0'
  character. String str should be UTF-8 encoded.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   str Pointer to string to write.
  @param   n Maximum number of memory block bytes to set.
  @return  None.

****************************************************************************************************
*/
void ioc_setstring(
    iocHandle *handle,
    int addr,
    const os_char *str,
    int n)
{
    ioc_write_internal(handle, addr, (os_uchar*)str, n, IOC_MBLK_STRING);
}


/**
****************************************************************************************************

  @brief Read string from the memory block.
  @anchor ioc_getstring

  The ioc_getstring() function reads a string from memory block and stores it in str buffer.
  n sets maximum number of characters to read, including terminating '\0' character. String
  should be UTF-8 encoded.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @param   str Pointer to buffer where to store the string.
  @param   n Size of str buffer in bytes (maximum number of bytes to read).
  @return  Integer value.

****************************************************************************************************
*/
void ioc_getstring(
    iocHandle *handle,
    int addr,
    os_char *str,
    int n)
{
    ioc_read_internal(handle, addr, (os_uchar*)str, n, IOC_MBLK_STRING);
}


/**
****************************************************************************************************

  @brief Store array of 16 bit integers to the memory block.
  @anchor ioc_setarray16

  The ioc_setarray16() function writes an array of 16 bit integers to the memory block. It is
  more efficient to use this function than to loop trough values and write them one at a time.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   arr Pointer to array to write. If writing unsigned 16 bit integers, just cast the
           pointer. It doesn't really make any difference.
  @param   n Number of elements in array (not number of bytes).
  @return  None.

****************************************************************************************************
*/
void ioc_setarray16(
    iocHandle *handle,
    int addr,
    const os_short *arr,
    int n)
{
    ioc_write_internal(handle, addr, (os_uchar*)arr, n * sizeof(os_short), IOC_SWAP_16);
}


/**
****************************************************************************************************

  @brief Read array of 16 bit integers from the memory block.
  @anchor ioc_getarray16

  The ioc_getarray16() function reads an array of 16 bit integers from the memory block. It is
  more efficient to use this function than to loop trough values and read them one at a time.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @param   arr Pointer to array where to store the data. If reading unsigned 16 bit integers,
           just cast the pointer. It doesn' really make any difference.
  @param   n Number of elements in array (not number of bytes).
  @return  None.

****************************************************************************************************
*/
void ioc_getarray16(
    iocHandle *handle,
    int addr,
    os_short *arr,
    int n)
{
    ioc_read_internal(handle, addr, (os_uchar*)arr, n * sizeof(os_short), IOC_SWAP_16);
}


/**
****************************************************************************************************

  @brief Store array of 32 bit integers to the memory block.
  @anchor ioc_setarray32

  The ioc_setarray32() function writes an array of 32 bit integers to the memory block. It is
  more efficient to use this function than to loop trough values and write them one at a time.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   arr Pointer to array to write. If writing unsigned 32 bit integers, just cast the
           pointer. It doesn't really make any difference.
  @param   n Number of elements in array (not number of bytes).
  @return  None.

****************************************************************************************************
*/
void ioc_setarray32(
    iocHandle *handle,
    int addr,
    const os_int *arr,
    int n)
{
    ioc_write_internal(handle, addr, (os_uchar*)arr, n * sizeof(os_int), IOC_SWAP_32);
}


/**
****************************************************************************************************

  @brief Read array of 32 bit integers from the memory block.
  @anchor ioc_getarray32

  The ioc_getarray32() function reads an array of 32 bit integers from the memory block. It is
  more efficient to use this function than to loop trough values and read them one at a time.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @param   arr Pointer to array where to store the data. If reading unsigned 32 bit integers,
           just cast the pointer. It doesn't really make any difference.
  @param   n Number of elements in array (not number of bytes).
  @return  None.

****************************************************************************************************
*/
void ioc_getarray32(
    iocHandle *handle,
    int addr,
    os_int *arr,
    int n)
{
    ioc_read_internal(handle, addr, (os_uchar*)arr, n * sizeof(os_int), IOC_SWAP_32);
}


/**
****************************************************************************************************

  @brief Store array of 32 bit floating point values to the memory block.
  @anchor ioc_setfloatarray

  The ioc_setfloatarray() function writes an array of 4 byte floating point values to memory
  block. It is more efficient to use this function than to loop trough values and write them
  one at a time.

  @param   handle Memory block handle.
  @param   addr Memory address to write to.
  @param   arr Pointer to array to write.
  @param   n Number of elements in array (not number of bytes).
  @return  None.

****************************************************************************************************
*/
void ioc_setfloatarray(
    iocHandle *handle,
    int addr,
    const os_float *arr,
    int n)
{
    ioc_write_internal(handle, addr, (os_uchar*)arr, n * sizeof(os_float), IOC_SWAP_32);
}


/**
****************************************************************************************************

  @brief Read array of 32 bit floating point values from the memory block.
  @anchor ioc_getfloatarray

  The ioc_getfloatarray() function reads an array of 4 byte floating point values from the memory
  block. It is more efficient to use this function than to loop trough values and read them
  one at a time.

  @param   handle Memory block handle.
  @param   addr Memory address to read from.
  @param   arr Pointer to array where to store the data.
  @param   n Number of elements in array (not number of bytes).
  @return  None.

****************************************************************************************************
*/
void ioc_getfloatarray(
    iocHandle *handle,
    int addr,
    os_float *arr,
    int n)
{
    ioc_read_internal(handle, addr, (os_uchar*)arr, n * sizeof(os_float), IOC_SWAP_32);
}


/**
****************************************************************************************************

  @brief Clear N bytes of memory block starting from specified address.
  @anchor ioc_clear

  The ioc_clear() function stores zeros into memory block. Argument addr is start addres to
  be cleared and n is the last one. This is fairly efficient and can be used to wipe even whole
  memory blocks.

  @param   handle Memory block handle.
  @param   addr Memory address start zeroing.
  @param   n Number of bytes to set to zero.
  @return  None.

****************************************************************************************************
*/
void ioc_clear(
    iocHandle *handle,
    int addr,
    int n)
{
    ioc_write_internal(handle, addr, OS_NULL, n, IOC_CLEAR_MBLK_RANGE);
}


/**
****************************************************************************************************

  @brief Send data synchronously.
  @anchor ioc_send

  The ioc_send() function pushes all writes to memory block to proceed as a snapshot. This
  function must be called from application IOC_AUTO_SYNC is not enabled (flag given as argument
  when memory block is initialized).

  Call ioc_send() function repeatedly, for example in mictorontroller's main loop. Synchronous
  sending causes all changes done in same main loop round to be transmitted together.

  It is possible to reduce data transmitted from noicy analog inputs by calling ioc_send()
  at low frequency. This assumes that analog inputs with same desired maximum update frequency
  are grouped into same memory block.

  @param   handle Memory block handle.
  @return  None.

****************************************************************************************************
*/
void ioc_send(
    iocHandle *handle)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    iocSourceBuffer *sbuf;


    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    for (sbuf = mblk->sbuf.first;
         sbuf;
         sbuf = sbuf->mlink.next)
    {
        ioc_sbuf_synchronize(sbuf);
    }
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Receive data synchronously.
  @anchor ioc_receive

  The ioc_receive() function moves received data as snapshot to be abailable for reads. This
  function must be called by application if IOC_AUTO_SYNC flag is off.
  This receives all data matching to one ioc_send() call at other end.

  @param   handle Memory block handle.
  @return  None.

****************************************************************************************************
*/
void ioc_receive(
    iocHandle *handle)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    iocTargetBuffer *tbuf;
    int start_addr, end_addr, i;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* We should have only one target buffer.
     */
    tbuf = mblk->tbuf.first;

    /* To receive delta encoding. MOVE TO IOC_RECEIVE
     */
    if (tbuf && tbuf->syncbuf.buf_used)
    {
        start_addr = tbuf->syncbuf.buf_start_addr;
        end_addr = tbuf->syncbuf.buf_end_addr;

        os_memcpy(mblk->buf + start_addr,
            tbuf->syncbuf.buf + start_addr,
            (os_memsz)end_addr - start_addr + 1);

        tbuf->syncbuf.buf_used = OS_FALSE;

        for (i = 0; i < IOC_MBLK_MAX_CALLBACK_FUNCS; i++)
        {
            if (mblk->func[i])
            {
                mblk->func[i](&mblk->handle, start_addr, end_addr, 0, mblk->context[i]);
            }
        }
    }

    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Add callback function.
  @anchor ioc_add_callback

  The ioc_add_callback() function adds a callback function to memory block. The callback function
  gets called when data is received from connection, etc. This allows application to react to
  recived data without polling it (faster and uses less processor time).

  @param   handle Memory block handle.
  @param   func Pointer to a callback function.
  @param   context Application specific pointer to be passed to callback function.
  @return  None.

****************************************************************************************************
*/
void ioc_add_callback(
    iocHandle *handle,
    ioc_callback func,
    void *context)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_int i;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* If we already got the same callback function with the same context, do nothing.
     */
    for (i = 0; i < IOC_MBLK_MAX_CALLBACK_FUNCS; i++)
    {
        if (mblk->func[i] == func &&
            mblk->context[i] == context)
        {
            ioc_unlock(root);
            return;
        }
    }

    /* Add callback function to first free position
     */
    for (i = 0; i < IOC_MBLK_MAX_CALLBACK_FUNCS; i++)
    {
        if (mblk->func[i] == OS_NULL)
        {
            mblk->func[i] = func;
            mblk->context[i] = context;
            ioc_unlock(root);
            return;
        }
    }

    osal_debug_error("Too many callback functions");
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Mark address range of changed values.
  @anchor ioc_mblk_invalidate

  The ioc_mblk_invalidate() function marks address range as possibly changed values. This is not
  necessarily same as changed values, because same values can be written again and comparison
  is against actually transmitted values.

  ioc_lock() must be on before calling this function.

  @param   handle Memory block handle.
  @param   start_addr Beginning address of changes.
  @param   end_addr End address of changed.
  @return  None.

****************************************************************************************************
*/
static void ioc_mblk_invalidate(
    iocMemoryBlock *mblk,
    int start_addr,
    int end_addr)
{
    iocSourceBuffer *sbuf;

    for (sbuf = mblk->sbuf.first;
         sbuf;
         sbuf = sbuf->mlink.next)
    {
        ioc_sbuf_invalidate(sbuf, start_addr, end_addr);
        if (mblk->flags & IOC_AUTO_SYNC) ioc_sbuf_synchronize(sbuf);
    }
}


/**
****************************************************************************************************

  @brief Create unique identifier for memory block.
  @anchor ioc_get_unique_mblk_id

  The ioc_get_unique_mblk_id() function cretes unique identifier for the memory block. Unique
  identifier is just a number from 10 to 65535 which is used by no other memory block within
  the root stucture. Identifier 0 marks not set value, and 1 - 7 + > 32767 are reserved for
  future expansion of the library, whould they ever be needed.

  ioc_lock() must be on before calling this function.

  @param   root Pointer to the root object.
  @return  Unique memory block identifier 8 .. 32767.

****************************************************************************************************
*/
static int ioc_get_unique_mblk_id(
    iocRoot *root)
{
    iocMemoryBlock *mblk;
    int id, max_id, i, j;

    #define IOC_MAX_SMALL_UNIQUE_ID 255
    #define IOC_MIN_UNIQUE_ID 8
    #define IOC_MAX_UNIQUE_ID 32767
    char mark[IOC_MAX_SMALL_UNIQUE_ID/8 + 1];

    /* Create array which flags used small identidiers and find out the biggest
       used memory block identifier.
     */
    os_memclear(mark, sizeof(mark));
    max_id = 0;
    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        id = mblk->mblk_id;
        if (id > max_id) max_id = id;
        if (id < IOC_MIN_UNIQUE_ID || id > IOC_MAX_SMALL_UNIQUE_ID) continue;
        mark[id >> 3] |= 1 << (id & 7);
    }

    /* No small free identifier, see to reserve
       next big one which has not been used.
     */
    if (max_id < IOC_MAX_SMALL_UNIQUE_ID)
    {
        return max_id >= IOC_MIN_UNIQUE_ID ? max_id + 1 : IOC_MIN_UNIQUE_ID;
    }

    /* See if we have free one small identifiers
     */
    for (i = 1; i < sizeof(mark); i++)
    {
        if ((os_uchar)mark[i] == 255) continue;
        for (j = 0; j < 8; j++)
        {
            if ((mark[i] & (1 << j)) == 0)
            {
                return 8 * i + j;
            }
        }
    }

    /* No small free identifier, see to reserve
       next big one which has not been used.
     */
    if (max_id < IOC_MAX_UNIQUE_ID)
    {
        return max_id + 1;
    }

    /* Slow trial and error (this should never or extremely rarely be
       needed in practise)
     */
    for (i = IOC_MAX_SMALL_UNIQUE_ID;
         i <= IOC_MAX_UNIQUE_ID;
         i++)
    {
        for (mblk = root->mblk.first;
             mblk;
             mblk = mblk->link.next)
        {
            if (mblk->mblk_id == i) goto notthis;
        }
        return i;

notthis:;
    }

    /* We should never get here. This indicates that we
       would have 32760 memory blocks.
     */
    osal_debug_error("Too many memory blocks?");
    return 0;
}
