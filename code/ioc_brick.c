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
#if IOC_STREAMER_SUPPORT

#if IOC_USE_JPEG_COMPRESSION
#include "eosal_jpeg.h"
#endif


/**
****************************************************************************************************

  @brief Initialize brick buffer (does not allocate any memory yet)
  @anchor ioc_initialize_brick_buffer

  Examples:

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

  @param   b Pointer to brick buffer structure to initialize.
  @param   signals Pointer to structure with pointers to every signal used for brick data
           transfer. The function copied data in the signals structure, so it can be allocated
           form stack. Actual individual structure for each signal must exist as long as
           the brick buffer is used.
  @param   timeout_ms -1 = Infinite, 0 = use default, other values are timeout in ms
  @return  None.

****************************************************************************************************
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

    /* In device end, we need to set IDLE status with connected state bit */
    if (b->prm.is_device && !signals->flat_buffer) {
        ioc_set(signals->state, IOC_STREAM_IDLE);
    }
}


/* Release brick buffer which has been initialized by ioc_initialize_brick_buffer
 */
void ioc_release_brick_buffer(
    iocBrickBuffer *b)
{
    ioc_lock(b->root);
    ioc_free_brick_buffer(b);

    if (b->stream) {
        ioc_streamer_close(b->stream, OSAL_STREAM_DEFAULT);
        b->stream = OS_NULL;
    }

    ioc_unlock(b->root);
}


/**
****************************************************************************************************

  @brief Check if we are can send new brick (previous has been processed)
  @anchor ioc_ready_for_new_brick

  @param   b Pointer to brick buffer
  @return  OS_TRUE if if we can send new brick, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean ioc_ready_for_new_brick(
    iocBrickBuffer *b)
{
#if IOC_BRICK_RING_BUFFER_SUPPORT
    if (!b->signals->flat_buffer) return b->buf_n == 0;
#endif
    return b->flat_ready_for_brick;
}


/**
****************************************************************************************************

  @brief Check if we are connected to someone who wants to received data.
  @anchor ioc_is_brick_connected

  @param   b Pointer to brick buffer
  @return  OS_TRUE if connected, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean ioc_is_brick_connected(
    iocBrickBuffer *b)
{
#if IOC_BRICK_RING_BUFFER_SUPPORT
    if (!b->signals->flat_buffer) return b->stream != OS_NULL;
#endif
    return b->flat_connected;
}


/**
****************************************************************************************************

  @brief Set function to call when brick is received.
  @anchor ioc_set_brick_received_callback

  Stores callback and context pointers within brick buffer.

  @param   b Pointer to brick buffer
  @param   func Pointer to function to call once whole brick has been received.
  @param   context Application specific pointer to pass to callback function.
  @return  None.

****************************************************************************************************
*/
void ioc_set_brick_received_callback(
    iocBrickBuffer *b,
    ioc_brick_received *func,
    void *context)
{
    b->receive_context = context;
    b->receive_callback = func;
}


/**
****************************************************************************************************

  @brief Allocate internal buffer for the brick buffer.
  @anchor ioc_allocate_brick_buffer

  This function should be called when brick buffer is used to send data. It allocates buffer
  large enough to hold any brick to send. Do not call if bit buffer is used to receive data.

  @param   b Pointer to brick buffer
  @param   buf_sz Maximum size of outgoing brick (including brick header).
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_allocate_brick_buffer(
    iocBrickBuffer *b,
    os_memsz buf_sz)
{

#if IOC_BRICK_RING_BUFFER_SUPPORT
    /* No temp buffer needed for flat buffer transfers
     */
    if (!b->signals->flat_buffer)
    {
        if (buf_sz <= sizeof(iocBrickHdr) || buf_sz > IOC_MAX_BRICK_ALLOC)
        {
            osal_debug_error("ioc_allocate_brick_buffer: Illegal size");
            return OSAL_STATUS_FAILED;
        }

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
    }
#endif

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Release brick buffer's internal buffer.
  @anchor ioc_free_brick_buffer

  @param   b Pointer to brick buffer
  @return  None.

****************************************************************************************************
*/
void ioc_free_brick_buffer(
    iocBrickBuffer *b)
{
#if IOC_BRICK_RING_BUFFER_SUPPORT
    if (!b->signals->flat_buffer)
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
#endif
}


/**
****************************************************************************************************

  @brief Store/compress data to send into brick buffer.
  @anchor ioc_compress_brick

  @param  b Pointer to brick buffer.
  @param  hdr Brick header to save.
  @param  data Uncompressed (or compressed in special cases) source data.
  @param  data_sz Data size in bytes, important if data is compressed JPEG.
  @param  format Source data format, set IOC_BYTE_BRICK, IOC_RGB24_BRICK, IOC_GRAYSCALE8_BRICK...
  @param  w Source image width in pixels, etc.
  @param  h Source image height in pixels, etc.
  @param  compression How to compress data, set IOC_UNCOMPRESSED_BRICK, IOC_SMALL_JPEG,
          IOC_NORMAL_JPEG, IOC_LARGE_JPEG...
  @return OSAL_SUCCESS (0) if all is fine. Other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_compress_brick(
    iocBrickBuffer *b,
    iocBrickHdr *hdr,
    os_uchar *data,
    os_memsz data_sz,
    osalBitmapFormat format,
    os_int w,
    os_int h,
    iocBrickCompression compression)
{
    iocBrickHdr *dhdr, dhdr_tmp;
    os_uchar *buf;
    os_memsz sz, buf_sz;
    os_ushort checksum = 0;
    os_boolean flat_buffer;
    osalStatus s = OSAL_SUCCESS;

#if IOC_USE_JPEG_COMPRESSION
    os_int quality;
#endif

#if IOC_BRICK_RING_BUFFER_SUPPORT
    flat_buffer = b->signals->flat_buffer;
    if (flat_buffer)
    {
        buf = OS_NULL;
        buf_sz = 0;
        dhdr = &dhdr_tmp;
        ioc_lock(b->root);
    }
    else
    {
        buf = b->buf;
        buf_sz = b->buf_sz;
        dhdr = (iocBrickHdr*)buf;
    }
#else
    flat_buffer = OS_TRUE;
    buf = OS_NULL;
    buf_sz = 0;
    dhdr = &dhdr_tmp;
    ioc_lock(b->root);
#endif
    os_memcpy(dhdr, hdr, sizeof(iocBrickHdr));

    if (compression == IOC_DEFAULT_CAM_IMG_COMPR)
    {
#if IOC_USE_JPEG_COMPRESSION
        compression = IOC_NORMAL_JPEG;
#else
        compression = hdr->compression;
#endif
    }

    /* Copy or compress.
     */
    switch (compression)
    {
#if IOC_USE_JPEG_COMPRESSION
        case IOC_SMALL_JPEG:
            quality = 15;
            goto compress_jpeg;

        case IOC_NORMAL_JPEG:
            quality = 40;
            goto compress_jpeg;

        case IOC_LARGE_JPEG:
            quality = 75;
compress_jpeg:
#else
        case IOC_SMALL_JPEG:
        case IOC_NORMAL_JPEG:
        case IOC_LARGE_JPEG:
#endif
            /* If already compressed by camera (ESP32 cam can make JPEG)
             */
            if (hdr->compression == IOC_NORMAL_JPEG)
            {
                if (data_sz + (os_memsz)sizeof(iocBrickHdr) > buf_sz) {
                    osal_debug_error("ioc_brick: buffer too small for JPEG");
                    data_sz = buf_sz - sizeof(iocBrickHdr);
                }

                os_memcpy(buf + sizeof(iocBrickHdr), data, data_sz);
                sz = data_sz;
                break;
            }

#if IOC_USE_JPEG_COMPRESSION
            /* Flat buffer
             */
            if (flat_buffer) {
                buf_sz = b->signals->buf->n - sizeof(iocBrickHdr);
                buf = (os_uchar*)os_malloc(buf_sz, OS_NULL);
                if (buf == OS_NULL)  {
                    s = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
                    goto getout;
                }

                s = os_compress_JPEG(data, w, h, format, quality,
                    OS_NULL, buf, buf_sz, &sz, OSAL_JPEG_DEFAULT);
                if (s)  {
                    if (quality > 10) 
                    {
                        osal_debug_error_int("Out of flat buffer, JPEG quality reduced to ", quality/2);
                        s = os_compress_JPEG(data, w, h, format, quality/2,
                            OS_NULL, buf, buf_sz, &sz, OSAL_JPEG_DEFAULT);
                    }
                    if (s)  {
                        s = OSAL_STATUS_OUT_OF_BUFFER;
                        goto getout;
                    }
                }
                dhdr->compression = IOC_NORMAL_JPEG;

                ioc_move_array(b->signals->buf, sizeof(iocBrickHdr), buf, (os_int)sz,
                    OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);

                checksum = os_checksum((os_char*)buf, sz, OS_NULL);
                break;
            }

            /* Ring buffer
             */
#if IOC_BRICK_RING_BUFFER_SUPPORT
            s = os_compress_JPEG(data, w, h, format, quality,
                OS_NULL, buf + sizeof(iocBrickHdr), buf_sz - sizeof(iocBrickHdr), &sz, OSAL_JPEG_DEFAULT);
            if (s == OSAL_SUCCESS)
            {
                /* Flag always as IOC_NORMAL_JPEG regardless */
                dhdr->compression = IOC_NORMAL_JPEG;
                break;
            }
#endif
            /* continues... */
#else
            osal_debug_error("JPEG is not included in build");
            /* continues... */
#endif

        default:
        case IOC_UNCOMPRESSED_BRICK:
            sz = w * (os_memsz)h * OSAL_BITMAP_BYTES_PER_PIX(format);
            osal_debug_assert(sz == data_sz);
            if (sz +  (os_memsz)sizeof(iocBrickHdr) > buf_sz) {
                sz = buf_sz - sizeof(iocBrickHdr);
                osal_debug_error("ioc_brick: buffer too small");
            }
            os_memcpy(buf + sizeof(iocBrickHdr), data, sz);
            /* set IOC_UNCOMPRESSED_BRICK as default */
            dhdr->compression = IOC_UNCOMPRESSED_BRICK;
            break;
    }

    dhdr->format = format;
    dhdr->width[0] = (os_uchar)w;
    dhdr->width[1] = (os_uchar)(w >> 8);
    dhdr->height[0] = (os_uchar)h;
    dhdr->height[1] = (os_uchar)(h >> 8);

    sz += sizeof(iocBrickHdr);
    dhdr->buf_sz[0] = (os_uchar)sz;
    dhdr->buf_sz[1] = (os_uchar)(sz >> 8);
    dhdr->buf_sz[2] = (os_uchar)(sz >> 16);
    dhdr->buf_sz[3] = (os_uchar)(sz >> 24);

    ioc_set_brick_timestamp(dhdr);
    dhdr->checksum[0] = 0;
    dhdr->checksum[1] = 0;
    os_checksum((const os_char*)dhdr, sizeof(iocBrickHdr), &checksum);
    ioc_set_brick_checksum(dhdr, checksum);
    b->buf_n = sz;

    if (flat_buffer) {
        ioc_move_array(b->signals->buf, 0, dhdr, sizeof(iocBrickHdr),
            OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);
        ioc_set(b->signals->head, sz);
        if (++(b->flat_frame_count) == 0) b->flat_frame_count++;
        ioc_set(b->signals->state, b->flat_frame_count);
    }

getout:
    if (flat_buffer) {
        os_free(buf, buf_sz);
        ioc_unlock(b->root);
    }

    return s;
}


/**
****************************************************************************************************

  @brief Set timestamp (timer) in brick header.
  @anchor ioc_set_brick_timestamp

  @param   hdr Pointer to brick header (in buffer).
  @return  None.

****************************************************************************************************
*/
void ioc_set_brick_timestamp(
    iocBrickHdr *hdr)
{
    os_timer ti;
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


/**
****************************************************************************************************

  @brief Store check sum within brick header
  @anchor ioc_set_brick_checksum

  @param   hdr Pointer to brick header (in buffer).
  @param   checksum Value to set.
  @return  None.

****************************************************************************************************
*/
void ioc_set_brick_checksum(
    iocBrickHdr *hdr,
    os_ushort checksum)
{
    hdr->checksum[0] = (os_uchar)checksum;
    hdr->checksum[1] = (os_uchar)(checksum >> 8);
}


#if IOC_BRICK_RING_BUFFER_SUPPORT
/**
****************************************************************************************************

  @brief Send all or part of brick data to output stream (internal).
  @anchor ioc_send_brick_data

  Helper function for ioc_run_brick_send().

  @param   bhdr Pointer to the brick header
  @return  OSAL_SUCCESS if all is fine. Other values indicate an error.

****************************************************************************************************
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
#endif


/**
****************************************************************************************************

  @brief Keep on sending data from brick buffer.
  @anchor ioc_run_brick_send

  This function can be called repeatedly from loop.

  @param   b Pointer to brick buffer
  @return  OSAL_NOTHING_TO_DO indicates that nothing was done, no need to hurry to call again.
           OSAL_SUCCESS indicates that work was done.
           Other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_run_brick_send(
    iocBrickBuffer *b)
{
    os_int cmd, prev_cmd, state;
    os_char state_bits;

    cmd = (os_int)ioc_get_ext(b->signals->cmd, &state_bits, IOC_SIGNAL_DEFAULT);
    prev_cmd = b->prev_cmd;
    b->prev_cmd = cmd;

#if IOC_BRICK_RING_BUFFER_SUPPORT
    if (!b->signals->flat_buffer)
    {
        osalStream stream;
        os_memsz n_written;
        osalStatus s;

        if (b->stream == OS_NULL) {
            if (cmd != IOC_STREAM_RUNNING || cmd == prev_cmd || (state_bits & OSAL_STATE_CONNECTED) == 0) {
                return OSAL_NOTHING_TO_DO;
            }

            /* Order here is important.
             */
            stream = ioc_streamer_open(OS_NULL, &b->prm, OS_NULL, OSAL_STREAM_WRITE);
            if (stream == OS_NULL) return OSAL_NOTHING_TO_DO;
            b->buf_n = 0;
            b->stream = stream;

            if (b->timeout_ms) {
                osal_stream_set_parameter(b->stream, OSAL_STREAM_WRITE_TIMEOUT_MS, b->timeout_ms);
            }
        }

        /* If we got data, then try to sending it. Even without data, keep streamer alive.
         */
        if (b->pos < b->buf_n) {
            s = ioc_send_brick_data(b);
        }
        else {
            s = ioc_streamer_write(b->stream, osal_str_empty, 0, &n_written, OSAL_STREAM_DEFAULT);
            if (!OSAL_IS_ERROR(s)) s = OSAL_NOTHING_TO_DO;
        }

        if (OSAL_IS_ERROR(s)) {
            ioc_streamer_close(b->stream, OSAL_STREAM_DEFAULT);
            b->stream = OS_NULL;
        }
        return s;
    }
#endif
    if ((state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        b->flat_ready_for_brick = OS_FALSE;
        b->flat_connected = OS_FALSE;
    }
    else
    {
        if (cmd && cmd != prev_cmd)
        {
            os_get_timer(&b->flat_frame_timer);
            b->flat_ready_for_brick = OS_TRUE;
            b->flat_connected = OS_TRUE;
        }
        else
        {
            if (b->flat_connected) {
                if (os_has_elapsed(&b->flat_frame_timer, 10000))
                {
                    b->flat_connected = OS_FALSE;
                }
            }
        }
    }

    /* Set connected bit. We should write function just to set connected bit. This will also rewrite value, which may cause sync problems */
    state = (os_int)ioc_get_ext(b->signals->state, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0) {
        ioc_set(b->signals->state, state);
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Enable or disable receiving data to brick buffer.
  @anchor ioc_brick_set_receive

  @param   b Pointer to brick buffer
  @param   enable OS_TRUE to enable reciving, OS_FALSE to disable it.
  @return  None.

****************************************************************************************************
*/
void ioc_brick_set_receive(
    iocBrickBuffer *b,
    os_boolean enable)
{
    b->enable_receive = enable;
}


/**
****************************************************************************************************

  @brief Get integer from brick header.
  @anchor ioc_brick_int

  Get integer value from brick header. Integers byte order in brick header are always least
  significant first, and they are not necessarily aligned by type size. This function gets
  integer right way, regardless of processor architechture.

  @param   data Pointer to first byte of integer in brick header
  @param   nro_bytes Number of bytes reserved for this integer.
  @return  Integer value.

****************************************************************************************************
*/
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


/**
****************************************************************************************************

  @brief Check that bhdr is legimate brick header.
  @anchor osal_validate_brick_header

  Check that brick is valid. This is used enforce interoperbility of different implementations,
  so that bugs are detected and fixed.

  @param   bhdr Pointer to the brick header
  @return  OSAL_SUCCESS if brick header is ok and valid. Other values indicate corrupted or
           unknown brick header.

****************************************************************************************************
*/
static osalStatus osal_validate_brick_header(
    iocBrickHdr *bhdr)
{
    os_uint w, h, buf_sz, alloc_sz, bytes_per_pix, max_brick_sz, max_brick_alloc;
    os_int i;
    const os_uchar format_list[] = {OSAL_GRAYSCALE8, OSAL_GRAYSCALE16, OSAL_RGB24, OSAL_RGBA32, 0};

    i = 0;
    while (bhdr->format != format_list[i])
    {
        if (format_list[++i] == 0) {
            return OSAL_STATUS_FAILED;
        }
    }

    if (bhdr->compression < IOC_MIN_BRICK_COMPRESSION ||
        bhdr->compression > IOC_MAX_BRICK_COMPRESSION)
    {
        return OSAL_STATUS_FAILED;
    }

    w = (os_uint)ioc_brick_int(bhdr->width, IOC_BRICK_DIM_SZ);
    h = (os_uint)ioc_brick_int(bhdr->height, IOC_BRICK_DIM_SZ);
    if (w < 1 || w > IOC_MAX_BRICK_WIDTH ||
        h < 1 || h > IOC_MAX_BRICK_HEIGHT)
    {
        return OSAL_STATUS_FAILED;
    }

    bytes_per_pix = OSAL_BITMAP_BYTES_PER_PIX(bhdr->format);
    max_brick_sz = w * h * bytes_per_pix + sizeof(iocBrickHdr);
    max_brick_alloc = 3*((IOC_MAX_BRICK_WIDTH * IOC_MAX_BRICK_HEIGHT
        * bytes_per_pix)/2) + sizeof(iocBrickHdr);

    buf_sz = (os_uint)ioc_brick_int(bhdr->buf_sz, IOC_BRICK_BYTES_SZ);
    alloc_sz = (os_uint)ioc_brick_int(bhdr->alloc_sz, IOC_BRICK_BYTES_SZ);
    if (buf_sz < 1 || buf_sz > max_brick_sz ||
        alloc_sz < 1 || alloc_sz > max_brick_alloc)
    {
        return OSAL_STATUS_FAILED;
    }
    return OSAL_SUCCESS;
}


#if IOC_BRICK_RING_BUFFER_SUPPORT
/**
****************************************************************************************************

  @brief Receive data into brick buffer. (internal).
  @anchor ioc_receive_brick_data

  Helper function for ioc_run_brick_receive().

  @param   b Pointer to brick buffer structure.
  @return  OSAL_SUCCESS all fine, but no complete brick received. OSAL_COMPLETED new brick
           received. Other values indicate an error.

****************************************************************************************************
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

    if (b->pos < sizeof(iocBrickHdr))
    {
        s = ioc_streamer_read(b->stream, (os_char*)first.bytes,
            sizeof(iocBrickHdr), &n_read, OSAL_STREAM_PEEK);
        if (s) {
            return s;
        }

        if (n_read < sizeof(iocBrickHdr)) {
            return OSAL_SUCCESS;
        }

        if (osal_validate_brick_header(&first.hdr)) {
            return OSAL_STATUS_FAILED;
        }

        b->buf_sz = (os_memsz)ioc_brick_int(first.hdr.buf_sz, IOC_BRICK_BYTES_SZ);

        alloc_sz = b->buf_sz | 0x0FFF;;
        if (b->buf == OS_NULL || alloc_sz > b->buf_alloc_sz)
        {
            if (b->buf) {
                os_free(b->buf, b->buf_alloc_sz);
            }
            b->buf = (os_uchar*)os_malloc(alloc_sz, &b->buf_alloc_sz);
            if (b->buf == OS_NULL) {
                return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
            }
            os_memclear(b->buf, b->buf_alloc_sz);
        }
    }

    n = b->buf_sz - b->pos;
    s = ioc_streamer_read(b->stream, (os_char*)b->buf + b->pos, n, &n_read, OSAL_STREAM_DEFAULT);
    if (s) {
        return s;
    }
    b->pos += n_read;

    if (b->pos < b->buf_sz) {
        return OSAL_SUCCESS;
    }

    /* Verify the checksum.
     */
    bhdr = (iocBrickHdr*)b->buf;
    checksum = (os_uint)ioc_brick_int(bhdr->checksum, IOC_BRICK_CHECKSUM_SZ);
    os_memclear(bhdr->checksum, IOC_BRICK_CHECKSUM_SZ);
    if (os_checksum((const os_char*)b->buf, b->buf_sz, OS_NULL) != checksum)
    {
        osal_debug_error("brick checksum error");
        return OSAL_STATUS_CHECKSUM_ERROR;
    }

    /* Callback function.
     */
    if (b->receive_callback)
    {
        s = b->receive_callback(b, b->receive_context);
        if (OSAL_IS_ERROR(s)) return s;
    }

    b->pos = 0;
    return OSAL_COMPLETED;
}
#endif


/**
****************************************************************************************************

  @brief Process received data in flat brick buffer. (internal).
  @anchor ioc_process_flat_brick_data

  Helper function for ioc_run_brick_receive().

  @param   b Pointer to brick buffer structure.
  @return  None.

****************************************************************************************************
*/
static void ioc_process_flat_brick_data(
    iocBrickBuffer *b)
{
    iocBrickHdr hdr, *dhdr;
    os_int n;
    os_uint checksum;
    os_char state_bits;

    n = (os_int)ioc_get_ext(b->signals->head, &state_bits, IOC_SIGNAL_DEFAULT);
    if (n <= sizeof(iocBrickHdr) || (state_bits & OSAL_STATE_CONNECTED) == 0) 
    {
        osal_debug_error_int("Invalid received brick length", n);
        return;
    }

    ioc_move_array(b->signals->buf, 0, &hdr, sizeof(iocBrickHdr), 
        OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT);
    if (ioc_brick_int(hdr.buf_sz, IOC_BRICK_BYTES_SZ) != n || osal_validate_brick_header(&hdr)) 
    {
        osal_debug_error_int("Corrupted brick header received", n);
        return;
    }

    /* Setup temporary buffer with data for passing the brick to the callback function.
     */
    b->buf = os_malloc(n, OS_NULL);
    if (b->buf == OS_NULL) return;
    b->buf_sz = n;
    ioc_move_array(b->signals->buf, 0, b->buf, n, OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT);

    /* Verify that checksum is correct
     */
    checksum = (os_uint)ioc_brick_int(hdr.checksum, IOC_BRICK_CHECKSUM_SZ);
    dhdr = (iocBrickHdr*)b->buf;
    os_memclear(dhdr->checksum, IOC_BRICK_CHECKSUM_SZ);
    if (os_checksum((const os_char*)b->buf, n, OS_NULL) != checksum)
    {
        osal_debug_error("brick checksum error");
        // goto getout;
    }

    /* Callback function.
     */
    if (b->receive_callback)
    {
        b->receive_callback(b, b->receive_context);
    }

getout:
    os_free(b->buf, n);
    b->buf = OS_NULL;
    b->buf_sz = 0;
}

/**
****************************************************************************************************

  @brief Receive data into brick buffer.
  @anchor ioc_run_brick_receive

  This function can be called repeatedly from loop to keep on receiveng data.

  @param   b Pointer to brick buffer structure.
  @return  OSAL_SUCCESS all fine, but no complete brick received. OSAL_COMPLETED new brick
           received. Other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_run_brick_receive(
    iocBrickBuffer *b)
{
    os_int state;
    os_char state_bits;
#if IOC_BRICK_RING_BUFFER_SUPPORT
    os_int cmd;
    osalStatus s;
#endif

    if (!b->enable_receive)
    {
#if IOC_BRICK_RING_BUFFER_SUPPORT
        if (b->stream != OS_NULL) {
            ioc_streamer_close(b->stream, OSAL_STREAM_DEFAULT);
            b->stream = OS_NULL;
        }
#endif
        if (b->signals->flat_buffer) {
            if (b->flat_connected) {
                ioc_set(b->signals->cmd, 0);
                b->flat_connected = OS_FALSE;
            }
        }

        return OSAL_SUCCESS;
    }

#if IOC_BRICK_RING_BUFFER_SUPPORT
    if (b->signals->flat_buffer)
    {
#endif
        state = (os_int)ioc_get_ext(b->signals->state, &state_bits, IOC_SIGNAL_DEFAULT);
        if ((state_bits & OSAL_STATE_CONNECTED) == 0) {
            if (b->flat_connected) {
                ioc_set(b->signals->cmd, 0);
                b->flat_connected = OS_FALSE;
            }
            return OSAL_SUCCESS;
        }

        if (!b->flat_connected || os_has_elapsed(&b->flat_frame_timer, 3000) || state != b->prev_state) 
        {
            ioc_lock(b->root);
            os_get_timer(&b->flat_frame_timer);

            if (b->prev_state != state && state) {
                ioc_process_flat_brick_data(b);
            }
            b->prev_state = state;

            if (!b->flat_connected) {
                b->prev_cmd = (os_int)ioc_get_ext(b->prm.frd.cmd, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
                b->flat_connected = OS_TRUE;
            }

            if (++(b->prev_cmd) == 0) b->prev_cmd++;
            ioc_set(b->signals->cmd, b->prev_cmd); 

            ioc_unlock(b->root);
        }

        return OSAL_SUCCESS;
#if IOC_BRICK_RING_BUFFER_SUPPORT
    }

    if (b->stream == OS_NULL)
    {
        /* Keep small pause between tries.
         */
        if (b->err_timer_set) {
            if (!os_has_elapsed(&b->err_timer, 500)) {
                return OSAL_SUCCESS;
            }
            b->err_timer_set = OS_FALSE;
        }
        os_get_timer(&b->err_timer);
        b->err_timer_set = OS_TRUE;

        if (b->prm.frd.state)
        {
            cmd = (os_int)ioc_get_ext(b->prm.frd.cmd, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0 || cmd) {
                ioc_set(b->prm.frd.cmd, 0);
            }

            state = (iocStreamerState)ioc_get_ext(b->prm.frd.state, &state_bits, IOC_SIGNAL_DEFAULT);
            if (state != IOC_STREAM_IDLE || (state_bits & OSAL_STATE_CONNECTED) == 0) {
                return (state_bits & OSAL_STATE_CONNECTED) ? OSAL_STATUS_NOT_CONNECTED : OSAL_SUCCESS;
            }
        }

        b->stream = ioc_streamer_open(OS_NULL, &b->prm, OS_NULL, OSAL_STREAM_READ);
        if (b->stream == OS_NULL) {
            return OSAL_STATUS_FAILED;
        }
        if (b->timeout_ms) {
            osal_stream_set_parameter(b->stream, OSAL_STREAM_READ_TIMEOUT_MS, b->timeout_ms);
        }

        // os_get_timer(&b->open_timer);
        b->pos = 0;
    }

    s = ioc_receive_brick_data(b);
    if (OSAL_IS_ERROR(s))
    {
        ioc_streamer_close(b->stream, OSAL_STREAM_DEFAULT);
        b->stream = OS_NULL;
    }

    return s;
#endif
}

#endif
