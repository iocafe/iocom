/**

  @file    common/ioc_bitmap.c
  @brief   Structres and functions related to bitmap transfer.
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


/* Initialize bitmap buffer (does not allocate any memory yet)
 */
void ioc_initialize_bitmap_buffer(
    iocBitmapBuffer *b,
    iocRoot *root)
{
    os_memclear(b, sizeof(iocBitmapBuffer));
    b->root = root;
}

/* Allocate buffer
 */
void ioc_allocate_bitmap_buffer(
    iocBitmapBuffer *b,
    os_memsz buf_sz)
{
    ioc_lock(b->root);
    b->buf_n = 0;
    b->pos = 0;
    if (b->buf_sz != buf_sz)
    {
        ioc_free_bitmap_buffer(b);
        b->buf = (os_uchar*)os_malloc(buf_sz, OS_NULL);
        b->buf_sz = buf_sz;
    }
    ioc_unlock(b->root);
}

void ioc_free_bitmap_buffer(
    iocBitmapBuffer *b)
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


/* Compress bitmap into buffer.
 * @param buf Buffer where to store bitmap header and compressed data.
 * @param src Source data, bitmap header + uncompressed bitmap data.
 * @return number of final bytes in buf (includes bitmap header).
 */
os_memsz ioc_compress_bitmap(
    os_uchar *buf,
    os_memsz buf_sz,
    os_uchar *src,
    iocBitmapFormat src_format,
    os_int src_w,
    os_int src_h,
    iocBitmapCompression compression)
{
    iocBitmapHdr *hdr;

    /* Very dummy and limited uncompressed implementation.
     */
    os_memsz sz;
    sz = src_w * src_h + sizeof(iocBitmapHdr);
    if (sz > buf_sz) sz = buf_sz;
    os_memcpy(buf, src, sz);

    hdr = (iocBitmapHdr*)buf;
    hdr->compression = compression;

    return sz;
}


/* Store time stamp into the bitmap header (must be called before ioc_set_bitmap_checksum)
 */
void ioc_set_bitmap_timestamp(
    os_uchar *buf)
{
    iocBitmapHdr *hdr;
    os_timer ti;

    hdr = (iocBitmapHdr*)buf;
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


/* Store check sum within bitmap header
 */
void ioc_set_bitmap_checksum(
    os_uchar *buf,
    os_memsz buf_n)
{
    iocBitmapHdr *hdr;
    os_ushort checksum;

    hdr = (iocBitmapHdr*)buf;
    hdr->checksum_low = 0;
    hdr->checksum_high = 0;
    checksum = os_checksum((const os_char*)buf, buf_n, OS_NULL);
    hdr->checksum_low = (os_uchar)checksum;
    hdr->checksum_high = (os_uchar)(checksum >> 8);
}


/* Send all or part of bitmap data to output stream.
 */
void ioc_send_bitmap_data(
    iocBitmapBuffer *b,
    struct iocOutputStream *video_output)
{
    os_memsz n;

    ioc_lock(b->root);

    if (b->pos < b->buf_n)
    {
        n = b->buf_n - b->pos;
        b->pos += ioc_write_item_to_output_stream(video_output, (const os_char*)b->buf + b->pos, n);
    }

    /* If whole bitmap has been sent, mark buffer empty.
     */
    if (b->pos >= b->buf_n) {
        b->buf_n = 0;
    }

    ioc_unlock(b->root);
}
