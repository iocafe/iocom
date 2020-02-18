/**

  @file    ioc_memory_block.c
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
#include "iocom.h"

/* Forward referred static functions.
 */
static os_uint ioc_get_unique_mblk_id(
    iocRoot *root);

static void ioc_mblk_auto_sync(
    iocSourceBuffer *sbuf);


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
           - device_name Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
             identifies IO device type, like "TEMPCTRL".
           - device_nr If there are multiple devices of same type (same device name),
             this identifies the device. This number is often written in
             context as device name, like "TEMPCTRL1".
           - buf Buffer for memory block content. If dynamic memory allocation is supported,
             this argument can be OS_NULL, and the buffer will be allcated by the function.
             If buf argument is given, it must be pointer to buffer which can hold nro_bytes
             data.
           - nbytes. Memory block size in bytes (data size).
           - flags IOC_MBLK_DOWN, IOC_MBLK_UP, IOC_AUTO_SYNC.

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
    os_char *buf;
    os_int nbytes;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    /* Synchronize.
     */
    ioc_lock(root);

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
    if (buf == OS_NULL && nbytes > 0)
    {
        buf = ioc_malloc(root, nbytes, OS_NULL);
        mblk->buf_allocated = OS_TRUE;
    }

    /* Set up memory block structure.
     */
    mblk->buf = buf;
    mblk->nbytes = nbytes;
    mblk->flags = prm->flags;
    if ((prm->flags & IOC_STATIC) == 0)
    {
#if IOC_BIDIRECTIONAL_MBLK_CODE
        mblk->flags |= IOC_BIDIRECTIONAL;
#endif
        os_memclear(buf, nbytes);
    }
    mblk->local_flags = prm->local_flags;

    os_strncpy(mblk->device_name, prm->device_name, IOC_NAME_SZ);
    mblk->device_nr = prm->device_nr;
    os_strncpy(mblk->mblk_name, prm->mblk_name, IOC_NAME_SZ);
    os_strncpy(mblk->network_name, prm->network_name, IOC_NETWORK_NAME_SZ);

    /* Setup handle within memory block structure and one given as argument.
     */
    ioc_setup_handle(&mblk->handle, root, mblk);
    ioc_setup_handle(handle, root, mblk);

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
    ioc_add_mblk_to_global_mbinfo(mblk);

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

#if IOC_DYNAMIC_MBLK_CODE
    /* Memory block is being deleted, remove it from dynamic configuration.
     */
    ioc_droot_mblk_is_deleted(root->droot, mblk);
#endif

    /* Application may want to know that the memory block is being deleted.
     */
    ioc_new_root_event(root, IOC_MEMORY_BLOCK_DELETED, OS_NULL, mblk,
        root->callback_context);

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


#if IOC_DYNAMIC_MBLK_CODE
/**
****************************************************************************************************

  @brief Release dynamic memory block if it is no longer attached.
  @anchor ioc_release_dynamic_mblk_if_not_connected

  The ioc_release_dynamic_mblk_if_not_attached() function releases dynamically created memory
  block, if it is no longer attached downwards (checked trough source and target buffers)
  This function is called when connection cleans up resources.

  ioc_lock must be on when calling this function.

  @param   handle Memory block handle.
  @return  None.

****************************************************************************************************
*/
void ioc_release_dynamic_mblk_if_not_attached(
    iocMemoryBlock *mblk,
    struct iocConnection *deleting_con,
    os_boolean really_delete)
{
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf;
    iocConnection *con;

    if (mblk == OS_NULL) return;
    if ((mblk->flags & IOC_DYNAMIC) == 0) return;

    for (sbuf = mblk->sbuf.first;
         sbuf;
         sbuf = sbuf->mlink.next)
    {
        con = sbuf->clink.con;
        if (con && con != deleting_con)
        {
            if ((con->flags & IOC_CONNECT_UP) == 0) return;
        }
    }

    for (tbuf = mblk->tbuf.first;
         tbuf;
         tbuf = tbuf->mlink.next)
    {
        con = tbuf->clink.con;
        if (con && con != deleting_con)
        {
            if ((con->flags & IOC_CONNECT_UP) == 0) return;
        }
    }

    if (really_delete)
    {
        ioc_release_memory_block(&mblk->handle);
    }
    else
    {
        mblk->to_be_deleted = OS_TRUE;
    }
}
#endif


#if IOC_DYNAMIC_MBLK_CODE
/**
****************************************************************************************************

  @brief Generate "remove memory block" requests.
  @anchor ioc_generate_del_mblk_request

  We need to generate "remove memory block" requests for those memory blocks which are to
  be deleted deleted and have connections "up".

  ioc_lock must be on when calling this function.

  @param   mblk Pointer to memory block perhaps being deleted ("to_be_deleted" flag).
  @param   deleting_con Connection beging deleted. Set OS_NULL to send del request to all
           memory blocks upwards.
  @return  None.

****************************************************************************************************
*/
void ioc_generate_del_mblk_request(
    iocMemoryBlock *mblk,
    struct iocConnection *deleting_con)
{
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf;
    iocConnection *con;

    if (mblk == OS_NULL) return;
    if (!mblk->to_be_deleted) return;
    mblk->to_be_deleted = OS_FALSE;

    for (sbuf = mblk->sbuf.first;
         sbuf;
         sbuf = sbuf->mlink.next)
    {
        con = sbuf->clink.con;
        if (con && con != deleting_con)
        {
            if (con->flags & IOC_CONNECT_UP)
            {
                ioc_add_request_to_remove_mblk(&con->del_mlk_req_list, sbuf->remote_mblk_id);
            }
        }
    }

    for (tbuf = mblk->tbuf.first;
         tbuf;
         tbuf = tbuf->mlink.next)
    {
        con = tbuf->clink.con;
        if (con && con != deleting_con)
        {
            if (con->flags & IOC_CONNECT_UP)
            {
                ioc_add_request_to_remove_mblk(&con->del_mlk_req_list, tbuf->remote_mblk_id);
            }
        }
    }
}

#endif


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
        ioc_receive(handle);
        ioc_send(handle);
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
           IOC_DEVICE_NR, IOC_MBLK_AUTO_SYNC_FLAG, or IOC_MBLK_SIZE.
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

        case IOC_MBLK_AUTO_SYNC_FLAG:
            value = (mblk->flags & IOC_AUTO_SYNC) ? OS_TRUE : OS_FALSE;
            break;

        case IOC_MBLK_SZ:
            value = mblk->nbytes;
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
           IOC_NETWORK_NAME, IOC_DEVICE_NAME, IOC_DEVICE_NR, IOC_MBLK_NAME or
           IOC_MBLK_AUTO_SYNC_FLAG, or IOC_MBLK_SIZE.
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

        case IOC_MBLK_AUTO_SYNC_FLAG:
            value = (mblk->flags & IOC_AUTO_SYNC) ? OS_TRUE : OS_FALSE;
            break;

        case IOC_MBLK_SZ:
            value = mblk->nbytes;
            break;
    }

    if (value != -1)
    {
        osal_int_to_str(buf, buf_sz, value);
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
void ioc_write(
    iocHandle *handle,
    os_int addr,
    const os_char *buf,
    os_int n,
    os_short flags)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_char *p;
    os_int max_n, count;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* Check function arguments.
     */
    osal_debug_assert(buf != OS_NULL || (flags & IOC_CLEAR_MBLK_RANGE));
    osal_debug_assert(n > 0);

    /* Clip address and nuber of bytes to write within internal buffer.
     */
    max_n = mblk->nbytes - addr;
    if (max_n <= 0) goto getout;
    if (n > max_n) n = max_n;

    /* Store the data.
     */
    p = mblk->buf + addr;
    if (flags & IOC_MBLK_STRING)
    {
        count = (os_int)os_strlen((os_char*)buf) - 1;
        if (count > n) count = n;
        if (count > 0) os_memcpy(p, buf, count);
        if (n > count) os_memclear(p + count, (os_memsz)n - count);
    }
    else if (flags & IOC_CLEAR_MBLK_RANGE)
    {
        os_memclear(p, n);
    }
    else
    {
        ioc_byte_ordered_copy(p, buf, n, flags & IOC_SWAP_MASK);
    }
    ioc_mblk_invalidate(mblk, addr, addr + n - 1);

getout:
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
  @param   flags Set zero for normal operation.
           Bit fields.
           - IOC_MBLK_STRING buf is string. n is maximum number of bytes to store
             into str buffer. The string stored in buf is terminated with '\0'.
           - IOC_SWAP_16 If big endian processor, every second byte is swapped. For all swap modes
             number of bytes n must be divisible by swapped bytes.
           - IOC_SWAP_32 If big endian processor, every four byte group is swapped.
           - IOC_SWAP_64 If big endian processor, every eight byte group is swapped.
  @return  None.

****************************************************************************************************
*/
void ioc_read(
    iocHandle *handle,
    os_int addr,
    os_char *buf,
    os_int n,
    os_short flags)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_char *p;
    os_int max_n, count;

    /* Get memory block pointer and start synchronization. If memory block is not found, mark
       return zeroes.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL)
    {
        os_memclear(buf, n);
        return;
    }

    /* Check function arguments.
     */
    osal_debug_assert(buf != OS_NULL);
    osal_debug_assert(n > 0);
    osal_debug_assert(addr >= 0);

    /* Clip address and nuber of bytes to write within internal buffer.
     */
    max_n = mblk->nbytes - addr;
    if (max_n <= 0) goto getout;
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
        ioc_byte_ordered_copy(buf, p, n, flags & IOC_SWAP_MASK);
    }

getout:
    ioc_unlock(root);
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
    os_int addr,
    os_int n)
{
    ioc_write(handle, addr, OS_NULL, n, IOC_CLEAR_MBLK_RANGE);
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

    if (handle == OS_NULL) return;

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
    os_int start_addr, end_addr, i;
#if IOC_BIDIRECTIONAL_MBLK_CODE
    iocSourceBuffer *sbuf;
    os_char *spos, *dpos;
    os_uchar *bits;
    os_int bitsi;
#endif

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return;

    /* We usually have only one target buffer. Multiple target buffers relate to special
     * options like bidirectional transfers and perhaps redundancy in future.
     */
    for (tbuf = mblk->tbuf.first;
         tbuf;
         tbuf = tbuf->mlink.next)
    {
        if (tbuf && tbuf->syncbuf.buf_used)
        {
            start_addr = tbuf->syncbuf.buf_start_addr;
            end_addr = tbuf->syncbuf.buf_end_addr;

#if IOC_BIDIRECTIONAL_MBLK_CODE
            if (tbuf->syncbuf.flags & IOC_BIDIRECTIONAL)
            {
// Find if we have echo buffer
                bits = (os_uchar*)tbuf->syncbuf.buf + tbuf->syncbuf.ndata;
                dpos = mblk->buf;
                spos = tbuf->syncbuf.buf;
                for (i = start_addr; i <= end_addr; ++i)
                {
                    bitsi = i >> 3;
                    if ((bits[bitsi] >> (i & 7)) & 1)
                    {
                        dpos[i] = spos[i];
                    }
                }
                if (mblk->sbuf.first == OS_NULL)
                {
                    bitsi = start_addr >> 3;
                    i = (end_addr >> 3) - bitsi + 1;
                    os_memclear(bits + bitsi, i);
                }
            }
            else
            {
#endif
                os_memcpy(mblk->buf + start_addr,
                    tbuf->syncbuf.buf + start_addr,
                    (os_memsz)end_addr - start_addr + 1);

#if IOC_BIDIRECTIONAL_MBLK_CODE
            }
#endif

            tbuf->syncbuf.buf_used = OS_FALSE;

            for (i = 0; i < IOC_MBLK_MAX_CALLBACK_FUNCS; i++)
            {
                if (mblk->func[i])
                {
                    mblk->func[i](&mblk->handle, start_addr, end_addr,
                        IOC_MBLK_CALLBACK_RECEIVE, mblk->context[i]);
                }
            }

            /* If the memory block is also data source, echo received data there.
             * Except if we are connected downwards to same memory block from
             * which we initially received the data.
             */
            if (mblk->sbuf.first)
            {
#if IOC_BIDIRECTIONAL_MBLK_CODE
                if (tbuf->syncbuf.flags & IOC_BIDIRECTIONAL)
                {
                    bits = (os_uchar*)tbuf->syncbuf.buf + tbuf->syncbuf.ndata;

                    for (sbuf = mblk->sbuf.first;
                         sbuf;
                         sbuf = sbuf->mlink.next)
                    {
                        if (sbuf->clink.con == tbuf->clink.con)
                        {
                            if ((sbuf->remote_mblk_id == tbuf->remote_mblk_id) &&
                                (sbuf->clink.con->flags & IOC_CONNECT_UP) == 0)
                            {
                                continue;
                            }
                        }

                        /* Invalidate changed bytes only */
                        if (sbuf->syncbuf.flags & IOC_BIDIRECTIONAL)
                        {
                            for (i = start_addr; i <= end_addr; ++i)
                            {
                                bitsi = i >> 3;
                                if ((bits[bitsi] >> (i & 7)) & 1)
                                {
                                    ioc_sbuf_invalidate(sbuf, i, i);
                                }
                            }
                        }
                        else
                        {
                            ioc_sbuf_invalidate(sbuf, start_addr, end_addr);
                        }
                        if (mblk->flags & IOC_AUTO_SYNC)
                        {
                            ioc_mblk_auto_sync(sbuf);
                        }
                    }

                    bitsi = start_addr >> 3;
                    i = (end_addr >> 3) - bitsi + 1;
                    os_memclear(bits + bitsi, i);
                }
                else
                {
                    ioc_mblk_invalidate(mblk, start_addr, end_addr);
                }
#else
                ioc_mblk_invalidate(mblk, start_addr, end_addr);
#endif

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

  @param   mblk Pointer to memory block structure.
  @param   start_addr Beginning address of changes.
  @param   end_addr End address of changed.
  @return  None.

****************************************************************************************************
*/
void ioc_mblk_invalidate(
    iocMemoryBlock *mblk,
    os_int start_addr,
    os_int end_addr)
{
    iocSourceBuffer *sbuf;

    for (sbuf = mblk->sbuf.first;
         sbuf;
         sbuf = sbuf->mlink.next)
    {
        ioc_sbuf_invalidate(sbuf, start_addr, end_addr);
        if (mblk->flags & IOC_AUTO_SYNC)
        {
            ioc_mblk_auto_sync(sbuf);
        }
    }
}


#if IOC_RESIZE_MBLK_CODE
/**
****************************************************************************************************

  @brief Make sure that memory block can hold N bytes.
  @anchor ioc_resize_mblk

  The ioc_resize_mblk() function checks makes sure that memory block is at least N bytes long,
  and reallocates the buffer if necessary. Existing data is preserved and new allocation, if
  any, is filled with zeros.

  - Memory block to be resized must have IOC_ALLOW_RESIZE flag.
  - ioc_lock() must be on before calling this function.
  - This function can be used only if dynamic memory allocation is used or static pool is large
    enough to contain the reallocation.

  @param   mblk Pointer to memory block structure.
  @param   nbytes Minimim size needed.
  @param   flags IOC_RESIZE_PRECISE IOC_to allocate precisely up to nbytes. Otherwise
           memory block size will be rounded up to memory allocation block.
           Flag DISCONNECT_MBLK_ON_RESIZE disconnects memory block if it is resized.
  @return  If memory block change was changed, the function returns OSAL_COMPLETED.
           If memory block was already big enough, the function returns OSAL_SUCCESS.
           Return value OSAL_STATUS_NOT_SUPPORTED indicates that memory block doesn't have
           IOC_ALLOW_RESIZE flag set.
           Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_resize_mblk(
    iocMemoryBlock *mblk,
    os_int nbytes,
    os_short flags)
{
    iocRoot *root;
    os_char *newbuf;

    if (nbytes <= mblk->nbytes)
    {
        return OSAL_SUCCESS;
    }

    if ((mblk->flags & IOC_ALLOW_RESIZE) == 0)
    {
        return OSAL_STATUS_NOT_SUPPORTED;
    }

    /* If zero size memory block was allocated first, it can be resized.
     */
    if (!mblk->buf_allocated && mblk->buf)
    {
        osal_debug_error("Attempt to resize memory block with static buffer");
        return OSAL_STATUS_FAILED;
    }

    if (flags & IOC_DISCONNECT_MBLK_ON_RESIZE)
    {
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
    }

    root = mblk->link.root;
    newbuf = ioc_malloc(root, nbytes, OS_NULL);
    if (newbuf == OS_NULL) return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    os_memclear(newbuf + mblk->nbytes, nbytes - mblk->nbytes);
    if (mblk->buf)
    {
        os_memcpy(newbuf, mblk->buf, mblk->nbytes);
        ioc_free(root, mblk->buf, mblk->nbytes);
    }
    mblk->buf = newbuf;
    mblk->nbytes = nbytes;
    mblk->buf_allocated = OS_TRUE;

    return OSAL_COMPLETED;
}
#endif


/**
****************************************************************************************************

  @brief Trigger sending changes immediately.
  @anchor ioc_mblk_auto_sync

  Activate sending invalidated changes immediately in "auto sync" mode.

  ioc_lock() must be on before calling this function.

  @param   sbuf Pointer to the source buffer.
  @return  None.

****************************************************************************************************
*/
static void ioc_mblk_auto_sync(
    iocSourceBuffer *sbuf)
{
    if (ioc_sbuf_synchronize(sbuf))
    {
        sbuf->immediate_sync_needed = OS_TRUE;

#if OSAL_MULTITHREAD_SUPPORT
        /* Trigger communication that synchronization buffer would get processed.
         */
        if (sbuf->clink.con->worker.trig)
        {
            osal_event_set(sbuf->clink.con->worker.trig);
        }
#endif
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
  @return  Unique memory block identifier 8 .. 0xFFFFFFFF.

****************************************************************************************************
*/
static os_uint ioc_get_unique_mblk_id(
    iocRoot *root)
{
    iocMemoryBlock *mblk;
    os_uint id;
    os_int count;

    /* Just return next number
     */
    if (root->next_unique_mblk_id)
    {
        return root->next_unique_mblk_id++;
    }

    /* We run out of numbers. Strange, this can be possible only if special effort is
       made for this to happen. Handle anyhow.
     */
    count = 100000;
    while (count--)
    {
        id = (os_uint)osal_rand(IOC_MIN_UNIQUE_ID, 0xFFFFFFFFL);

        for (mblk = root->mblk.first;
             mblk;
             mblk = mblk->link.next)
        {
            if (id == mblk->mblk_id) break;
        }
        if (mblk == OS_NULL) return id;
    }

    osal_debug_error("Out of numbers");
    return 1;
}


/**
****************************************************************************************************

  @brief Copy data and swap byte order on big endian processors.
  @anchor ioc_byte_ordered_copy

  The ioc_byte_ordered_copy() function...

  @param   buf Target pointer.
  @param   p Source data pointer.
  @param   total_sz Number of bytes to copy.
  @param   type_sz Size of data type, byte order will be swapped for values 2 and 4 when
           running on big endian processor.
  @return  None.

****************************************************************************************************
*/
void ioc_byte_ordered_copy(
    os_char *buf,
    const os_char *p,
    os_memsz total_sz,
    os_memsz type_sz)
{
    if (total_sz <= 0) return;

#if OSAL_SMALL_ENDIAN == 0
    os_memsz count;

    switch (type_sz)
    {
        case 2:
            count = total_sz;
            while (count >= type_sz)
            {
                buf[1] = *(p++);
                buf[0] = *(p++);
                buf += 2;
                count -= type_sz;
            }
            break;

        case 4:
            count = total_sz;
            while (count >= type_sz)
            {
                buf[3] = *(p++); buf[2] = *(p++);
                buf[1] = *(p++); buf[0] = *(p++);
                buf += 4;
                count -= type_sz;
            }
            break;

        case 8:
            count = total_sz;
            while (count >= type_sz)
            {
                buf[7] = *(p++); buf[6] = *(p++);
                buf[5] = *(p++); buf[4] = *(p++);
                buf[3] = *(p++); buf[2] = *(p++);
                buf[1] = *(p++); buf[0] = *(p++);
                buf += 8;
                count -= type_sz;
            }
            break;

        default:
            os_memcpy(buf, p, total_sz);
            break;

    }
#else
    os_memcpy(buf, p, total_sz);
#endif
}
