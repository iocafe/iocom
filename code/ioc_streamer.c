/**

  @file    ioc_streamer.c
  @brief   Data stream trough memory block API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Ther end of the stream routed trough memory block is flagged as controller and the other as
  device. Controller is the "boss" who starts the transfers. Transfer ends either when the
  while file, etc, has been transferred, or the controller interrupts the transfer.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_STREAMER_SUPPORT

/* Maximum number of streamers when using static memory allocation.
 */
#if OSAL_DYNAMIC_MEMORY_ALLOCATION == 0
static iocStreamer ioc_streamer[IOC_MAX_STREAMERS];
#endif

/* Forward referred static functions.
 */
static osalStatus ioc_streamer_device_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags);

static osalStatus ioc_streamer_device_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags);

static osalStatus ioc_streamer_controller_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags);

static osalStatus ioc_streamer_controller_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags);

static os_int ioc_streamer_read_internal(
    iocStreamerSignals *signals,
    os_char *buf,
    os_int buf_sz,
    os_int n,
    os_int head,
    os_int *tail,
    os_int flags);

static os_int ioc_streamer_write_internal(
    iocStreamerSignals *signals,
    const os_char *buf,
    os_int buf_sz,
    os_int n,
    os_int *head,
    os_int tail);

#if IOC_DEVICE_STREAMER
static void ioc_ctrl_stream_from_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params);

static os_long ioc_streamer_tx_available(
    osalStream stream);

static void ioc_ctrl_stream_to_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params);
#endif


/**
****************************************************************************************************

  @brief Open a stream.
  @anchor ioc_streamer_open

  The ioc_streamer_open() function opens a stream trough memory buffer.

  @param  parameters Not used.
  @param  option Pointer to streamer set parameter up structure.
  @param  status Pointer to integer into which to store the function status code. Value
          OSAL_SUCCESS (0) indicates success and all nonzero values indicate an error.
          This parameter can be OS_NULL, if no status code is needed.

  @param  flags Flags for creating the streamer. Bit fields, combination of:
          - OSAL_STREAM_WRITE
          - OSAL_STREAM_READ

  @return Stream handle, or OS_NULL if memory allocation failed.

****************************************************************************************************
*/
osalStream ioc_streamer_open(
    const os_char *parameters,
    void *option,
    osalStatus *status,
    os_int flags)
{
    iocStreamer *streamer;
    iocStreamerParams *prm;

    /* Allocate streamer structure, either dynamic or static.
     */
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    streamer = (iocStreamer*)os_malloc(sizeof(iocStreamer), OS_NULL);
    if (streamer == OS_NULL) goto getout;
#else
    os_int count;
    streamer = ioc_streamer;
    count = IOC_MAX_STREAMERS;
    while (streamer->used)
    {
        if (--count == 0)
        {
            streamer = OS_NULL;
            goto getout;
        }
        streamer++;
    }
#endif

    /* Initialize streamer structure.
     */
    os_memclear(streamer, sizeof(iocStreamer));
    prm = (iocStreamerParams*)option;
    streamer->prm = (iocStreamerParams*)option;
    streamer->hdr.iface = &ioc_streamer_iface;
    streamer->read_timeout_ms = IOC_STREAMER_TIMEOUT;
    streamer->write_timeout_ms = IOC_STREAMER_TIMEOUT;
    streamer->flags = flags;
    streamer->used = OS_TRUE;
    streamer->step = IOC_SSTEP_INITIALIZED;
    streamer->checksum = OSAL_CHECKSUM_INIT;

    /* Get select parameter, like block number
     */
    if (parameters)
    {
        streamer->select = (os_int)osal_str_to_int(parameters, OS_NULL);
    }

    ioc_set_streamer_error((osalStream)streamer, OSAL_SUCCESS, IOC_STREAMER_MODE_UNCONDITIONAL);
    if (flags & OSAL_STREAM_READ)
    {
        if (prm->is_device) {
            ioc_set(prm->tod.state, IOC_STREAM_IDLE);
            ioc_set(prm->tod.tail, 0);
        }
        else {
            ioc_set(prm->frd.cmd, IOC_STREAM_IDLE);
            ioc_set(prm->frd.tail, 0);
        }
    }

    if (flags & OSAL_STREAM_WRITE)
    {
        if (prm->is_device) {
            ioc_set(prm->frd.state, IOC_STREAM_IDLE);
            ioc_set(prm->frd.head, 0);
            ioc_set(prm->frd.cs, 0);
        }
        else {
            ioc_set(prm->tod.cmd, IOC_STREAM_IDLE);
            ioc_set(prm->tod.head, 0);
            ioc_set(prm->tod.cs, 0);
        }
    }

    /* Set status code and return stream pointer.
     */
getout:
    if (status) *status = streamer ? OSAL_SUCCESS : OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    return (osalStream)streamer;
}


/**
****************************************************************************************************

  @brief Close the stream,
  @anchor ioc_streamer_close

  The ioc_streamer_close() function closes a streamer port, earlier opened by the ioc_streamer_open()
  function. All resource related to the streamer port are freed. Any attempt to use the streamer after
  this call may result in crash.

  @param   stream Stream handle. After this call stream
           pointer will point to invalid memory location.
  @param   flags Ignored, set OSAL_STREAM_DEFAULT (0).
  @return  None.

****************************************************************************************************
*/
void ioc_streamer_close(
    osalStream stream,
    os_int flags)
{
    iocStreamer *streamer;
    iocStreamerParams *prm;
    OSAL_UNUSED(flags);

    /* If called with NULL argument, do nothing.
     */
    if (stream == OS_NULL) return;

    /* Cast stream pointer to streamer structure pointer. The osal_debug_assert here is used
       to detect access to already closed stream while debugging.
     */
    streamer = (iocStreamer*)stream;
    osal_debug_assert(streamer->hdr.iface == &ioc_streamer_iface);

    if (streamer->used)
    {
        prm = streamer->prm;
        if (streamer->flags & OSAL_STREAM_READ)
        {
            if (streamer->prm->is_device) {
                ioc_set(prm->tod.state, IOC_STREAM_IDLE);
                ioc_set(prm->tod.tail, 0);
            }
            else {
                ioc_set(prm->frd.cmd, IOC_STREAM_IDLE);
                ioc_set(prm->frd.tail, 0);
            }
        }

        if (streamer->flags & OSAL_STREAM_WRITE)
        {
            if (prm->is_device) {
                ioc_set(prm->frd.state, IOC_STREAM_IDLE);
                ioc_set(prm->frd.head, 0);
            }
            else {
                ioc_set(prm->tod.cmd, IOC_STREAM_IDLE);
                ioc_set(prm->tod.head, 0);
            }
        }

        streamer->used = OS_FALSE;
    }

    /* Free memory allocated for the streamer structure.
     */
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_free(streamer, sizeof(iocStreamer));
#endif
}


/**
****************************************************************************************************

  @brief Flush data to the stream
  @anchor ioc_streamer_flush

  The flush function can be used to do the final handshake of data transfer.

  @param   stream Stream handle.
  @param   flags Flags for the function, bits. Set OSAL_STREAM_DEFAULT (0) for normal operation
           or OSAL_STREAM_INTERRUPT to interrupt the transfer (final handshake) as failed.

  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and OSAL_PENDING
           that finald closing handshake is still going on. All other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_streamer_flush(
    osalStream stream,
    os_int flags)
{
    os_memsz n_written;
    osalStatus s;

    if (flags & OSAL_STREAM_FINAL_HANDSHAKE)
    {
        s = ioc_streamer_write(stream, osal_str_empty, -1, &n_written, flags);
        switch (s)
        {
            case OSAL_SUCCESS: return OSAL_PENDING;
            case OSAL_COMPLETED: return OSAL_SUCCESS;
            default: return s;
        }
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write data to the stream
  @anchor ioc_streamer_write

  The ioc_streamer_write() function writes up to n bytes of data from buffer to the stream.
  Call this function repeatedly to send the content until it returns OSAL_COMPLETED
  or error code. Call the function with n argument -1 to mark successful completion
  of the transfer.

  @param   stream Stream handle.
  @param   buf Pointer to the beginning of data to place into the the stream,
  @param   n Maximum number of bytes to write. Set -1 to mark completed transfer.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written to streamer port, which may be less than n if there is not enough space
           left in write buffer. If the function fails n_written is set to zero.
  @param   flags Flags for the function. Set OSAL_STREAM_DEFAULT (0) for normal operation
           or OSAL_STREAM_INTERRUPT to interrupt the transfer as failed.

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_COMPLETED transnsfer has been completed.
           Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_streamer_write(
    osalStream stream,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags)
{
    iocStreamer *streamer;
    osalStatus s;

    *n_written = 0;
    if (stream == OS_NULL || buf == OS_NULL || n < -1) return OSAL_STATUS_FAILED;

    /* Cast stream type to streamer structure pointer, get operating system's streamer port handle.
     */
    streamer = (iocStreamer*)stream;
    osal_debug_assert(streamer->hdr.iface == &ioc_streamer_iface);

    /* Move data
     */
#if IOC_CONTROLLER_STREAMER
    if (streamer->prm->is_device)
    {
        s = ioc_streamer_device_write(streamer, &streamer->prm->frd, buf, n, n_written, flags);
    }
    else
    {
        s = ioc_streamer_controller_write(streamer, &streamer->prm->tod, buf, n, n_written, flags);
    }
#else
    s = ioc_streamer_device_write(streamer, &streamer->prm->frd, buf, n, n_written, flags);
#endif

    /* Return success/failure code.
     */
    return s;
}


/**
****************************************************************************************************

  @brief Read data from the stream
  @anchor ioc_streamer_read

  The ioc_streamer_read() function reads up to n bytes of data from streamer port into buffer.
  Call this function repeatedly to receive the content until it returns OSAL_COMPLETED
  or error code. If transfer needs to be interrupted in middle, call ioc_streamer_close()
  function.

  @param   stream Stream handle.
  @param   buf Pointer to buffer to read into.
  @param   n Maximum number of bytes to read. The data buffer must large enough to hold
           at least this many bytes.
  @param   n_read Pointer to integer into which the function stores the number of bytes read,
           which may be less than n if there are fewer bytes available. If the function fails
           n_read is set to zero.
  @param   flags Flags for the function, set OSAL_STREAM_DEFAULT (0).

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_COMPLETED transnsfer has been completed.
           Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_streamer_read(
    osalStream stream,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags)
{
    iocStreamer *streamer;
    iocStreamerSignals *signals;
    osalStatus s;

    *n_read = 0;
    if (stream == OS_NULL || buf == OS_NULL || n < 0) return OSAL_STATUS_FAILED;

    /* Cast stream type to streamer structure pointer, get operating system's streamer
       port handle, check function argument.
     */
    streamer = (iocStreamer*)stream;
    osal_debug_assert(streamer->hdr.iface == &ioc_streamer_iface);

    /* Move data
     */
#if IOC_CONTROLLER_STREAMER
    if (streamer->prm->is_device)
    {
        signals = &streamer->prm->tod;
        s = ioc_streamer_device_read(streamer, signals, buf, n, n_read, flags);
    }
    else
    {
        signals = &streamer->prm->frd;
        s = ioc_streamer_controller_read(streamer, signals, buf, n, n_read, flags);
    }
#else
    signals = &streamer->prm->tod;
    s = ioc_streamer_device_read(streamer, signals, buf, n, n_read, flags);
#endif

    /* Add received data to checksum, werify checksum when all transfers have been completed.
     */
    if ((streamer->flags & OSAL_STREAM_DISABLE_CHECKSUM) == 0) {
        os_checksum(buf, *n_read, &streamer->checksum);
        if (s == OSAL_COMPLETED) {
            if (streamer->checksum != ioc_get(signals->cs))
            {
                osal_trace("Checksum error");
                ioc_set_streamer_error(stream, OSAL_STATUS_CHECKSUM_ERROR,
                    IOC_STREAMER_MODE_SET_ERROR);
                s = OSAL_STATUS_CHECKSUM_ERROR;
            }
        }
    }

    /* Return success/failure code.
     */
    return s;
}


#if IOC_DEVICE_STREAMER
/**
****************************************************************************************************

  @brief Transfer from the IO device
  @anchor ioc_streamer_device_write

  The ioc_streamer_device_write() function handles IO device writing data to stream.
  Call this function repeatedly to receive the content until it returns OSAL_COMPLETED
  or error code. Call the function with n argument -1 to mark successful completion
  of the transfer. Alternativelu, ioc_streamer_flush() with OSAL_STREAM_FINAL_HANDSHAKE
  flag does call write with n = -1 and return SUCCESS/pending return values. This is propably
  more readable way to handle final handshake.

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer holding data to write.
  @param   n Maximum number of bytes to write. Set -1 to mark completed transfer.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written, which may be less than n if there is not space (yet) in outgoing buffer.
           If the function fails n_written is set to zero.
  @param   flags Flags for the function. Set OSAL_STREAM_DEFAULT (0) for normal operation
           or OSAL_STREAM_INTERRUPT to interrupt the transfer as failed.

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_COMPLETED transnsfer has been completed.
           Other return values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_streamer_device_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags)
{
    iocStreamerState cmd;
    os_char cmd_state_bits;
    os_int buf_sz, tail, nbytes, timeout_ms;
    osalStatus s;

    nbytes = 0;

    cmd = (iocStreamerState)ioc_get_ext(signals->cmd, &cmd_state_bits, IOC_SIGNAL_DEFAULT);
    if ((cmd_state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        if (streamer->step != IOC_SSTEP_FAILED_AND_IDLE_SET)
        {
            ioc_set(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
        }
    }

    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
            osal_trace3("IOC_SSTEP_INITIALIZED");
            if (cmd != IOC_STREAM_RUNNING && cmd != IOC_STREAM_COMPLETED)
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            streamer->select = (os_int)ioc_get(signals->select);
            ioc_set(signals->state, IOC_STREAM_RUNNING);
            streamer->step = IOC_SSTEP_TRANSFER_DATA;
            os_get_timer(&streamer->mytimer);
            osal_trace3("IOC_SSTEP_TRANSFER_DATA (SEND)");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DATA:
            if ((cmd != IOC_STREAM_RUNNING && cmd != IOC_STREAM_COMPLETED) || flags & OSAL_STREAM_INTERRUPT)
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING or interrupted");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            buf_sz = signals->buf->n;
            tail = (os_int)ioc_get(signals->tail);

            if (tail < 0 || tail >= buf_sz)
            {
                osal_trace3("IOC_SSTEP_FAILED, no tail");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            if (n > 0)
            {
                nbytes = ioc_streamer_write_internal(signals, buf, buf_sz,
                    (os_int)n, &streamer->head, tail);
            }

            if (nbytes)
            {
                if ((streamer->flags & OSAL_STREAM_DISABLE_CHECKSUM) == 0) {
                    os_checksum(buf, nbytes, &streamer->checksum);
                }
                os_get_timer(&streamer->mytimer);
            }
            else if (n)
            {
                if (n == -1)
                {
                    ioc_set(signals->cs, streamer->checksum);
                    ioc_set(signals->state, IOC_STREAM_COMPLETED);
                    streamer->step = IOC_SSTEP_TRANSFER_DONE;
                    osal_trace3("IOC_SSTEP_TRANSFER_DONE");
                }
                else
                {
                    timeout_ms = streamer->write_timeout_ms;
                    if (timeout_ms > 0 && os_has_elapsed(&streamer->mytimer, timeout_ms))
                    {
                        osal_trace3("Streamer timeout");
                        streamer->step = IOC_SSTEP_FAILED;
                        goto getout;
                    }
                }
            }
            break;

        case IOC_SSTEP_TRANSFER_DONE:
            if (cmd == IOC_STREAM_RUNNING &&
                (cmd_state_bits & OSAL_STATE_CONNECTED))
            {
                if (!os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT)) {
                    break;
                }
            }
            ioc_set(signals->state, IOC_STREAM_IDLE);
            ioc_set(signals->head, 0);
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            osal_trace3("IOC_SSTEP_ALL_COMPLETED");
            break;

        case IOC_SSTEP_FAILED:
            ioc_set(signals->state, IOC_STREAM_INTERRUPT);
            streamer->step = IOC_SSTEP_FAILED2;
            os_get_timer(&streamer->mytimer);
            break;

        case IOC_SSTEP_FAILED2:
            if (cmd == IOC_STREAM_RUNNING &&
                (cmd_state_bits & OSAL_STATE_CONNECTED))
            {
                if (!os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT)) break;
            }

            ioc_set(signals->state, IOC_STREAM_IDLE);
            ioc_set(signals->head, 0);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
            osal_trace3("IOC_SSTEP_FAILED_AND_IDLE_SET");
            break;

        default:
            break;
    }

getout:
    switch (streamer->step)
    {
        case IOC_SSTEP_TRANSFER_DATA:
        case IOC_SSTEP_TRANSFER_DONE:
        case IOC_SSTEP_FAILED:
        case IOC_SSTEP_FAILED2:
            s = OSAL_SUCCESS;
            break;

        case IOC_SSTEP_ALL_COMPLETED:
            s = OSAL_COMPLETED;
            break;

        default:
            s = OSAL_STATUS_FAILED;
            break;
    }
    *n_written = nbytes;
    return s;
}


/**
****************************************************************************************************

  @brief Receive by device
  @anchor ioc_streamer_device_read

  The ioc_streamer_device_read() function handles receiving data from stream, by IO device.
  Call this function repeatedly to receive the content until it returns
  OSAL_COMPLETED or error code. If transfer needs to be interrupted in middle,
  call ioc_streamer_close() function.

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer where to store data.
  @param   n Buffer size, maximum number of bytes to read.
  @param   n_read Pointer to integer into which the function stores the number of bytes
           actually read, which may be less than n if there is no more in incoming buffer.
           If the function fails n_read is set to zero.
  @param   flags Flags for the function. Set OSAL_STREAM_DEFAULT (0) for normal operation
           or OSAL_STREAM_INTERRUPT to interrupt the transfer as failed.

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_COMPLETED transnsfer has been completed.
           Other return values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_streamer_device_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags)
{
    iocStreamerState cmd;
    os_char cmd_state_bits;
    os_int buf_sz, head, nbytes, timeout_ms;
    osalStatus s;

    nbytes = 0;

    cmd = (iocStreamerState)ioc_get_ext(signals->cmd, &cmd_state_bits, IOC_SIGNAL_DEFAULT);
    if ((cmd_state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        if (streamer->step != IOC_SSTEP_FAILED_AND_IDLE_SET)
        {
            ioc_set(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
        }
    }

    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
            osal_trace3("IOC_SSTEP_INITIALIZED");
            streamer->select = (os_int)ioc_get(signals->select);
            if ((cmd != IOC_STREAM_RUNNING && cmd != IOC_STREAM_COMPLETED))
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            ioc_set(signals->state, IOC_STREAM_RUNNING);
            streamer->step = IOC_SSTEP_TRANSFER_DATA;
            os_get_timer(&streamer->mytimer);
            osal_trace3("IOC_SSTEP_TRANSFER_DATA (RECEIVE)");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DATA:
            if ((cmd != IOC_STREAM_RUNNING && cmd != IOC_STREAM_COMPLETED) ||
                flags & OSAL_STREAM_INTERRUPT)
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING,COMPLETED or interrupted");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            buf_sz = signals->buf->n;
            head = (os_int)ioc_get(signals->head);
            if (head < 0 || head >= buf_sz)
            {
                osal_trace3("IOC_SSTEP_FAILED, no head");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            nbytes = ioc_streamer_read_internal(signals, buf, buf_sz,
                (os_int)n, head, &streamer->tail, flags);
            if (nbytes < 0)
            {
                osal_trace3("IOC_SSTEP_FAILED, buffer read failed");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            if (nbytes)
            {
                os_get_timer(&streamer->mytimer);
            }

            /* If more data to come, NOTICE break.
             */
            if (cmd == IOC_STREAM_RUNNING)
            {
                timeout_ms = streamer->read_timeout_ms;
                if (!nbytes && timeout_ms > 0 && os_has_elapsed(&streamer->mytimer, timeout_ms))
                {
                    osal_trace3("IOC_SSTEP_FAILED, receiving data timed out");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }
                break;
            }

            streamer->step = IOC_SSTEP_TRANSFER_DONE;
            osal_trace3("IOC_SSTEP_TRANSFER_DONE");
            break;

        case IOC_SSTEP_TRANSFER_DONE:
            ioc_set(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            osal_trace3("IOC_SSTEP_ALL_COMPLETED");
            break;

        case IOC_SSTEP_FAILED:
            ioc_set(signals->state, IOC_STREAM_INTERRUPT);
            streamer->step = IOC_SSTEP_FAILED2;
            os_get_timer(&streamer->mytimer);
            break;

        case IOC_SSTEP_FAILED2:
            if ((cmd == IOC_STREAM_RUNNING || cmd == IOC_STREAM_COMPLETED) &&
                (cmd_state_bits & OSAL_STATE_CONNECTED))
            {
                if (!os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT)) break;
            }
            ioc_set(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
            osal_trace3("IOC_SSTEP_FAILED_AND_IDLE_SET");
            break;

        default:
            break;
    }

getout:
    switch (streamer->step)
    {
        case IOC_SSTEP_TRANSFER_DATA:
        case IOC_SSTEP_TRANSFER_DONE:
        case IOC_SSTEP_FAILED:
        case IOC_SSTEP_FAILED2:
            s = OSAL_SUCCESS;
            break;

        case IOC_SSTEP_ALL_COMPLETED:
            s = OSAL_COMPLETED;
            break;

        default:
            s = OSAL_STATUS_FAILED;
            break;
    }
    *n_read = nbytes;
    return s;
}
#endif


#if IOC_CONTROLLER_STREAMER
/**
****************************************************************************************************

  @brief Transfer from the controller
  @anchor ioc_streamer_controller_write

  The ioc_streamer_controller_write() function handles a controller writing data to stream.

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer to write from.
  @param   n Maximum number of bytes to write.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written, which may be less than n if there is not space (yet) in outgoing
           buffer. If the function fails n_written is set to zero.
  @param   flags Flags for the function. Set OSAL_STREAM_DEFAULT (0) for normal operation
           or OSAL_STREAM_INTERRUPT to interrupt the transfer as failed.

  @return  OSAL_SUCCESS to indicate success.

****************************************************************************************************
*/
static osalStatus ioc_streamer_controller_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags)
{
    iocStreamerState state;
    os_char state_bits;
    os_int buf_sz, tail, nbytes, timeout_ms;
    osalStatus s;

    nbytes = 0;

    state = (iocStreamerState)ioc_get_ext(signals->state, &state_bits, IOC_SIGNAL_DEFAULT);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        if (streamer->step != IOC_SSTEP_FAILED_AND_IDLE_SET)
        {
            ioc_set(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
        }
    }

    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
            ioc_set(signals->select, streamer->select);
            ioc_set(signals->cmd, IOC_STREAM_RUNNING);
            streamer->step = IOC_SSTEP_INITIALIZED2;
            os_get_timer(&streamer->mytimer);
            osal_trace3("IOC_SSTEP_INITIALIZED");
            break;

        case IOC_SSTEP_INITIALIZED2:
            if (flags & OSAL_STREAM_INTERRUPT)
            {
                osal_trace3("IOC_SSTEP_FAILED, interrupted");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            if (state != IOC_STREAM_RUNNING)
            {
                if (os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT))
                {
                    osal_trace3("Streamer timeout");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }
                break;
            }
            streamer->step = IOC_SSTEP_TRANSFER_DATA;
            osal_trace3("IOC_SSTEP_TRANSFER_DATA (SEND)");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DATA:
            if (state != IOC_STREAM_RUNNING || flags & OSAL_STREAM_INTERRUPT)
            {
                osal_trace3("IOC_SSTEP_FAILED, state != RUNNING or interrupted");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            if (n > 0)
            {
                buf_sz = signals->buf->n;
                tail = (os_int)ioc_get(signals->tail);
                if (tail < 0 || tail >= buf_sz)
                {
                    osal_trace3("IOC_SSTEP_FAILED, no tail");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }

                nbytes = ioc_streamer_write_internal(signals, buf, buf_sz,
                    (os_int)n, &streamer->head, tail);
                if ((streamer->flags & OSAL_STREAM_DISABLE_CHECKSUM) == 0) {
                    os_checksum(buf, nbytes, &streamer->checksum);
                }

                /* More data to come, break.
                 */
                os_get_timer(&streamer->mytimer);
                break;
            }
            if (n == 0)
            {
                timeout_ms = streamer->write_timeout_ms;
                if (timeout_ms > 0 && os_has_elapsed(&streamer->mytimer, timeout_ms))
                {
                    osal_trace3("Streamer timeout");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }
                break;
            }

            ioc_set(signals->cs, streamer->checksum);
            ioc_set(signals->cmd, IOC_STREAM_COMPLETED);
            streamer->step = IOC_SSTEP_TRANSFER_DONE;
            osal_trace3("IOC_SSTEP_TRANSFER_DONE");
            break;

        case IOC_SSTEP_TRANSFER_DONE:
            if (state == IOC_STREAM_RUNNING)
            {
                if (!os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT)) break;
            }
            ioc_set(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            osal_trace3("IOC_SSTEP_ALL_COMPLETED");
            break;

        case IOC_SSTEP_FAILED:
            ioc_set(signals->cmd, IOC_STREAM_INTERRUPT);
            streamer->step = IOC_SSTEP_FAILED2;
            os_get_timer(&streamer->mytimer);
            break;

        case IOC_SSTEP_FAILED2:
            if (state == IOC_STREAM_RUNNING)
            {
                if (!os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT)) break;
            }
            ioc_set(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
            osal_trace3("IOC_SSTEP_FAILED_AND_IDLE_SET");
            break;

        default:
            break;
    }

getout:
    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED2:
        case IOC_SSTEP_TRANSFER_DATA:
        case IOC_SSTEP_TRANSFER_DONE:
        case IOC_SSTEP_FAILED:
        case IOC_SSTEP_FAILED2:
            s = OSAL_SUCCESS;
            break;

        case IOC_SSTEP_ALL_COMPLETED:
            s = OSAL_COMPLETED;
            break;

        default:
            s = OSAL_STATUS_FAILED;
            break;
    }
    *n_written = nbytes;
    return s;
}


/**
****************************************************************************************************

  @brief Receive by controller
  @anchor ioc_streamer_controller_read

  The ioc_streamer_controller_read() function handles controller reveiving data from
  IO device.

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer where to store data.
  @param   n Buffer size, maximum number of bytes to read.
  @param   n_read Pointer to integer into which the function stores the number of bytes
           actually read, which may be less than n if there is no more in incoming buffer.
           If the function fails n_read is set to zero.
  @param   flags Flags for the function. Set OSAL_STREAM_DEFAULT (0) for normal operation
           or OSAL_STREAM_INTERRUPT to interrupt the transfer as failed.

  @return  OSAL_SUCCESS when no errors.
           OSAL_COMPLETED when transfer has been completed.
           Other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_streamer_controller_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags)
{
    iocStreamerState state;
    os_char state_bits;
    os_int buf_sz, head, nbytes, timeout_ms;
    osalStatus s;

    nbytes = 0;

    state = (iocStreamerState)ioc_get_ext(signals->state, &state_bits, IOC_SIGNAL_DEFAULT);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        if (streamer->step != IOC_SSTEP_FAILED_AND_IDLE_SET)
        {
            ioc_set(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
        }
    }

    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
            ioc_set(signals->select, streamer->select);
            ioc_set(signals->cmd, IOC_STREAM_RUNNING);
            streamer->step = IOC_SSTEP_INITIALIZED2;
            os_get_timer(&streamer->mytimer);
            osal_trace3("IOC_SSTEP_INITIALIZED");
            break;

        case IOC_SSTEP_INITIALIZED2:
            if (state != IOC_STREAM_RUNNING && state != IOC_STREAM_COMPLETED)
            {
                if (os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT))
                {
                    osal_trace3("Streamer timeout");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }
                break;
            }

            streamer->step = IOC_SSTEP_TRANSFER_DATA;
            osal_trace3("IOC_SSTEP_TRANSFER_DATA (RECEIVE)");
            break;

        case IOC_SSTEP_TRANSFER_DATA:
            if ((state != IOC_STREAM_RUNNING && state != IOC_STREAM_COMPLETED)  ||
                 flags & OSAL_STREAM_INTERRUPT)
            {
                osal_trace3("IOC_SSTEP_FAILED, state != RUNNING,COMPLETED");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            buf_sz = signals->buf->n;
            head = (os_int)ioc_get(signals->head);
            if (head < 0 || head >= buf_sz)
            {
                osal_trace3("IOC_SSTEP_FAILED, no head");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            nbytes = ioc_streamer_read_internal(signals, buf, buf_sz,
                (os_int)n, head, &streamer->tail, flags);
            if (nbytes < 0)
            {
                osal_trace2("IOC_SSTEP_FAILED, buffer read failed");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            if (nbytes)
            {
                os_get_timer(&streamer->mytimer);
            }

            /* If more data to come, break.
             */
            if (state == IOC_STREAM_RUNNING)
            {
                timeout_ms = streamer->read_timeout_ms;
                if (!nbytes && timeout_ms > 0 && os_has_elapsed(&streamer->mytimer, timeout_ms))
                {
                    osal_trace3("Streamer timeout");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }

                break;
            }

            streamer->step = IOC_SSTEP_TRANSFER_DONE;
            /* continues ... */

        case IOC_SSTEP_TRANSFER_DONE:
            ioc_set(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_TRANSFER_DONE2;
            osal_trace3("IOC_SSTEP_TRANSFER_DONE");
            break;

        case IOC_SSTEP_TRANSFER_DONE2:
            if (state == IOC_STREAM_RUNNING || state == IOC_STREAM_COMPLETED)
            {
                if (!os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT)) break;
            }
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            ioc_set(signals->tail, 0);
            osal_trace3("IOC_SSTEP_ALL_COMPLETED");
            break;

        case IOC_SSTEP_FAILED:
            ioc_set(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED2;
            os_get_timer(&streamer->mytimer);
            break;

        case IOC_SSTEP_FAILED2:
            if (state == IOC_STREAM_RUNNING)
            {
                if (!os_has_elapsed(&streamer->mytimer, IOC_STREAMER_TIMEOUT)) break;
            }
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
            osal_trace3("IOC_SSTEP_FAILED_AND_IDLE_SET");
            break;

        default:
            break;
    }

getout:
    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED2:
        case IOC_SSTEP_TRANSFER_DATA:
        case IOC_SSTEP_TRANSFER_DONE2:
        case IOC_SSTEP_TRANSFER_DONE:
        case IOC_SSTEP_FAILED:
        case IOC_SSTEP_FAILED2:
            s = OSAL_SUCCESS;
            break;

        case IOC_SSTEP_ALL_COMPLETED:
            s = OSAL_COMPLETED;
            break;

        default:
            s = OSAL_STATUS_FAILED;
            break;
    }
    *n_read = nbytes;
    return s;
}

#endif


/**
****************************************************************************************************

  @brief Read data from ring buffer.
  @anchor ioc_streamer_read_internal

  The ioc_streamer_read_internal() function read data from ring buffer in memory block and
  moves tail.

  @return  Number of bytes stored.

****************************************************************************************************
*/
static os_int ioc_streamer_read_internal(
    iocStreamerSignals *signals,
    os_char *buf,
    os_int buf_sz,
    os_int n,
    os_int head,
    os_int *tail,
    os_int flags)
{
    os_int nbytes, rdnow, ltail;
    os_char state_bits;

    nbytes = 0;
    ltail = *tail;

    if (ltail > head)
    {
        rdnow = buf_sz - ltail;
        if (rdnow > n) rdnow = n;
        if (rdnow > 0)
        {
            state_bits = ioc_move_array(signals->buf, ltail, buf, rdnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0) return -1;

            ltail += rdnow;
            if (ltail >= buf_sz) ltail = 0;

            buf += rdnow;
            n -= rdnow;
            nbytes += rdnow;
            osal_trace3_int("DATA MOVED 1, bytes = ", rdnow);
        }
    }

    if (ltail < head)
    {
        rdnow = head - ltail;
        if (rdnow > n) rdnow = n;

        if (rdnow > 0)
        {
            state_bits = ioc_move_array(signals->buf, ltail, buf, rdnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0) return -1;

            ltail += rdnow;
            nbytes += rdnow;
            osal_trace3_int("DATA MOVED 2, bytes = ", rdnow);
        }
    }

    if (nbytes && (flags & OSAL_STREAM_PEEK) == 0)
    {
        ioc_set(signals->tail, ltail);
        *tail = ltail;
    }

    return nbytes;
}


/**
****************************************************************************************************

  @brief Write data to ring buffer.
  @anchor ioc_streamer_write_internal

  The ioc_streamer_write_internal() function stores data to ring buffer in memory block and
  moves head.

  @return  Number of bytes stored.

****************************************************************************************************
*/
static os_int ioc_streamer_write_internal(
    iocStreamerSignals *signals,
    const os_char *buf,
    os_int buf_sz,
    os_int n,
    os_int *head,
    os_int tail)
{
    os_int nbytes, wrnow;

    nbytes = 0;

    if (*head >= tail)
    {
        wrnow = buf_sz - *head;
        if (!tail) wrnow--;
        if (wrnow > n) wrnow = n;
        if (wrnow > 0)
        {
            ioc_move_array(signals->buf, *head, (os_char*)buf, wrnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);

            *head += wrnow;
            if (*head >= buf_sz) *head = 0;

            buf += wrnow;
            n -= wrnow;
            nbytes += wrnow;
        }
    }

    if (*head < tail)
    {
        wrnow = tail - *head - 1;
        if (wrnow > n) wrnow = n;

        if (wrnow > 0)
        {
            ioc_move_array(signals->buf, *head, (os_char*)buf, wrnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);

            *head += wrnow;
            nbytes += wrnow;
        }
    }

    if (nbytes)
    {
        ioc_set(signals->head, *head);
    }

    return nbytes;
}


/**
****************************************************************************************************

  @brief Initialize memory block streamer data structure.
  @anchor ioc_streamer_initialize

  The ioc_set_streamer_error() function stored  streamer error code for devide into in "exp"
  memory block. Setting is conditioned by mode.

  @param   stream Pointer to streamer structure.
  @param   s Error code (or status code) to set.
  @param   mode also_good One of IOC_STREAMER_MODE_UNCONDITIONAL, IOC_STREAMER_MODE_SET_ERROR, or
           IOC_STREAMER_MODE_COMPLETED.

  @return  None.

****************************************************************************************************
*/
void ioc_set_streamer_error(
    osalStream stream,
    osalStatus s,
    iocStremErrSetMode mode)
{
    iocStreamerParams *prm;
    const iocSignal *sig;
    iocStreamer *streamer;
    osalStatus old_s;

    if (stream == OS_NULL) return;
    streamer = (iocStreamer*)stream;
    prm = streamer->prm;
    if (prm->is_device)
    {
        if (mode != IOC_STREAMER_MODE_SET_ERROR || OSAL_IS_ERROR(s))
        {
            sig = (streamer->flags & OSAL_STREAM_READ) ? prm->tod.err : prm->frd.err;
            if (sig) {
                if (sig->handle) {
                    if (mode == IOC_STREAMER_MODE_COMPLETED) {
                        old_s = ioc_get_ext(sig, OS_NULL, IOC_SIGNAL_NO_TBUF_CHECK);
                        if (!OSAL_IS_ERROR(old_s)) {
                            ioc_set(sig, s);
                        }
                    }
                    else {
                        ioc_set(sig, s);
                    }
                }
            }
#if OSAL_DEBUG
            else {
                osal_debug_error("NULL signal, check streamer JSON conf");
            }
#endif
        }
    }
}


/**
****************************************************************************************************

  @brief Initialize memory block streamer data structure.
  @anchor ioc_streamer_initialize

  The ioc_streamer_initialize() clears static memory allocated for streamers. This is needed
  in some microcontroller when RAM is not cleared in soft reboot.

  @return  None.

****************************************************************************************************
*/
void ioc_streamer_initialize(
    void)
{
#if OSAL_DYNAMIC_MEMORY_ALLOCATION == 0
    os_memclear(ioc_streamer, sizeof(ioc_streamer));
#endif
}


#if IOC_DEVICE_STREAMER
/**
****************************************************************************************************

  @brief Initialize control stream.
  @anchor ioc_init_control_stream

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
void ioc_init_control_stream(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    os_memclear(ctrl, sizeof(iocControlStreamState));

    ioc_set(params->frd.state, 0);
    ioc_set(params->tod.state, 0);

#if OSAL_DEBUG
    ctrl->initialized = 'I';
#endif
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
osalStatus ioc_run_control_stream(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    iocStreamerState cmd;
    osPersistentBlockNr select;
    os_char state_bits;
    osalStatus s = OSAL_NOTHING_TO_DO, rval;

    /* Just for debugging, assert here that ioc_init_control_stream() has been called.
     */
    osal_debug_assert(ctrl->initialized == 'I');

    /* No status yet
     */
    ctrl->transfer_status = IOC_NO_BLOCK_TRANSFERRED;

    if (ctrl->frd == OS_NULL)
    {
        cmd = (iocStreamerState)ioc_get_ext(params->frd.cmd, &state_bits, IOC_SIGNAL_DEFAULT);
        if (cmd == IOC_STREAM_RUNNING && (state_bits & OSAL_STATE_CONNECTED))
        {
            osal_trace3("IOC_STREAM_RUNNING command");
            ctrl->frd = ioc_streamer_open(OS_NULL, params, OS_NULL, OSAL_STREAM_WRITE);

            if (ctrl->frd)
            {
                ctrl->transferring_default_config = OS_FALSE;
                select = (osPersistentBlockNr)ioc_get(params->frd.select);

                if (select == OS_PBNR_DEFAULTS)
                {
                    ctrl->transferring_default_config = OS_TRUE;
                    ctrl->default_config_pos = 0;
                    ctrl->fdr_persistent_ok = OS_TRUE;
                }
                else
                {
                    ctrl->fdr_persistent = os_persistent_open(select, OS_NULL, OSAL_PERSISTENT_READ);
                    ctrl->fdr_persistent_ok = (ctrl->fdr_persistent != OS_NULL);
                    if (ctrl->fdr_persistent == OS_NULL)
                    {
                        osal_debug_error_int("Reading persistent block failed, select=", select);
                        ioc_set_streamer_error(ctrl->frd, OSAL_STATUS_READING_FILE_FAILED,
                            IOC_STREAMER_MODE_SET_ERROR);
                    }
                }

                /* If we are getting certificate chain, mark that we have it.
                 */
                if (select == OS_PBNR_CLIENT_CERT_CHAIN) {
                    osal_set_network_state_int(OSAL_NS_NO_CERT_CHAIN, 0, OS_FALSE);
                }

                os_get_timer(&ctrl->timer_ms);
            }
        }
    }

    if (ctrl->frd)
    {
        ioc_ctrl_stream_from_device(ctrl, params);
        s = OSAL_SUCCESS;
    }

    /* Program has been transferred and we are waiting for programming status
     */
    if (ctrl->poll_programming_status) {
        rval = get_device_programming_status();
        if (rval != OSAL_PENDING) {
            ioc_set(params->tod.err, rval);
            ctrl->poll_programming_status = OS_FALSE;
        }
    }

    if (ctrl->tod == OS_NULL)
    {
        cmd = (iocStreamerState)ioc_get_ext(params->tod.cmd, &state_bits, IOC_SIGNAL_DEFAULT);
        if (cmd == IOC_STREAM_RUNNING && (state_bits & OSAL_STATE_CONNECTED))
        {
            ctrl->tod = ioc_streamer_open(OS_NULL, params, OS_NULL, OSAL_STREAM_READ);

            if (ctrl->tod) {
                select = (osPersistentBlockNr)ioc_get(params->tod.select);
                ctrl->transferred_block_nr = select;
                ctrl->transferring_program = OS_FALSE;
                if (select == OS_PBNR_FLASH_PROGRAM) {
                    ctrl->transferring_program = OS_TRUE;
                    rval = osal_start_device_programming();
                    if (OSAL_IS_ERROR(rval)) {
                        ioc_set_streamer_error(ctrl->tod, rval, IOC_STREAMER_MODE_SET_ERROR);
                        ioc_streamer_close(ctrl->tod, OSAL_STREAM_DEFAULT);
                        ctrl->tod = OS_NULL;
                    }
                }
                else {
                    ctrl->tod_persistent = os_persistent_open(select, OS_NULL, OSAL_PERSISTENT_WRITE);
                    if (ctrl->tod_persistent == OS_NULL)
                    {
                        osal_debug_error_int("Writing persistent block failed", select);
                        ioc_set_streamer_error(ctrl->tod, OSAL_STATUS_NO_ACCESS_RIGHT,
                            IOC_STREAMER_MODE_SET_ERROR);
                    }
                }
            }
        }
    }

    if (ctrl->tod)
    {
        ioc_ctrl_stream_to_device(ctrl, params);
        s = OSAL_SUCCESS;
    }

    return s;
}


/**
****************************************************************************************************

  @brief Move data from IO device to controller.
  @anchor ioc_ctrl_stream_from_device

  This code is used in IO device. The function is called repeatedly to run data transfer
  from IO device to controller. The function reads data from persistent storage and
  writes it to stream buffer in memory block. When the data ends, the device will show
  IOC_STREAM_COMPLETED state.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
static void ioc_ctrl_stream_from_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_char *buf = OS_NULL;
    os_memsz buf_sz = 0;
#else
    os_char buf[256];
    const os_memsz buf_sz = sizeof(buf);
#endif

    os_memsz rdnow, n_written, n_read, bytes;
    osalStatus s;

    if (ctrl->fdr_persistent || ctrl->transferring_default_config)
    {
        bytes = ioc_streamer_tx_available(ctrl->frd);
        while (OS_TRUE)
        {
            if (bytes <= 0) {
                if (!os_has_elapsed(&ctrl->timer_ms, IOC_STREAMER_TIMEOUT)) {
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
                    os_free(buf, buf_sz);
#endif
                    return;
                }
                ioc_set_streamer_error(ctrl->frd, OSAL_STATUS_TIMEOUT,
                    IOC_STREAMER_MODE_SET_ERROR);
                break;
            }
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
            if (buf == OS_NULL)
            {
                buf_sz = params->frd.buf->n - 1;
                osal_debug_assert(buf_sz > 0);
                buf = os_malloc(buf_sz, OS_NULL);
                if (buf == OS_NULL) return;
            }
#endif
            os_get_timer(&ctrl->timer_ms);

            rdnow = bytes;
            if (rdnow > buf_sz) rdnow = buf_sz;

            /* Get static default network congiguration.
             */
            if (ctrl->transferring_default_config) {
                if (params->default_config == OS_NULL) {
                    ctrl->fdr_persistent_ok = OS_FALSE;
                    break;
                }
                n_read = params->default_config_sz - ctrl->default_config_pos;
                if (rdnow < n_read) n_read = rdnow;
                os_memcpy(buf, params->default_config + ctrl->default_config_pos, n_read);
                ctrl->default_config_pos += (os_int)n_read;
            }

            /* Get actual persistent data.
             */
            else {
                n_read = os_persistent_read(ctrl->fdr_persistent, buf, rdnow);
            }

            if (n_read > 0) {
                s = ioc_streamer_write(ctrl->frd, buf, n_read, &n_written, OSAL_STREAM_DEFAULT);
                if (s) break;
                osal_debug_assert(n_written == n_read);
            }

            /* If all has been read?
             */
            if (n_read < rdnow) {
                if (n_read < 0) ctrl->fdr_persistent_ok = OS_FALSE;
                break;
            }
            bytes -= n_read;
        }

        os_persistent_close(ctrl->fdr_persistent, OSAL_PERSISTENT_DEFAULT);
        ctrl->fdr_persistent = OS_NULL;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
        os_free(buf, buf_sz);
#endif
    }

    if (!ctrl->fdr_persistent_ok) {
        ioc_set_streamer_error(ctrl->frd, OSAL_STATUS_READING_FILE_FAILED, IOC_STREAMER_MODE_SET_ERROR);
    }

    /* Finalize any handshaking signal stuff.
     */
    s = ioc_streamer_flush(ctrl->frd, ctrl->fdr_persistent_ok
        ? OSAL_STREAM_FINAL_HANDSHAKE
        : OSAL_STREAM_FINAL_HANDSHAKE|OSAL_STREAM_INTERRUPT);

    if (s == OSAL_PENDING) return;

    /* Close the stream
     */
    ioc_set_streamer_error(ctrl->frd, OSAL_COMPLETED, IOC_STREAMER_MODE_COMPLETED);
    ioc_streamer_close(ctrl->frd, OSAL_STREAM_DEFAULT);
    ctrl->frd = OS_NULL;
}


static os_long ioc_streamer_tx_available(
    osalStream stream)
{
    iocStreamer *streamer;
    iocStreamerSignals *signals;
    os_int buf_sz, tail, space_available, buffered_bytes;
    os_boolean is_device;
    os_char state_bits;

    if (stream == OS_NULL) return 0;
    streamer = (iocStreamer*)stream;
    is_device = streamer->prm->is_device;
    signals = is_device ? &streamer->prm->frd : &streamer->prm->tod;

    buf_sz = signals->buf->n;
    tail = (os_int)ioc_get_ext(signals->tail, &state_bits, IOC_SIGNAL_DEFAULT);

    if ((state_bits & OSAL_STATE_CONNECTED) == 0 || tail < 0 || tail >= buf_sz)
    {
        return -1;
    }

    buffered_bytes = streamer->head - tail;
    if (buffered_bytes < 0) buffered_bytes += buf_sz;
    space_available = buf_sz - buffered_bytes - 1;
    return space_available;
}


/**
****************************************************************************************************

  @brief Move data from controller to IO device.
  @anchor ioc_ctrl_stream_to_device

  This code is used in IO device. The function is called repeatedly to run data transfer
  from controller to IO device. The function reads data from persistent storage and
  writes it to stream buffer in memory block. When the data ends, the device will show
  IOC_STREAM_COMPLETED state. If transfer is interrupted (for example reading persistent
  storage fails, the IOC_STREAM_INTERRUPT state is set to memory block.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
static void ioc_ctrl_stream_to_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_char *buf;
    os_memsz buf_sz;
#else
    os_char buf[256];
    const os_memsz buf_sz = sizeof(buf);
#endif
    os_memsz n_read;
    osalStatus s;
    os_int stream_flags;
    osalStatus rval;

    stream_flags = (ctrl->tod_persistent || ctrl->transferring_program)
        ? OSAL_STREAM_DEFAULT : OSAL_STREAM_INTERRUPT;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    buf_sz = params->tod.buf->n - 1;
    osal_debug_assert(buf_sz > 0);
    buf = os_malloc(buf_sz, OS_NULL);
    if (buf == OS_NULL) return;
#endif

    do {
        s = ioc_streamer_read(ctrl->tod, buf, buf_sz, &n_read, stream_flags);
        if (n_read == 0) {
            if (s == OSAL_SUCCESS) {
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
                os_free(buf, buf_sz);
#endif
                return;
            }
            break;
        }
        if (ctrl->transferring_program) {
            rval = osal_program_device(buf, n_read);
            if (rval != OSAL_SUCCESS) {
                ioc_set_streamer_error(ctrl->tod, rval, OS_FALSE);
                s = OSAL_DEVICE_PROGRAMMING_FAILED;
            }
        }

        else if (ctrl->tod_persistent) {
            if (os_persistent_write(ctrl->tod_persistent, buf, n_read)) {
                ioc_set_streamer_error(ctrl->tod, OSAL_STATUS_WRITING_FILE_FAILED, OS_FALSE);
            }
        }
    }
    while (s == OSAL_SUCCESS);

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_free(buf, buf_sz);
#endif

    if (s != OSAL_COMPLETED) stream_flags = OSAL_STREAM_INTERRUPT;

    if (ctrl->transferring_program) {
        if (s == OSAL_COMPLETED) {
            osal_finish_device_programming(0);
            ctrl->poll_programming_status = OS_TRUE;
        }
        else {
            osal_cancel_device_programming();
        }
    }

    else if (ctrl->tod_persistent)
    {

        ctrl->transfer_status = IOC_BLOCK_WRITTEN;
        os_persistent_close(ctrl->tod_persistent, stream_flags);
        ctrl->tod_persistent = OS_NULL;
        ioc_set_streamer_error(ctrl->tod, OSAL_COMPLETED, IOC_STREAMER_MODE_COMPLETED);
    }

    ioc_streamer_close(ctrl->tod, stream_flags);
    ctrl->tod = OS_NULL;

    if (s == OSAL_COMPLETED) {
        if (ctrl->transferred_block_nr == OS_PBNR_CLIENT_CERT_CHAIN /* ||
            ctrl->transferred_block_nr == OS_PBNR_FLASH_PROGRAM */)
        {
            osal_reboot(0);
        }
    }
}
#endif


/** Stream interface for OSAL streamers. This is structure is filled with
    function pointers to memory block streamer implementation.
 */
const osalStreamInterface ioc_streamer_iface
 = {OSAL_STREAM_IFLAG_NONE,
    ioc_streamer_open,
    ioc_streamer_close,
    osal_stream_default_accept,
    ioc_streamer_flush,
    osal_stream_default_seek,
    ioc_streamer_write,
    ioc_streamer_read,
    osal_stream_default_select,
    OS_NULL,
    OS_NULL};

#endif
