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

struct iocBrickBuffer;

typedef enum iocBrickFormat
{
    IOC_BYTE_BRICK = 50,           /* 8 bits per pixel, one channel */
    IOC_RGB24_BRICK = 51,          /* RGB 24 color image */ 

    IOC_MIN_BRICK_FORMAT = 50,
    IOC_MAX_BRICK_FORMAT = 51
}
iocBrickFormat;

typedef enum iocBrickCompression
{
    IOC_UNCOMPRESSED_BRICK = 1,     /* Uncompresssed brick */

    IOC_MIN_BRICK_COMPRESSION = 1,
    IOC_MAX_BRICK_COMPRESSION = 1
}
iocBrickCompression;


#if IOC_STREAMER_SUPPORT

#define IOC_MAX_BRICK_WIDTH 50000
#define IOC_MAX_BRICK_HEIGHT 3000
#define IOC_MAX_BRICK_ALLOC (IOC_MAX_BRICK_WIDTH * IOC_MAX_BRICK_HEIGHT * 3)


/* Flags for ioc_initialize_brick_buffer(), bit fields. The same numeric values
   as IOC_IS_CONTROLLER and IOC_IS_DEVICE in ioc_dyn_stream.h.
 */
#define IOC_BRICK_CONTROLLER 2
#define IOC_BRICK_DEVICE 4


/** Camera image as received by camera callback function.
    This must be flat.
    format Stream data format
 */
#define IOC_BRICK_TSTAMP_SZ 8
#define IOC_BRICK_CHECKSUM_SZ 2
#define IOC_BRICK_DIM_SZ 2
#define IOC_BRICK_BYTES_SZ 4

typedef struct iocBrickHdr
{
    os_uchar format;
    os_uchar compression;
    os_uchar checksum[IOC_BRICK_CHECKSUM_SZ];
    os_uchar width[IOC_BRICK_DIM_SZ];    /* Width in pixels, etc. */
    os_uchar height[IOC_BRICK_DIM_SZ];   /* Height in pixels, etc. */
    os_uchar buf_sz[IOC_BRICK_BYTES_SZ];   /* Number of actual bytes including this header for reading right number of bytes in frame */
    os_uchar alloc_sz[IOC_BRICK_BYTES_SZ]; /* Maximum bytes needed including this header for memory allocation by receiver */
    os_uchar tstamp[IOC_BRICK_TSTAMP_SZ];
}
iocBrickHdr;

/* Brick received callback function.
 */
typedef osalStatus ioc_brick_received(
    struct iocBrickBuffer *b,
    void *context);

typedef struct iocBrickBuffer
{
    iocRoot *root;
    os_uchar *buf;
    os_memsz buf_sz;
    os_memsz buf_alloc_sz;
    volatile os_memsz buf_n;
    volatile os_memsz pos;

    iocStreamerParams prm;
    iocStreamerSignals *signals;
    osalStream stream;
    os_int timeout_ms; /* timeout for streamer continuous data transfer, -1 = no timeout */
    os_boolean state_initialized;

    /* error retry timer. */
    os_boolean err_timer_set;
    os_timer err_timer;

    /* callback */
    volatile os_boolean enable_receive;
    ioc_brick_received *receive_callback;
    void *receive_context;
}
iocBrickBuffer;

/* Initialize brick buffer (does not allocate any memory yet)
 */
void ioc_initialize_brick_buffer(
    iocBrickBuffer *b,
    const iocStreamerSignals *signals,
    iocRoot *root,
    os_int timeout_ms,
    os_int flags);

/* Release brick buffer which has been initialized by ioc_initialize_brick_buffer
 */
void ioc_release_brick_buffer(
    iocBrickBuffer *b);

/* Set function to call when brick is received.
 */
void ioc_set_brick_received_callback(
    iocBrickBuffer *b,
    ioc_brick_received *func,
    void *context);

/* Allocate brick buffer's internal data buffer.
 */
osalStatus ioc_allocate_brick_buffer(
    iocBrickBuffer *b,
    os_memsz buf_sz);

/* Release brick buffer's internal data buffer.
 */
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

/* Run brick data transfer
 */
void ioc_run_brick_send(
    iocBrickBuffer *b);

/* Turn receiving on or off
 */
void ioc_brick_set_receive(
    iocBrickBuffer *b,
    os_boolean enable);

/* Run brick data transfer
 */
osalStatus ioc_run_brick_receive(
    iocBrickBuffer *b);

os_ulong ioc_brick_int(
    os_uchar *data,
    os_int nro_bytes);

#define ioc_is_brick_empty(b) ((b)->buf_n == 0)
#define ioc_is_brick_connected(b) ((b)->stream != OS_NULL)
#define ioc_is_brick_connected(b) ((b)->stream != OS_NULL)

#endif
