/**

  @file    common/ioc_brick.h
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

typedef enum iocBrickFormat
{
    IOC_BYTE_BRICK = 50           /* 8 bits per pixel, one channel */
}
iocBrickFormat;

typedef enum iocBrickCompression
{
    IOC_UNCOMPRESSED_BRICK = 0     /* Uncompresssed brick */
}
iocBrickCompression;


/** Camera image as received by camera callback function.
    This must be flat.
    format Stream data format
 */
#define IOC_BITMAP_TSTAMP_SZ 8
typedef struct iocBrickHdr
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
iocBrickHdr;


typedef struct iocBrickBuffer
{
    iocRoot *root;
    os_uchar *buf;
    os_memsz buf_sz;
    volatile os_memsz buf_n;
    volatile os_memsz pos;

    iocStreamerParams prm;
    iocStreamerSignals *signals;
    osalStream stream;
}
iocBrickBuffer;

/* Initialize brick buffer (does not allocate any memory yet)
 */
void ioc_initialize_brick_buffer(
    iocBrickBuffer *b,
    const iocStreamerSignals *signals,
    iocRoot *root);

void ioc_allocate_brick_buffer(
    iocBrickBuffer *b,
    os_memsz buf_sz);

void ioc_free_brick_buffer(
    iocBrickBuffer *b);

/* Compress brick into buffer.
 */
os_memsz ioc_compress_brick(
    os_uchar *buf,
    os_memsz buf_sz,
    os_uchar *src,
    iocBrickFormat src_format,
    os_int src_w,
    os_int src_h,
    iocBrickCompression compression);

/* Store time stamp into the brick header (must be called before ioc_set_brick_checksum)
 */
void ioc_set_brick_timestamp(
    os_uchar *buf);

/* Store check sum within brick header
 */
void ioc_set_brick_checksum(
    os_uchar *buf,
    os_memsz buf_n);

/* Send all or part of brick data to output stream.
 */
// void ioc_send_brick_data(
//     iocBrickBuffer *b);

void ioc_run_brick_transfer(
    iocBrickBuffer *b);


#define ioc_is_brick_empty(b) ((b)->buf_n == 0)
#define ioc_is_brick_connected(b) ((b)->stream != OS_NULL)

