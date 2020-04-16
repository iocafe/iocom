/**

  @file    common/ioc_brick.c
  @brief   Structres and functions related to brick transfer.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.4.2020

  A brick is name for a block of data, like a video frame, to be streamed as one piece over
  ring buffer.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/* Initialize brick buffer (does not allocate any memory yet)

    Initializing brick buffer from code
        iocStreamerSignals vsignals;
        os_memclear(&vsignals, sizeof(iocStreamerSignals));
        vsignals.cmd = &gina.imp.rec_cmd;
        vsignals.select = &gina.imp.rec_select;
        vsignals.buf = &gina.exp.rec_buf;
        vsignals.head = &gina.exp.rec_head;
        vsignals.tail = &gina.imp.rec_tail;
        vsignals.state = &gina.exp.rec_state;
        vsignals.to_device = OS_FALSE;
        ioc_initialize_brick_buffer(&video_output, &vsignals, &ioboard_root, 0, IOC_BRICK_DEVICE);

    Initializing brick buffer from assembly defined in signals.json
        ioc_initialize_brick_buffer(&video_output, &gina.ccd, &ioboard_root, 0, IOC_BRICK_DEVICE);


   @param timeout_ms -1 = Infinite, 0 = use default, other values are timeout in ms
 */
void ioc_initialize_brick_buffer(
    iocBrickBuffer *b,
    const iocStreamerSignals *signals,
    iocRoot *root,
    os_int timeout_ms,
    os_int flags)
{
    os_memclear(b, sizeof(iocBrickBuffer));
    b->root = root;
    b->timeout_ms = timeout_ms;

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
    b->prm.is_device = (os_boolean)((flags & IOC_BRICK_CONTROLLER) == 0);
}


/* Set function to call when brick is received.
 */
void ioc_set_brick_received_callback(
    iocBrickBuffer *b,
    ioc_brick_received *func,
    void *context)
{
    b->receive_context = context;
    b->receive_callback = func;
}


/* Allocate buffer
 */
osalStatus ioc_allocate_brick_buffer(
    iocBrickBuffer *b,
    os_memsz buf_sz)
{
    ioc_lock(b->root);
    b->buf_n = 0;
    b->pos = 0;
    if (b->buf_sz != buf_sz)
    {
        ioc_free_brick_buffer(b);
        b->buf = (os_uchar*)os_malloc(buf_sz, &b->buf_alloc_sz);
        if (b->buf == OS_NULL) return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        os_memclear(b->buf, b->buf_alloc_sz);
        b->buf_sz = buf_sz;
    }
    ioc_unlock(b->root);
    return OSAL_SUCCESS;
}

void ioc_free_brick_buffer(
    iocBrickBuffer *b)
{
    ioc_lock(b->root);
    if (b->buf)
    {
        os_free(b->buf, b->buf_alloc_sz);
        b->buf = OS_NULL;
        b->buf_sz = 0;
        b->buf_alloc_sz = 0;
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
    os_memcpy(hdr->tstamp, &ti, IOC_BRICK_TSTAMP_SZ);
#else
    os_uchar *ss, *dd;
    int count;

    count = IOC_BRICK_TSTAMP_SZ;
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
    hdr->checksum[0] = 0;
    hdr->checksum[1] = 0;
    checksum = os_checksum((const os_char*)buf, buf_n, OS_NULL);
    hdr->checksum[0] = (os_uchar)checksum;
    hdr->checksum[1] = (os_uchar)(checksum >> 8);
}


/* Send all or part of brick data to output stream.
 */
static osalStatus ioc_send_brick_data(
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
            return s;
        }

        b->pos += n_written;
    }

    /* If whole brick has been sent, mark buffer empty.
     */
    if (b->pos >= b->buf_n) {
        b->buf_n = 0;
    }

    ioc_unlock(b->root);
    return OSAL_SUCCESS;
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
void ioc_run_brick_send(
    iocBrickBuffer *b)
{
    iocStreamerState cmd;
    os_char state_bits;

    if (b->stream == OS_NULL) {
        if (b->prm.frd.state && !b->state_initialized)
        {
            ioc_sets0_int(b->prm.frd.state, 0);
            b->state_initialized = OS_TRUE;
        }

        cmd = (iocStreamerState)ioc_gets_int(b->signals->cmd, &state_bits, IOC_SIGNAL_DEFAULT);
        if (cmd != IOC_STREAM_RUNNING || (state_bits & OSAL_STATE_CONNECTED) == 0) return;

        osal_trace("BRICK: IOC_STREAM_RUNNING command");
        b->stream = ioc_streamer_open(OS_NULL, &b->prm, OS_NULL, OSAL_STREAM_WRITE);
        if (b->stream == OS_NULL) return;

        if (b->timeout_ms) {
            osal_stream_set_parameter(b->stream, OSAL_STREAM_WRITE_TIMEOUT_MS, b->timeout_ms);
        }
    }

    /* If we got data, then try to sending it.
     */
    if (b->pos < b->buf_n) {
        if (ioc_send_brick_data(b))
        {
            ioc_streamer_close(b->stream, OSAL_STREAM_DEFAULT);
            b->stream = OS_NULL;
        }
    }
}


void ioc_brick_set_receive(
    iocBrickBuffer *b,
    os_boolean enable)
{
    b->enable_receive = enable;
}


os_ulong ioc_brick_int(
    os_uchar *data,
    os_int nro_bytes)
{
    os_ulong x = 0;
    while (nro_bytes--) {
        x <<= 8;
        x |= data[nro_bytes];
    }
    return x;
}

/* osal_validate_brick_header Send all or part of brick data to output stream.
 */
static osalStatus osal_validate_brick_header(
    iocBrickHdr *bhdr)
{
    os_uint w, h, buf_sz, alloc_sz, bytes_per_pix, max_brick_sz, max_brick_alloc;

    if (bhdr->format < IOC_MIN_BRICK_FORMAT ||
        bhdr->format > IOC_MAX_BRICK_FORMAT ||
        bhdr->compression < IOC_MIN_BRICK_COMPRESSION ||
        bhdr->compression > IOC_MAX_BRICK_COMPRESSION)
    {
        return OSAL_STATUS_FAILED;
    }

    w = ioc_brick_int(bhdr->width, IOC_BRICK_DIM_SZ);
    h = ioc_brick_int(bhdr->height, IOC_BRICK_DIM_SZ);
    if (w < 1 || w > IOC_MAX_BRICK_WIDTH ||
        h < 1 || h > IOC_MAX_BRICK_HEIGHT)
    {
        return OSAL_STATUS_FAILED;
    }

    bytes_per_pix = 1;
    max_brick_sz = w * h * bytes_per_pix + sizeof(iocBrickHdr);
    max_brick_alloc = 3*((IOC_MAX_BRICK_WIDTH * IOC_MAX_BRICK_HEIGHT * bytes_per_pix)/2) + sizeof(iocBrickHdr);

    buf_sz = ioc_brick_int(bhdr->buf_sz, IOC_BRICK_BYTES_SZ);
    alloc_sz = ioc_brick_int(bhdr->alloc_sz, IOC_BRICK_BYTES_SZ);
    if (buf_sz < 1 || buf_sz > max_brick_sz ||
        alloc_sz < 1 || alloc_sz > max_brick_alloc)
    {
        return OSAL_STATUS_FAILED;
    }
    return OSAL_SUCCESS;
}


/* Send all or part of brick data to output stream.
 */
static osalStatus ioc_receive_brick_data(
    iocBrickBuffer *b)
{
    iocBrickHdr *bhdr;
    os_memsz n, n_read, alloc_sz;
    osalStatus s;
    os_uint checksum;

    union
    {
        os_uchar bytes[sizeof(iocBrickHdr)];
        iocBrickHdr hdr;
    }
    first;

    if (b->buf == OS_NULL)
    {
        n = sizeof(iocBrickHdr) - b->pos;
        s = ioc_streamer_read(b->stream, (os_char*)first.bytes + b->pos, n, &n_read, OSAL_STREAM_DEFAULT);
        if (s) {
            return s;
        }
        b->pos += n_read;
        if (b->pos < sizeof(iocBrickHdr)) {
            return OSAL_SUCCESS;
        }

        if (osal_validate_brick_header(&first.hdr)) {
            return OSAL_STATUS_FAILED;
        }

        alloc_sz = (os_memsz)ioc_brick_int(first.hdr.alloc_sz, IOC_BRICK_BYTES_SZ);
        b->buf = (os_uchar*)os_malloc(alloc_sz, &b->buf_alloc_sz);
        if (b->buf == OS_NULL) {
            return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        }
        os_memclear(b->buf, b->buf_alloc_sz);
        os_memcpy(b->buf, first.bytes, sizeof(iocBrickHdr));
    }

    bhdr = (iocBrickHdr*)b->buf;
    if (n_read + n < sizeof(iocBrickHdr))
    {
        n = sizeof(iocBrickHdr) - b->pos;
    }
    else
    {
        b->buf_sz = (os_memsz)ioc_brick_int(bhdr->buf_sz, IOC_BRICK_BYTES_SZ);
        n = b->buf_sz - b->pos;
    }

    s = ioc_streamer_read(b->stream, (os_char*)b->buf + b->pos, n, &n_read, OSAL_STREAM_DEFAULT);
    if (s) {
        return s;
    }
    b->pos += n_read;

    if (b->pos < sizeof(iocBrickHdr)) {
        return OSAL_SUCCESS;
    }

    if (b->pos == sizeof(iocBrickHdr))
    {
        if (osal_validate_brick_header(bhdr)) {
            return OSAL_STATUS_FAILED;
        }

        b->buf_sz = (os_memsz)ioc_brick_int(bhdr->buf_sz, IOC_BRICK_BYTES_SZ);
        if (b->buf_sz > b->buf_alloc_sz)
        {
            return OSAL_STATUS_FAILED;
        }

        n = b->buf_sz - b->pos;
        s = ioc_streamer_read(b->stream, (os_char*)b->buf + b->pos, n, &n_read, OSAL_STREAM_DEFAULT);
        if (s) {
            return s;
        }
        b->pos += n_read;
    }

    if (b->pos < b->buf_sz) {
        return OSAL_SUCCESS;
    }

    /* verify checksum */
    checksum = ioc_brick_int(bhdr->checksum, IOC_BRICK_CHECKSUM_SZ);
    os_memclear(bhdr->checksum, IOC_BRICK_CHECKSUM_SZ);
    if (os_checksum((const os_char*)b->buf, b->buf_sz, OS_NULL) != checksum)
    {
        return OSAL_STATUS_CHECKSUM_ERROR;
    }

    if (b->receive_callback)
    {
        b->receive_callback(b, b->receive_context);
    }

    b->pos = 0;
    return OSAL_SUCCESS;
}


/* Run brick data transfer
 */
void ioc_run_brick_receive(
    iocBrickBuffer *b)
{
    iocStreamerState state;
    os_char state_bits;

    if (!b->enable_receive)
    {
        if (b->stream != OS_NULL) {
            ioc_streamer_close(b->stream, OSAL_STREAM_DEFAULT);
            b->stream = OS_NULL;
        }
        return;
    }

    if (b->stream == OS_NULL) {
        if (b->prm.tod.state && !b->state_initialized)
        {
            ioc_sets0_int(b->prm.tod.state, 0);
            b->state_initialized = OS_TRUE;
        }

        if (b->prm.frd.state)
        {
            state = (iocStreamerState)ioc_gets_int(b->prm.frd.state, &state_bits, IOC_SIGNAL_DEFAULT);
            if (state != IOC_STREAM_IDLE || (state_bits & OSAL_STATE_CONNECTED) == 0) return;
        }

        b->stream = ioc_streamer_open(OS_NULL, &b->prm, OS_NULL, OSAL_STREAM_READ);
        if (b->stream == OS_NULL) return;
        if (b->timeout_ms) {
            osal_stream_set_parameter(b->stream, OSAL_STREAM_READ_TIMEOUT_MS, b->timeout_ms);
        }

        b->pos = 0;
    }

    if (ioc_receive_brick_data(b))
    {
        ioc_streamer_close(b->stream, OSAL_STREAM_DEFAULT);
        b->stream = OS_NULL;
    }
}
