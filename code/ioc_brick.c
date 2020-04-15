/**

  @file    common/ioc_brick.c
  @brief   Structres and functions related to brick transfer.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/* Initialize brick buffer (does not allocate any memory yet)
 */
void ioc_initialize_brick_buffer(
    iocBrickBuffer *b,
    const iocStreamerSignals *signals,
    iocRoot *root)
{
    os_memclear(b, sizeof(iocBrickBuffer));
    b->root = root;

    if (signals) {
        if (signals->to_device)
        {
            b->signals = &b->prm.tod;
        }
        else
        {
            b->signals = &b->prm.frd;
        }
        os_memcpy(b->signals, signals, sizeof(iocStreamerSignals));
    }
    b->prm.is_device = OS_TRUE;
}


/* Allocate buffer
 */
void ioc_allocate_brick_buffer(
    iocBrickBuffer *b,
    os_memsz buf_sz)
{
    ioc_lock(b->root);
    b->buf_n = 0;
    b->pos = 0;
    if (b->buf_sz != buf_sz)
    {
        ioc_free_brick_buffer(b);
        b->buf = (os_uchar*)os_malloc(buf_sz, OS_NULL);
        b->buf_sz = buf_sz;
    }
    ioc_unlock(b->root);
}

void ioc_free_brick_buffer(
    iocBrickBuffer *b)
{
    ioc_lock(b->root);
    if (b->buf)
    {
        os_free(b->buf, b->buf_sz);
        b->buf = OS_NULL;
        b->buf_sz = 0;
    }
    ioc_unlock(b->root);
}


/* Compress brick into buffer.
 * @param buf Buffer where to store brick header and compressed data.
 * @param src Source data, brick header + uncompressed brick data.
 * @return number of final bytes in buf (includes brick header).
 */
os_memsz ioc_compress_brick(
    os_uchar *buf,
    os_memsz buf_sz,
    os_uchar *src,
    iocBrickFormat src_format,
    os_int src_w,
    os_int src_h,
    iocBrickCompression compression)
{
    iocBrickHdr *hdr;

    /* Very dummy and limited uncompressed implementation.
     */
    os_memsz sz;
    sz = src_w * src_h + sizeof(iocBrickHdr);
    if (sz > buf_sz) sz = buf_sz;
    os_memcpy(buf, src, sz);

    hdr = (iocBrickHdr*)buf;
    hdr->compression = compression;

    return sz;
}


/* Store time stamp into the brick header (must be called before ioc_set_brick_checksum)
 */
void ioc_set_brick_timestamp(
    os_uchar *buf)
{
    iocBrickHdr *hdr;
    os_timer ti;

    hdr = (iocBrickHdr*)buf;
    os_get_timer(&ti);

#ifdef OSAL_SMALL_ENDIAN
    os_memcpy(hdr->tstamp, &ti, IOC_BITMAP_TSTAMP_SZ);
#else
    os_uchar *ss, *dd;
    int count;

    count = IOC_BITMAP_TSTAMP_SZ;
    ss = (os_uchar*)&ti;
    dd = (os_uchar*)hdr->tstamp;
    while (count--)
    {
        *(dd++) = ss[count];
    }
#endif
}


/* Store check sum within brick header
 */
void ioc_set_brick_checksum(
    os_uchar *buf,
    os_memsz buf_n)
{
    iocBrickHdr *hdr;
    os_ushort checksum;

    hdr = (iocBrickHdr*)buf;
    hdr->checksum_low = 0;
    hdr->checksum_high = 0;
    checksum = os_checksum((const os_char*)buf, buf_n, OS_NULL);
    hdr->checksum_low = (os_uchar)checksum;
    hdr->checksum_high = (os_uchar)(checksum >> 8);
}


/* Send all or part of brick data to output stream.
 */
void ioc_send_brick_data(
    iocBrickBuffer *b)
{
    os_memsz n, n_written;
    osalStatus s;

    ioc_lock(b->root);

    if (b->pos < b->buf_n)
    {
        n = b->buf_n - b->pos;
        s = ioc_streamer_write(b->stream, (const os_char*)b->buf + b->pos, n, &n_written, OSAL_STREAM_DEFAULT);
        if (s)
        {
            return;
        }

        b->pos += n_written;
    }

    /* If whole brick has been sent, mark buffer empty.
     */
    if (b->pos >= b->buf_n) {
        b->buf_n = 0;
    }

    ioc_unlock(b->root);
}


/**
****************************************************************************************************

  @brief Keep control stream for transferring IO device configuration and flash program alive.
  @anchor ioc_run_control_stream

  @return  If working in something, the function returns OSAL_SUCCESS. Return value
           OSAL_NOTHING_TO_DO indicates that this thread can be switched to slow
           idle mode as far as the control stream knows.

****************************************************************************************************
*/
void ioc_run_brick_transfer(
    iocBrickBuffer *b)
{
    iocStreamerState cmd;
    os_char state_bits;
    // osalStatus s = OSAL_NOTHING_TO_DO;

    if (b->stream == OS_NULL) {
        cmd = (iocStreamerState)ioc_gets_int(b->signals->cmd, &state_bits, IOC_SIGNAL_DEFAULT);
        if (cmd != IOC_STREAM_RUNNING || (state_bits & OSAL_STATE_CONNECTED) == 0) return;

        osal_trace("BRICK: IOC_STREAM_RUNNING command");
        b->stream = ioc_streamer_open(OS_NULL, &b->prm, OS_NULL, OSAL_STREAM_WRITE);
        if (b->stream == OS_NULL) return;
    }

    /* If we got data, then try to sending it.
     */
    if (b->pos < b->buf_n) {
        ioc_send_brick_data(b);
    }
}

