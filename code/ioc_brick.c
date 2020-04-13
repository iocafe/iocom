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
    iocRoot *root)
{
    os_memclear(b, sizeof(iocBrickBuffer));
    b->root = root;
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
    iocBrickBuffer *b,
    struct iocOutputStream *output_stream)
{
    os_memsz n;

    ioc_lock(b->root);

    if (b->pos < b->buf_n)
    {
        n = b->buf_n - b->pos;
        b->pos += ioc_write_item_to_output_stream(output_stream, (const os_char*)b->buf + b->pos, n);
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

  This is IO device end function, which handles transfer of configuration and flash software,
  etc. The device configuration included device identification, network configuration and
  security configuration, like certificates, etc.

  The streamer is used  to transfer a stream using buffer within memory block. This static
  params structure selects which signals are used for straming.

  The function is called repeatedly to run data this data transfer between the  controller and
  the IO device. The function reads data from the stream buffer in memory block (as much as
  there is) and writes it to persistent storage.

  If the function detects IOC_STREAM_COMPLETED or IOC_STREAM_INTERRUPTED command, or if
  connection has broken, it closes the persistent storage and memory block streamer.
  Closing persistent object is flagged with success OSAL_STREAM_DEFAULT only on
  IOC_STREAM_COMPLETED command, otherwise persistent object is closed with OSAL_STREAM_INTERRUPT
  flag (in this case persient object may not want to use newly received data, especially if
  it is flash program for micro-controller.

  This function must be called from one thread at a time.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.

  @return  If working in something, the function returns OSAL_SUCCESS. Return value
           OSAL_NOTHING_TO_DO indicates that this thread can be switched to slow
           idle mode as far as the control stream knows.

****************************************************************************************************
*/
void ioc_run_brick_transfer(
    iocBrickBuffer *b,
    struct iocOutputStream *output_stream)
{
    iocStreamerState cmd;
    osPersistentBlockNr select;
    os_char state_bits;
    osalStatus s = OSAL_NOTHING_TO_DO;

    /* No status yet
     */
    if (no oputout stream open)
    {
        try open output stream
        return
    }

    /* If we got data check try to send it.
     */
    if (bitmap_buffer.buf_n)
    {
        ioc_send_brick_data(&bitmap_buffer, &video_output);
    }

    /* Check if other end wants to close the stream
     */
    if (?) ??
}
