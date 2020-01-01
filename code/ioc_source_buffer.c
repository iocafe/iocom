/**

  @file    ioc_source_buffer.c
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
#include "iocom.h"


/**
****************************************************************************************************

  @brief Initialize source buffer.
  @anchor ioc_initialize_source_buffer

  The ioc_initialize_source_buffer() function initializes a source transfer buffer. Source
  buffer binds the connection and memory block together.

  A source buffer can always be allocated global variable. In this case pointer to memory to be
  initialized is given as argument and return value is the same pointer. If dynamic memory
  allocation is supported, and the sbuf argument is OS_NULL, the source buffer object is
  allocated by the function.

  @param   sbuf Pointer to static source buffer structure, or OS_NULL to allocate source buffer
           object dynamically.
  @param   con Pointer to connection object.
  @param   mblk Pointer to memory block.
  @param   remote_mblk_id Memory block identifier on remote end of connection.
           Am IO board has typically multiple memory blocks and this identifies the memory block
           within device (or more specifically under root object). 
  @param   itembuf Pointer to array of ioc_tbuf_item elements. If dynamic memory allocation
           is supported, this can be OS_NULL and the needed buffer is allocated. If given
           this array must have at least as many elements as memory block has bytes.
  @param   nitems Number of elements in itembuf array. Ignored if itembuf is OS_NULL.
  @return  Pointer to initialized source buffer object. OS_NULL if memory allocation failed.

****************************************************************************************************
*/
iocSourceBuffer *ioc_initialize_source_buffer(
    iocConnection *con,
    iocMemoryBlock *mblk,
    os_short remote_mblk_id,
    ioc_sbuf_item *itembuf,
    os_int nitems)
{
    iocRoot *root;
    iocSourceBuffer *sbuf;

    /* Check that connection and memory block are valid pointers.
     */
    osal_debug_assert(con->debug_id == 'C');
    osal_debug_assert(mblk->debug_id == 'M');

    root = con->link.root;

    /* Synchronize.
     */
    ioc_lock(root);

    sbuf = (iocSourceBuffer*)ioc_malloc(root, sizeof(iocSourceBuffer), OS_NULL);
    if (sbuf == OS_NULL)
    {
        ioc_unlock(root);
        return OS_NULL;
    }
    os_memclear(sbuf, sizeof(iocSourceBuffer));

    /* Set up synchronized buffer.
     */
    sbuf->syncbuf.nbytes = mblk->nbytes;
    if ((mblk->flags & IOC_STATIC) == 0)
    {
        if (itembuf)
        {
            osal_debug_assert(nitems == sbuf->syncbuf.nbytes);
            sbuf->syncbuf.buf = (os_char*)itembuf;
            sbuf->syncbuf.delta = sbuf->syncbuf.buf + nitems;
        }
        else
        {
            sbuf->syncbuf.buf = ioc_malloc(root, 2 * (os_memsz)sbuf->syncbuf.nbytes, OS_NULL);
            if (sbuf->syncbuf.buf == OS_NULL)
            {
                ioc_free(root, sbuf, sizeof(iocSourceBuffer));
                ioc_unlock(root);
                return OS_NULL;
            }
            os_memclear(sbuf->syncbuf.buf, 2 * sbuf->syncbuf.nbytes);
            sbuf->syncbuf.delta = sbuf->syncbuf.buf + sbuf->syncbuf.nbytes;
            sbuf->syncbuf.allocated = OS_TRUE;
        }
    }

    /* Save remote memory block identifier, always start with key frame.
     */
    sbuf->remote_mblk_id = remote_mblk_id;
    sbuf->syncbuf.make_keyframe = OS_TRUE;

    /* Save pointer to connection and memory block objects and join to linked list
       of source buffers for both connection and memory block.
     */
    sbuf->clink.con = con;
    sbuf->mlink.mblk = mblk;
    sbuf->clink.prev = con->sbuf.last;
    if (con->sbuf.last)
    {
        con->sbuf.last->clink.next = sbuf;
    }
    else
    {
        con->sbuf.first = sbuf;
    }
    con->sbuf.last = sbuf;

    sbuf->mlink.prev = mblk->sbuf.last;
    if (mblk->sbuf.last)
    {
        mblk->sbuf.last->mlink.next = sbuf;
    }
    else
    {
        mblk->sbuf.first = sbuf;
    }
    mblk->sbuf.last = sbuf;

    /* Mark source buffer structure as initialized source buffer object for debugging.
     */
    IOC_SET_DEBUG_ID(sbuf, 'S')

    /* End syncronization.
     */
    ioc_unlock(root);

    /* Return pointer to initialized source buffer.
     */
    return sbuf;
}


/**
****************************************************************************************************

  @brief Release source buffer.
  @anchor ioc_release_source_buffer

  The ioc_release_source_buffer() function releases resources allocated for the source buffer
  object. Memory allocated for the source buffer object is freed, if it was allocated by
  ioc_initialize_source_buffer().

  @param   sbuf Pointer to the source buffer object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_source_buffer(
    iocSourceBuffer *sbuf)
{
    iocRoot
        *root;

    iocConnection
        *con;

    /* Check that sbuf is valid pointer.
     */
    osal_debug_assert(sbuf->debug_id == 'S');

    /* Synchronize.
     */
    root = sbuf->clink.con->link.root;
    ioc_lock(root);

    /* If the connection has this souce buffer as current buffer for
       sending data, set current to NULL. If this is in turn for mbinfo
       reply, move mbinfo reply to next one.
     */
    con = sbuf->clink.con;
    if (con->sbuf.current == sbuf)
    {
        con->sbuf.current = OS_NULL;
    }
    if (con->sbuf.mbinfo_down == sbuf)
    {
        con->sbuf.mbinfo_down = sbuf->clink.next;
    }

    /* for (con = root->con.first;
         con;
         con = con->link.next)
    {
        if (con->sbuf.current == sbuf)
        {
            con->sbuf.current = OS_NULL;
        }

        if (con->sbuf.mbinfo_down == sbuf)
        {
            con->sbuf.mbinfo_down = sbuf->clink.next;
        }
    }
    */

    /* Remove source buffer from linked lists.
     */
    if (sbuf->clink.prev)
    {
        sbuf->clink.prev->clink.next = sbuf->clink.next;
    }
    else
    {
        sbuf->clink.con->sbuf.first = sbuf->clink.next;
    }
    if (sbuf->clink.next)
    {
        sbuf->clink.next->clink.prev = sbuf->clink.prev;
    }
    else
    {
        sbuf->clink.con->sbuf.last = sbuf->clink.prev;
    }

    if (sbuf->mlink.prev)
    {
        sbuf->mlink.prev->mlink.next = sbuf->mlink.next;
    }
    else
    {
        sbuf->mlink.mblk->sbuf.first = sbuf->mlink.next;
    }
    if (sbuf->mlink.next)
    {
        sbuf->mlink.next->mlink.prev = sbuf->mlink.prev;
    }
    else
    {
        sbuf->mlink.mblk->sbuf.last = sbuf->mlink.prev;
    }

    /* If this is source buffer of current send, the pointer is no longer valid.
     */
    if (sbuf->clink.con->sbuf.current == sbuf)
    {
        sbuf->clink.con->sbuf.current = OS_NULL;
    }

    if (sbuf->syncbuf.allocated)
    {
        ioc_free(root, sbuf->syncbuf.buf, 2 * sbuf->syncbuf.nbytes);
    }

    /* Clear allocated memory indicate that is no longer initialized (for debugging).
     */
    os_memclear(sbuf, sizeof(iocSourceBuffer));
    ioc_free(root, sbuf, sizeof(iocSourceBuffer));

    /* End syncronization.
     */
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Mark address range of changed values (internal).
  @anchor ioc_sbuf_invalidate

  The ioc_sbuf_invalidate() function marks address range as possibly changed values. This is not
  necessarily same as changed values, because same values can be written again and comparison
  is against actually transmitted values.

  ioc_lock() must be on before calling this function.

  @param   sbuf Pointer to the source buffer object.
  @param   start_addr Beginning address of changes.
  @param   end_addr End address of changed.
  @return  None.

****************************************************************************************************
*/
void ioc_sbuf_invalidate(
    iocSourceBuffer *sbuf,
    os_int start_addr,
    os_int end_addr)
{
    if (!sbuf->changed.range_set)
    {
        sbuf->changed.start_addr = start_addr;
        sbuf->changed.end_addr = end_addr;
        sbuf->changed.range_set = OS_TRUE;
    }
    else
    {
        if (start_addr < sbuf->changed.start_addr) sbuf->changed.start_addr = start_addr;
        if (end_addr > sbuf->changed.end_addr) sbuf->changed.end_addr = end_addr;
    }
}


/**
****************************************************************************************************

  @brief Synchronize data for sending.
  @anchor ioc_sbuf_synchronize

  The ioc_sbuf_synchronize() function copies changes from memory buffer to synchronization buffer
  and does the delta encoding. If synchronization buffer is in use, the function does nothing.

  Invalidated range is checked for actual changes, and shrunk from ends if there are no actual
  changes. If there is no invalidated data left, function does nothing.

  Delta encoded buffer is generated.

  ioc_lock() must be on before calling this function.

  @param   sbuf Pointer to the source buffer object.
  @return  OSA_STATUS_PENDING if we could not synchronize and synchronization as soon as possible
           is needed.

****************************************************************************************************
*/
osalStatus ioc_sbuf_synchronize(
    iocSourceBuffer *sbuf)
{
    os_char
        *buf,
        *syncbuf,
        *delta;

    os_int
        start_addr,
        end_addr,
        n,
        i;

    if ((!sbuf->changed.range_set && !sbuf->syncbuf.make_keyframe) ||
        sbuf->syncbuf.used)
    {
        return sbuf->changed.range_set ? OSAL_STATUS_PENDING : OSAL_SUCCESS;
    }

    buf = sbuf->mlink.mblk->buf;
    n = sbuf->mlink.mblk->nbytes;
    syncbuf = sbuf->syncbuf.buf;
    delta = sbuf->syncbuf.delta;
    sbuf->changed.range_set = OS_FALSE;

    /* If we want to make a key frame.
     */
    if (sbuf->syncbuf.make_keyframe)
    {
        start_addr = 0;
        end_addr = n - 1;
        if (delta) os_memcpy(delta, buf, n);
        sbuf->syncbuf.make_keyframe = OS_FALSE;
        sbuf->syncbuf.is_keyframe = OS_TRUE;
    }

    /* Static memory block (IOC_STATIC).
     */
    else if (syncbuf == OS_NULL)
    {
        start_addr = sbuf->changed.start_addr;
        end_addr = sbuf->changed.end_addr;
        sbuf->syncbuf.is_keyframe = OS_FALSE;
    }

    /* Not making a key frame or transferring static data.
       Check what is actually changed.
     */
    else
    {
        /* Shrink invalidated range if data has not actually changed.
         */
        for (start_addr = sbuf->changed.start_addr;
             start_addr <= sbuf->changed.end_addr;
             start_addr++)
        {
            if (syncbuf[start_addr] != buf[start_addr]) break;
        }

        for (end_addr = sbuf->changed.end_addr;
             end_addr >= start_addr;
             end_addr--)
        {
            if (syncbuf[end_addr] != buf[end_addr]) break;
        }
        if (end_addr < start_addr) return OSAL_SUCCESS;

        /* Do delta encoding.
         */
        for (i = start_addr; i <= end_addr; i++)
        {
            delta[i] = buf[i] - syncbuf[i];
        }

        sbuf->syncbuf.is_keyframe = OS_FALSE;
    }

    sbuf->syncbuf.start_addr = start_addr;
    sbuf->syncbuf.end_addr = end_addr;
    sbuf->syncbuf.used = OS_TRUE;

    if (syncbuf)
    {
        n = end_addr - start_addr + 1;
        os_memcpy(syncbuf + start_addr, buf + start_addr, n);
    }

#if OSAL_MULTITHREAD_SUPPORT
    /* Trigger communication that synchronization buffer would get processed.
     */
    if (sbuf->clink.con->worker.trig)
    {
        osal_event_set(sbuf->clink.con->worker.trig);
    }
#endif
    return OSAL_SUCCESS;
}
