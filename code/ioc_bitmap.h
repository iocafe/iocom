/**

  @file    common/ioc_bitmap.h
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

struct iocOutputStream;

typedef enum iocBitmapFormat
{
    IOC_8_BIT_BITMAP = 50           /* 8 bits per pixel, one channel */
}
iocBitmapFormat;

typedef enum iocBitmapCompression
{
    IOC_UNCOMPRESSED_BITMAP = 0     /* Uncompresssed bitmap */
}
iocBitmapCompression;


/** Camera image as received by camera callback function.
    This must be flat.
    format Stream data format
 */
#define IOC_BITMAP_TSTAMP_SZ 8
typedef struct iocBitmapHdr
{
    os_uchar format;
    os_uchar compression;
    os_uchar checksum_low;
    os_uchar checksum_high;
    os_uchar width_low;
    os_uchar width_high;
    os_uchar height_low;
    os_uchar height_high;
    os_uchar tstamp[IOC_BITMAP_TSTAMP_SZ];
}
iocBitmapHdr;


typedef struct iocBitmapBuffer
{
    os_uchar *buf;
    os_memsz buf_sz;
    os_memsz buf_n;
    os_memsz pos;
}
iocBitmapBuffer;

/* Initialize bitmap buffer (does not allocate any memory yet)
 */
void ioc_initialize_bitmap_buffer(
    iocBitmapBuffer *b);

void ioc_allocate_bitmap_buffer(
    iocBitmapBuffer *b,
    os_memsz buf_sz);

void ioc_free_bitmap_buffer(
    iocBitmapBuffer *b);

/* Compress bitmap into buffer.
 */
os_memsz ioc_compress_bitmap(
    os_uchar *buf,
    os_memsz buf_sz,
    os_uchar *src,
    iocBitmapFormat src_format,
    os_int src_w,
    os_int src_h,
    iocBitmapCompression compression);

/* Store time stamp into the bitmap header (must be called before ioc_set_bitmap_checksum)
 */
void ioc_set_bitmap_timestamp(
    os_uchar *buf);

/* Store check sum within bitmap header
 */
void ioc_set_bitmap_checksum(
    os_uchar *buf,
    os_memsz buf_n);

/* Send all or part of bitmap data to output stream.
 */
void ioc_send_bitmap_data(
    iocBitmapBuffer *b,
    struct iocOutputStream *video_output);
