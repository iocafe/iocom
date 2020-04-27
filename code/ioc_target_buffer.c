/**

  @file    ioc_target_buffer.c
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
#include "iocom.h"

/**
****************************************************************************************************

  @brief Initialize target buffer.
  @anchor ioc_initialize_target_buffer

  The ioc_initialize_target_buffer() function initializes a target transfer buffer. Target
  buffer binds the connection and memory block together.

  @param   con Pointer to connection object.
  @param   mblk Pointer to memory block.
  @param   remote_mblk_id Memory block identifier on remote end of connection.
           Am IO board has typically multiple memory blocks and this identifies the memory block
           within device (or more specifically under root object). 
  @param   flags Set IOC_DEFAULT (0) for default operation, or set IOC_BIDIRECTIONAL bit to create
           source buffer with byte based invalidation (change marking). The bidirectional mode is
           used two directional memory block data transfers. Requires IOC_BIDIRECTIONAL_MBLK_CODE
           define.
  @return  Pointer to initialized target buffer object. OS_NULL if memory allocation failed.

****************************************************************************************************
*/
iocTargetBuffer *ioc_initialize_target_buffer(
    iocConnection *con,
    iocMemoryBlock *mblk,
    os_short remote_mblk_id,
    os_short flags)
{
    iocRoot *root;
    iocTargetBuffer *tbuf;
#if IOC_BIDIRECTIONAL_MBLK_CODE
    os_char *p;
    os_int count;
#endif

    /* Check that connection and memory block are valid pointers.
     */
    osal_debug_assert(con->debug_id == 'C');
    osal_debug_assert(mblk->debug_id == 'M');

    /* Synchronize.
     */
    root = con->link.root;
    ioc_lock(root);

    tbuf = (iocTargetBuffer*)ioc_malloc(root, sizeof(iocTargetBuffer), OS_NULL);
    if (tbuf == OS_NULL)
    {
        ioc_unlock(root);
        return OS_NULL;
    }
    os_memclear(tbuf, sizeof(iocTargetBuffer));

    /* Save remote device and memory block numbers.
     */
    tbuf->remote_mblk_id = remote_mblk_id;

    /* Set up synchronized buffer.
     */
    tbuf->syncbuf.nbytes = mblk->nbytes;
#if IOC_BIDIRECTIONAL_MBLK_CODE
    tbuf->syncbuf.ndata = tbuf->syncbuf.nbytes;
    tbuf->syncbuf.flags = flags;
    if (flags & IOC_BIDIRECTIONAL)
    {
        tbuf->syncbuf.nbytes += (tbuf->syncbuf.nbytes + 7) / 8;
    }
#endif

    tbuf->syncbuf.buf = ioc_malloc(root, 2 * (os_memsz)tbuf->syncbuf.nbytes, OS_NULL);
    if (tbuf->syncbuf.buf == OS_NULL)
    {
        ioc_free(root, tbuf, sizeof(iocSourceBuffer));
        ioc_unlock(root);
        return OS_NULL;
    }
    tbuf->syncbuf.newdata = tbuf->syncbuf.buf + tbuf->syncbuf.nbytes;

    /* Copy data backwars to get the initial situation
     */
    os_memcpy(tbuf->syncbuf.buf, mblk->buf, mblk->nbytes);
    os_memcpy(tbuf->syncbuf.newdata, mblk->buf, mblk->nbytes);

    /* If this target buffer is for data received from device down for two
       directional communication, mark whole memory block to be updated.
       We will expect key frame first. Clear mark buffers anyhow.
     */
#if IOC_BIDIRECTIONAL_MBLK_CODE
    if (flags & IOC_BIDIRECTIONAL)
    {
        count = tbuf->syncbuf.nbytes - mblk->nbytes;

        os_memclear(tbuf->syncbuf.buf + mblk->nbytes, count);
        p = tbuf->syncbuf.newdata + mblk->nbytes;

        if ((mblk->flags & IOC_MBLK_DOWN) &&
            (con->flags & IOC_CONNECT_UP) == 0)
        {
            while (count--) *(p++) = 0xFF;
        }
        else
        {
            os_memclear(p, count);
        }
    }
#endif

    /* Save pointer to connection and memory block objects and join to linked list
       of target buffers for both connection and memory block.
     */
    tbuf->clink.con = con;
    tbuf->mlink.mblk = mblk;
    tbuf->clink.prev = con->tbuf.last;
    if (con->tbuf.last)
    {
        con->tbuf.last->clink.next = tbuf;
    }
    else
    {
        con->tbuf.first = tbuf;
    }
    con->tbuf.last = tbuf;

    tbuf->mlink.prev = mblk->tbuf.last;
    if (mblk->tbuf.last)
    {
        mblk->tbuf.last->mlink.next = tbuf;
    }
    else
    {
        mblk->tbuf.first = tbuf;
    }
    mblk->tbuf.last = tbuf;

    /* Mark target buffer structure as initialized target buffer object for debugging.
     */
    IOC_SET_DEBUG_ID(tbuf, 'T')

    /* End syncronization.
     */
    ioc_unlock(root);

    /* Return pointer to initialized target buffer.
     */
    return tbuf;
}


/**
****************************************************************************************************

  @brief Release target buffer.
  @anchor ioc_release_target_buffer

  The ioc_release_target_buffer() function releases retargets allocated for the target buffer
  object. Memory allocated for the target buffer object is freed, if it was allocated by
  ioc_initialize_target_buffer().

  @param   tbuf Pointer to the target buffer object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_target_buffer(
    iocTargetBuffer *tbuf)
{
    iocRoot
        *root;

    iocConnection
        *con;

    /* Check that tbuf is valid pointer.
     */
    osal_debug_assert(tbuf->debug_id == 'T');

    /* Synchronize.
     */
    root = tbuf->clink.con->link.root;
    ioc_lock(root);

    /* If this target buffer is in turn for mbinfo
       reply, move mbinfo reply to next one.
     */
    con = tbuf->clink.con;
    if (con->tbuf.mbinfo_down == tbuf)
    {
        con->tbuf.mbinfo_down = tbuf->clink.next;
    }

    /* Remove target buffer from linked lists.
     */
    if (tbuf->clink.prev)
    {
        tbuf->clink.prev->clink.next = tbuf->clink.next;
    }
    else
    {
        tbuf->clink.con->tbuf.first = tbuf->clink.next;
    }
    if (tbuf->clink.next)
    {
        tbuf->clink.next->clink.prev = tbuf->clink.prev;
    }
    else
    {
        tbuf->clink.con->tbuf.last = tbuf->clink.prev;
    }

    if (tbuf->mlink.prev)
    {
        tbuf->mlink.prev->mlink.next = tbuf->mlink.next;
    }
    else
    {
        tbuf->mlink.mblk->tbuf.first = tbuf->mlink.next;
    }
    if (tbuf->mlink.next)
    {
        tbuf->mlink.next->mlink.prev = tbuf->mlink.prev;
    }
    else
    {
        tbuf->mlink.mblk->tbuf.last = tbuf->mlink.prev;
    }

    ioc_free(root, tbuf->syncbuf.buf, 2 * (os_memsz)tbuf->syncbuf.nbytes);

    /* Clear allocated memory indicate that is no longer initialized (for debugging).
     */
#if OSAL_DEBUG
    os_memclear(tbuf, sizeof(iocTargetBuffer));
#endif
    ioc_free(root, tbuf, sizeof(iocTargetBuffer));

    /* End syncronization.
     */
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Mark address range of changed values (internal).
  @anchor ioc_tbuf_invalidate

  The ioc_tbuf_invalidate() function marks address range as possibly changed values. This is not
  necessarily same as changed values, because same values can be written again and comparision
  is against actually transmitted values.

  ioc_lock() must be on before calling this function.

  @param   tbuf Pointer to the target buffer object.
  @param   start_addr Beginning address of changes.
  @param   end_addr End address of changed.
  @return  None.

****************************************************************************************************
*/
void ioc_tbuf_invalidate(
    iocTargetBuffer *tbuf,
    os_int start_addr,
    os_int end_addr)
{
#if IOC_BIDIRECTIONAL_MBLK_CODE
    /* Ignore transfers of changed bits.
     */
     if (start_addr >= tbuf->syncbuf.ndata) return;
#endif

    if (!tbuf->syncbuf.has_new_data)
    {
        tbuf->syncbuf.newdata_start_addr = start_addr;
        tbuf->syncbuf.newdata_end_addr = end_addr;
        tbuf->syncbuf.has_new_data = OS_TRUE;
    }
    else
    {
        if (start_addr < tbuf->syncbuf.newdata_start_addr)
            tbuf->syncbuf.newdata_start_addr = start_addr;
        if (end_addr > tbuf->syncbuf.newdata_end_addr)
            tbuf->syncbuf.newdata_end_addr = end_addr;
    }
}


/**
****************************************************************************************************

  @brief Synchronize received data.
  @anchor ioc_tbuf_synchronize

  The ioc_tbuf_synchronize() function is called when IOC_SYNC_COMPLETE flag is received to
  indicate that all synchronized changes have been transferred.
  copies changes from memory buffer to synchronization buffer
  and does the delta encoding. If synchronization buffer is in use, the function does nothing.

  Invalidated range is checked for actual changes, and shrunk from ends if there are no actual
  changes. If there is no invalidated data left, function does nothing.

  ioc_lock() must be on before calling this function.

  @param   tbuf Pointer to the target buffer object.
  @return  None.

****************************************************************************************************
*/
void ioc_tbuf_synchronize(
    iocTargetBuffer *tbuf)
{
    os_char
        *syncbuf,
        *newdata;

    os_int
        start_addr,
        end_addr,
        n;

#if IOC_BIDIRECTIONAL_MBLK_CODE
    os_int
        bs,
        be,
        pos,
        count;

    os_char
        *src,
        *dst;
#endif

    if ((!tbuf->syncbuf.has_new_data) ||
        tbuf->syncbuf.buf == OS_NULL)
    {
        return;
    }

    syncbuf = tbuf->syncbuf.buf;
    newdata = tbuf->syncbuf.newdata;
    start_addr = tbuf->syncbuf.newdata_start_addr;
    end_addr = tbuf->syncbuf.newdata_end_addr;

#if IOC_BIDIRECTIONAL_MBLK_CODE
    if ((tbuf->syncbuf.flags & IOC_BIDIRECTIONAL) == 0)
    {
#endif
      /* Shrink invalidated range if data has not actually changed.
       */
      while (start_addr <= tbuf->syncbuf.newdata_end_addr)
      {
          if (syncbuf[start_addr] != newdata[start_addr]) break;
          start_addr++;
      }

      while (end_addr >= start_addr)
      {
          if (syncbuf[end_addr] != newdata[end_addr]) break;
          end_addr--;
      }
#if IOC_BIDIRECTIONAL_MBLK_CODE
    }
#endif

    tbuf->syncbuf.has_new_data = OS_FALSE;
    if (end_addr < start_addr) return;
    n = end_addr - start_addr + 1;

    os_memcpy(syncbuf + start_addr, newdata + start_addr, n);

#if IOC_BIDIRECTIONAL_MBLK_CODE
    if (tbuf->syncbuf.flags & IOC_BIDIRECTIONAL)
    {
        bs = start_addr >> 3;
        be = end_addr >> 3;
        pos = tbuf->syncbuf.ndata + bs;
        count = be - bs + 1;
        src = newdata + pos;
        dst = syncbuf + pos;
        while (count--)
        {
            *(dst++) |= *(src++);
        }
        /* os_memcpy(syncbuf + pos, newdata + pos, be - bs + 1); */
    }
#endif

    if (tbuf->syncbuf.buf_used)
    {
        if (start_addr < tbuf->syncbuf.buf_start_addr)
            tbuf->syncbuf.buf_start_addr = start_addr;

        if (end_addr > tbuf->syncbuf.buf_end_addr)
            tbuf->syncbuf.buf_end_addr = end_addr;
    }
    else
    {
        tbuf->syncbuf.buf_start_addr = start_addr;
        tbuf->syncbuf.buf_end_addr = end_addr;
        tbuf->syncbuf.buf_used = OS_TRUE;
    }

    /* If auto receive selected.
     */
    if (tbuf->mlink.mblk->flags & IOC_AUTO_SYNC)
    {
        ioc_receive(&tbuf->mlink.mblk->handle);
    }
}


#if IOC_DYNAMIC_MBLK_CODE
/**
****************************************************************************************************

  @Clear OSAL_STATE_CONNECTED status bit of signals no longer connected.
  @anchor ioc_tbuf_synchronize

  The ioc_tbuf_disconnect_signals() function is called when a target buffer is beging deleted
  because connection has been lost.
  -  it checks
  if there are

  Clear OSAL_STATE_CONNECTED status bit for signals.

  This function must not be called for "const" signal structure.
  ioc_lock() must be on before calling this function.

  @param   tbuf Pointer to the target buffer object.
  @return  None.

****************************************************************************************************
*/
void ioc_tbuf_disconnect_signals(
    iocTargetBuffer *tbuf)
{
    iocMemoryBlock *mblk;
    iocMblkSignalHdr *hdr;
    iocSourceBuffer *sbuf;
    iocSignal *sig;
    os_char *buf;
    os_int count, nbytes, addr;
    os_boolean changed;

    mblk = tbuf->mlink.mblk;
    if (mblk == OS_NULL) return;

    buf = mblk->buf;
    nbytes = mblk->nbytes;
    hdr = mblk->signal_hdr;
    if (hdr == OS_NULL) return;

    changed = OS_FALSE;
    count = hdr->n_signals;
    for (sig = hdr->first_signal; count--; sig++)
    {
        addr = sig->addr;
        if (addr >= 0 && addr < nbytes) {
            buf[addr] &= ~OSAL_STATE_CONNECTED;
            for (sbuf = mblk->sbuf.first;
                 sbuf;
                 sbuf = sbuf->mlink.next)
            {
                ioc_sbuf_invalidate(sbuf, addr, addr);
                changed = OS_TRUE;
            }
        }
    }

    if (changed && (mblk->flags & IOC_AUTO_SYNC))
    {
        for (sbuf = mblk->sbuf.first;
             sbuf;
             sbuf = sbuf->mlink.next)
        {
            ioc_mblk_auto_sync(sbuf);
        }
    }
}
#endif
