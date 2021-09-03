/**

  @file    ioc_brick.h
  @brief   Structres and functions related to brick transfer.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_BRICK_H_
#define IOC_BRICK_H_
#include "iocom.h"

struct iocBrickBuffer;


/* Do not change enumeration values, breaks compatibility. Future compressions should be marked
   with nonzero value 1 - 126 (highest bit zero, nonzero value). JPEG quality 0 means that
   quality is not set.
 */
#define IOC_UNCOMPRESSED 0
#define IOC_JPEG 0x80
#define IOC_JPEG_QUALITY_MASK 0x7F
#define IOC_DEFAULT_COMPRESSION 0x7F

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


/**
****************************************************************************************************

  @brief Brick header structure.

    - format Basic image format, See enumeration osalBitmapFormat.
    - compression: Compression as bit fields: IOC_UNCOMPRESSED, IOC_NORMAL_JPEG, etc.
    - checksum: Modbus checksum calculated over whole buffer including header
      and data (either compressed or uncompressed). Calculating the checksum
      should be last modification to a brick to send. I "locks" the brick.
      Two checksum bytes in brick are set to zero before calculating the checksum
      and set to real values after it.
    - width: WidthW idth in pixels, etc.
    - height: Height in pixels, etc.
    - buf_sz: Number of actual bytes in this brick, including both header and data.
      If image is uncomressed, this may be same as alloc_sz (is same unless we have varying
      image width/height). If image is compressed this is smaller than alloc size.
    - alloc_sz: Uncompresses format size: size of header + size of uncompressed image
      data. Used by received to allocate space for uncompressed buffer. This may be
      larger than minimum number of bytes needed, if image size varies.
    - tstamp: Time stamp, microseconds since boot.

****************************************************************************************************
*/
typedef struct iocBrickHdr
{
    os_uchar format;
    os_uchar compression;
    os_uchar checksum[IOC_BRICK_CHECKSUM_SZ];
    os_uchar width[IOC_BRICK_DIM_SZ];
    os_uchar height[IOC_BRICK_DIM_SZ];
    os_uchar buf_sz[IOC_BRICK_BYTES_SZ];
    os_uchar alloc_sz[IOC_BRICK_BYTES_SZ];
    os_uchar tstamp[IOC_BRICK_TSTAMP_SZ];
}
iocBrickHdr;

/* Brick received callback function type.
 */
typedef osalStatus ioc_brick_received(
    struct iocBrickBuffer *b,
    void *context);

/* Brick buffer used to send and receive large or complex data as "bricks".
 */
typedef struct iocBrickBuffer
{
    /* Data buffering. Different use for ring buffer and flat buffer transfers
      and receivind and sending end.
     */
    iocRoot *root;
    os_uchar *buf;
    os_memsz buf_sz;
    os_memsz buf_alloc_sz;
    volatile os_memsz buf_n;
    volatile os_memsz pos;

    /* Sturctoru containing used IO signal pointers.
     */
    iocStreamerSignals *signals;

    /* Stream,. OS_NULL for flat buffer trasfber.
     */
    osalStream stream;
    iocStreamerParams prm;

    /* Rimeout for streamer continuous data transfer, -1 = no timeout
     */
    os_int timeout_ms;

    /* Previous value to detecta change (edge)
     */
    os_int prev_cmd;
    os_int prev_state;

    os_timer err_timer;
    os_boolean err_timer_set;

    /* Flat buffer.
     */
    os_timer flat_frame_timer;
    os_ushort flat_frame_count;
    os_boolean flat_ready_for_brick;
    os_boolean flat_connected;

    /* Current JPEG compression quality, scale 0 - 100 (small and low quality - big and precise).
     */
    os_double compression_quality;

    /* Callback.
     */
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

os_boolean ioc_ready_for_new_brick(
    iocBrickBuffer *b);

os_boolean ioc_is_brick_connected(
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
osalStatus ioc_compress_brick(
    iocBrickBuffer *b,
    iocBrickHdr *hdr,
    os_uchar *data,
    os_memsz data_sz,
    osalBitmapFormat format,
    os_int w,
    os_int h,
    os_uchar compression);

/* Store time stamp into the brick header (must be called before setting check sum)
 */
void ioc_set_brick_timestamp(
    iocBrickHdr *hdr);

/* Run brick data transfer
 */
osalStatus ioc_run_brick_send(
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

/* Get integer value from brick header (take care of endianess)
 */
os_ulong ioc_get_brick_hdr_int(
    os_uchar *data,
    os_int nro_bytes);

/* Adjust compression quality used to send data to brick buffer.
 */
void ioc_adjust_jpeg_compression_quality(
    iocBrickBuffer *b,
    osalBitmapFormat format,
    os_int w,
    os_int h,
    os_int compression_quality,
    osalStatus compression_status,
    os_memsz compressed_sz);

/* Get compression quality to use for this brick.
 */
#define ioc_get_jpeg_compression_quality(b) ((os_int)((b)->compression_quality))

#endif
#endif
