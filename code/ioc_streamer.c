/**

  @file    ioc_streamer.c
  @brief   Data stream trough memory block API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.12.2019

  Ther end of the stream routed trough memory block is flagged as controller and the other as
  device. Controller is the "boss" who starts the transfers. Transfer ends either when the
  while file, etc, has been transferred, or the controller interrupts the transfer.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#define OSAL_TRACE 3

#include "iocom.h"
#if IOC_STREAMER_SUPPORT


/* Maximum number of streamers when using static memory allocation.
 */
#if OSAL_DYNAMIC_MEMORY_ALLOCATION == 0
static iocStreamer ioc_streamer[IOC_MAX_STREAMERS];
#endif


static osalStatus ioc_streamer_device_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written);

static osalStatus ioc_streamer_device_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read);

static osalStatus ioc_streamer_controller_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written);

static osalStatus ioc_streamer_controller_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read);

static os_int osal_streamer_read_internal(
    iocStreamerSignals *signals,
    os_char *buf,
    os_int buf_sz,
    os_int n,
    os_int head,
    os_int *tail);

static os_int osal_streamer_write_internal(
    iocStreamerSignals *signals,
    const os_char *buf,
    os_int buf_sz,
    os_int n,
    os_int *head,
    os_int tail);

void ioc_ctrl_stream_from_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params);

void ioc_ctrl_stream_to_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params);


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
    streamer->prm = (iocStreamerParams*)option;
    streamer->hdr.iface = &ioc_streamer_iface;
    streamer->flags = flags;
    streamer->used = OS_TRUE;
    streamer->step = IOC_SSTEP_INITIALIZED;

    osal_trace3_int("ioc_streamer_open()", (os_long)streamer);

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
  function. All resource related to the streamer port are freed. Any attemp to use the streamer after
  this call may result in crash.

  @param   stream Stream handle. After this call stream
           pointer will point to invalid memory location.
  @return  None.

****************************************************************************************************
*/
void ioc_streamer_close(
    osalStream stream,
    os_int flags)
{
    iocStreamer *streamer;

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
        if (streamer->prm->is_device)
        {
            if (streamer->flags & OSAL_STREAM_WRITE)
            {
                ioc_sets0_int(streamer->prm->frd.state, IOC_STREAM_IDLE);
            }
            else
            {
                ioc_sets0_int(streamer->prm->tod.state, IOC_STREAM_IDLE);
            }
        }
        else
        {
            if (streamer->flags & OSAL_STREAM_WRITE)
            {
                ioc_sets0_int(streamer->prm->tod.cmd, IOC_STREAM_IDLE);
            }
            else
            {
                ioc_sets0_int(streamer->prm->frd.cmd, IOC_STREAM_IDLE);
            }
        }

        streamer->used = OS_FALSE;
    }

    /* Free memory allocated for the streamer structure.
     */
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_free(streamer, sizeof(iocStreamer));
#endif

    osal_trace3_int("ioc_streamer_close()", (os_long)streamer);
}


/**
****************************************************************************************************

  @brief Flush data to the stream
  @anchor ioc_streamer_flush

  X

  @param   stream Stream handle.
  @param   flags Bit fields.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and OSAL_STATUS_PENDING
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
        s = ioc_streamer_write(stream, "", -1, &n_written, flags);
        switch (s)
        {
            case OSAL_SUCCESS: return OSAL_STATUS_PENDING;
            case OSAL_STATUS_COMPLETED: return OSAL_SUCCESS;
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
  Call this function repeatedly to send the content until it returns OSAL_STATUS_COMPLETED
  or error code. Call the function with n argument -1 to mark successfull completion
  of the transfer.

  @param   stream Stream handle.
  @param   buf Pointer to the beginning of data to place into the the stream,
  @param   n Maximum number of bytes to write. Set -1 to mark completed transfer.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written to streamer port, which may be less than n if there is not enough space
           left in write buffer. If the function fails n_written is set to zero.
  @param   flags Flags for the function, ignored. Set OSAL_STREAM_DEFAULT (0).

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_STATUS_COMPLETED transnsfer has been completed.
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
        s = ioc_streamer_device_write(streamer, &streamer->prm->frd, buf, n, n_written);
    }
    else
    {
        s = ioc_streamer_controller_write(streamer, &streamer->prm->tod, buf, n, n_written);
    }
#else
    s = ioc_streamer_device_write(streamer, &streamer->prm->frd, buf, n, n_written);
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
  Call this function repeatedly to receive the content until it returns OSAL_STATUS_COMPLETED
  or error code. If transfer needs to be interrupted in middle, call ioc_streamer_close()
  function.

  @param   stream Stream handle.
  @param   buf Pointer to buffer to read into.
  @param   n Maximum number of bytes to read. The data buffer must large enough to hold
           at least this many bytes.
  @param   n_read Pointer to integer into which the function stores the number of bytes read,
           which may be less than n if there are fewer bytes available. If the function fails
           n_read is set to zero.
  @param   flags Flags for the function, ignored. Set OSAL_STREAM_DEFAULT (0).

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_STATUS_COMPLETED transnsfer has been completed.
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
        s = ioc_streamer_device_read(streamer, &streamer->prm->tod, buf, n, n_read);
    }
    else
    {
        s = ioc_streamer_controller_read(streamer, &streamer->prm->frd, buf, n, n_read);
    }
#else
    s = ioc_streamer_device_read(streamer, &streamer->prm->tod, buf, n, n_read);
#endif

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
  Call this function repeatedly to receive the content until it returns OSAL_STATUS_COMPLETED
  or error code. Call the function with n argument -1 to mark successfull completion
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

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_STATUS_COMPLETED transnsfer has been completed.
           Other return values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_streamer_device_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written)
{
    iocStreamerState cmd;
    os_char state_bits;
    os_int buf_sz, tail, nbytes;
    osalStatus s;

    nbytes = 0;

    cmd = (iocStreamerState)ioc_gets_int(signals->cmd, &state_bits);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        streamer->step = IOC_SSTEP_FAILED;
        goto getout;
    }

    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
            osal_trace3("IOC_SSTEP_INITIALIZED");
            if (cmd != IOC_STREAM_RUNNING)
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            streamer->head = 0;
            ioc_sets0_int(signals->head, 0);
            streamer->select = ioc_gets0_int(signals->select);
            ioc_sets0_int(signals->state, IOC_STREAM_RUNNING);
            streamer->step = IOC_SSTEP_TRANSFER_DATA;
            osal_trace3("IOC_SSTEP_TRANSFER_DATA");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DATA:
            if (cmd != IOC_STREAM_RUNNING)
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            if (n > 0)
            {
                buf_sz = signals->buf->n;
                tail = (os_int)ioc_gets_int(signals->tail, &state_bits);
                if ((state_bits & OSAL_STATE_CONNECTED) == 0 || tail < 0 || tail >= buf_sz)
                {
                    osal_trace3("IOC_SSTEP_FAILED, no tail");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }

                nbytes = osal_streamer_write_internal(signals, buf, buf_sz,
                    (os_int)n, &streamer->head, tail);
                if (nbytes < 0)
                {
                    osal_trace3("IOC_SSTEP_FAILED, buffer write failed");
                    streamer->step = IOC_SSTEP_FAILED;
                    goto getout;
                }

                /* More data to come, break.
                 */
                break;
            }
            if (n == 0) break;

            ioc_sets0_int(signals->state, IOC_STREAM_COMPLETED);
            streamer->step = IOC_SSTEP_TRANSFER_DONE;
            osal_trace3("IOC_SSTEP_TRANSFER_DONE");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DONE:
            if (cmd == IOC_STREAM_RUNNING) break;
            ioc_sets0_int(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            osal_trace3("IOC_SSTEP_ALL_COMPLETED");
            /* continues... */

        case IOC_SSTEP_ALL_COMPLETED:
            break;

        case IOC_SSTEP_FAILED:
            if (cmd == IOC_STREAM_RUNNING) break;
            ioc_sets0_int(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
            osal_trace3("IOC_SSTEP_FAILED_AND_IDLE_SET");
            /* continues... */

        case IOC_SSTEP_FAILED_AND_IDLE_SET:
            break;
    }

getout:
    switch (streamer->step)
    {
        case IOC_SSTEP_TRANSFER_DATA:
        case IOC_SSTEP_TRANSFER_DONE:
        case IOC_SSTEP_FAILED:
            s = OSAL_SUCCESS;
            break;

        case IOC_SSTEP_ALL_COMPLETED:
            s = OSAL_STATUS_COMPLETED;
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
  OSAL_STATUS_COMPLETED or error code. If transfer needs to be interrupted in middle,
  call ioc_streamer_close() function.

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer where to store data.
  @param   n Buffer size, maximum number of bytes to read.
  @param   n_read Pointer to integer into which the function stores the number of bytes
           actually read, which may be less than n if there is no more in incoming buffer.
           If the function fails n_read is set to zero.

  @return  OSAL_SUCCESS if transfer is still running.
           OSAL_STATUS_COMPLETED transnsfer has been completed.
           Other return values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_streamer_device_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read)
{
    iocStreamerState cmd;
    os_char state_bits;
    os_int buf_sz, head, nbytes;
    osalStatus s;

    nbytes = 0;

    cmd = (iocStreamerState)ioc_gets_int(signals->cmd, &state_bits);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        streamer->step = IOC_SSTEP_FAILED;
        goto getout;
    }

    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
            osal_trace3("IOC_SSTEP_INITIALIZED");
            streamer->select = ioc_gets_int(signals->select, &state_bits);
            if ((cmd != IOC_STREAM_RUNNING && cmd != IOC_STREAM_COMPLETED) ||
                (state_bits & OSAL_STATE_CONNECTED) == 0)
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING,COMPLETED or no select");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            streamer->tail = 0;
            ioc_sets0_int(signals->tail, 0);
            ioc_sets0_int(signals->state, IOC_STREAM_RUNNING);
            streamer->step = IOC_SSTEP_TRANSFER_DATA;
            osal_trace3("IOC_SSTEP_TRANSFER_DATA");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DATA:
            if (cmd != IOC_STREAM_RUNNING && cmd != IOC_STREAM_COMPLETED)
            {
                osal_trace3("IOC_SSTEP_FAILED, cmd != RUNNING,COMPLETED");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            buf_sz = signals->buf->n;
            head = (os_int)ioc_gets_int(signals->head, &state_bits);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0 || head < 0 || head >= buf_sz)
            {
                osal_trace3("IOC_SSTEP_FAILED, no head");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            nbytes = osal_streamer_read_internal(signals, buf, buf_sz,
                (os_int)n, head, &streamer->tail);
            if (nbytes < 0)
            {
                osal_trace3("IOC_SSTEP_FAILED, buffer read failed");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            /* If more data to come, break.
             */
            if (cmd == IOC_STREAM_RUNNING)
            {
                break;
            }

            ioc_sets0_int(signals->state, IOC_STREAM_COMPLETED);
            streamer->step = IOC_SSTEP_TRANSFER_DONE;
            osal_trace3("IOC_SSTEP_TRANSFER_DONE");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DONE:
            if (cmd == IOC_STREAM_COMPLETED) break;
            ioc_sets0_int(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            osal_trace3("IOC_SSTEP_ALL_COMPLETED");
            /* continues... */

        case IOC_SSTEP_ALL_COMPLETED:
            break;

        case IOC_SSTEP_FAILED:
            if (cmd == IOC_STREAM_RUNNING || cmd == IOC_STREAM_COMPLETED) break;
            ioc_sets0_int(signals->state, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
            osal_trace3("IOC_SSTEP_FAILED_AND_IDLE_SET");
            /* continues... */

        case IOC_SSTEP_FAILED_AND_IDLE_SET:
            break;
    }

getout:
    switch (streamer->step)
    {
        case IOC_SSTEP_TRANSFER_DATA:
        case IOC_SSTEP_TRANSFER_DONE:
        case IOC_SSTEP_FAILED:
            s = OSAL_SUCCESS;
            break;

        case IOC_SSTEP_ALL_COMPLETED:
            s = OSAL_STATUS_COMPLETED;
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
  Controller can be in IOC_STREAM_IDLE, IOC_STREAM_RUNNING or IOC_STREAM_COMPLETED command "cmd".

  If "cmd" is IOC_STREAM_IDLE:
      If "state" is not IOC_STREAM_IDLE, do nothing. We are waiting for IO device to finish.
      Otherwise if controller wants to start the transfer, set "select" to ? (app spefific),
      "head" = 0 and "cmd" = IOSTREAM_RUNNING

  if "cmd" is IOC_STREAM_RUNNING:
      If "state" is IOC_STREAM_RUNNING: Write as much data to buffer as available and fits
      between tail and head and move head. If all data has been written and "state" is
      IOC_STREAM_RUNNING, set "cmd" to IOC_STREAM_COMPLETED.
      If controller want to interrupt the transfer, it sets "cmd" to IOC_STREAM_IDLE.

  Is cmd is IOC_STREAM_COMPLETED:
      If "state" is IOC_STREAM_IDLE, set cmd = IOC_STREAM_IDLE. All done.

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer to write from.
  @param   n Maximum number of bytes to write.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written, which may be less than n if there is not space (yet) in outgoing
           buffer. If the function fails n_written is set to zero.

  @return  OSAL_SUCCESS to indicate success.

****************************************************************************************************
*/
static osalStatus ioc_streamer_controller_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written)
{
    iocStreamerState
        state,
        cmd;

    os_char
        state_bits;

    os_int
        buf_sz,
        tail,
        nbytes;

    state = (iocStreamerState)ioc_gets_int(signals->state, &state_bits);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0) state = IOC_STREAM_IDLE;

    nbytes = 0;
    cmd = ioc_gets0_int(signals->cmd);
    switch (cmd)
    {
        case IOC_STREAM_INTERRUPT:
            goto switch_to_idle;

        case IOC_STREAM_COMPLETED:
            if (state != IOC_STREAM_RUNNING)
            {
                goto switch_to_idle;
            }
            break;

        case IOC_STREAM_IDLE:
            if (state != IOC_STREAM_IDLE) break;
            streamer->head = 0;
            ioc_sets0_int(signals->select, 115);
            ioc_sets0_int(signals->head, 0);
            ioc_sets0_int(signals->state, IOC_STREAM_RUNNING);
            /* continues ... */

        case IOC_STREAM_RUNNING:
            if (state != IOC_STREAM_RUNNING)
            {
                goto switch_to_idle;
            }

            buf_sz = signals->buf->n;
            tail = (os_int)ioc_gets_int(signals->tail, &state_bits);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0 || tail < 0 || tail >= buf_sz)
            {
                goto switch_to_idle;
            }
            else
            {
                nbytes = osal_streamer_write_internal(signals, buf, buf_sz,
                    (os_int)n, &streamer->head, tail);
            }
            break;
    }

    *n_written = nbytes;
    return OSAL_SUCCESS;

switch_to_idle:
    ioc_sets0_int(signals->cmd, IOC_STREAM_IDLE);
    return OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Receive by controller
  @anchor ioc_streamer_controller_read

  The ioc_streamer_controller_read() function handles controller reveiving data from
  IO device.

  Controller can be in IOC_STREAM_IDLE or IOC_STREAM_RUNNING command (in "cmd")

  If "cmd" is IOSTREAM_IDLE:
      If state is IOSTREAM_RUNNING, do nothing. We are waiting for IO device to finish.
      Otherwise if controller wants to start the transfer, set "select" to ? (app spefific),
      "tail" = 0 and "cmd" = IOSTREAM_RUNNING

  if "cmd" is IOSTREAM_RUNNING:
      if "state" is IOC_STREAM_RUNNING or IOSTREAM_COMPLETED: Read all available data and move "tail".
      If "state" is IOSTREAM_COMPLETED, the transfer has succesfully finished. Set "cmd" = IOSTREAM_IDLE.
      If controller wants to interrupt the transfer, set "cmd" = IOSTREAM_IDLE.

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer where to store data.
  @param   n Buffer size, maximum number of bytes to read.
  @param   n_read Pointer to integer into which the function stores the number of bytes
           actually read, which may be less than n if there is no more in incoming buffer.
           If the function fails n_read is set to zero.

  @return  OSAL_SUCCESS to indicate success.

****************************************************************************************************
*/
static osalStatus ioc_streamer_controller_read(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read)
{
    iocStreamerState state;
    os_char state_bits;
    os_int buf_sz, head, nbytes;
    osalStatus s;

    nbytes = 0;

    state = (iocStreamerState)ioc_gets_int(signals->state, &state_bits);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0)
    {
        streamer->step = IOC_SSTEP_FAILED;
        goto getout;
    }

    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
            streamer->tail = 0;
            ioc_sets0_int(signals->tail, 0);
            ioc_sets0_int(signals->select, streamer->select);
            ioc_sets0_int(signals->cmd, IOC_STREAM_RUNNING);
            if (state != IOC_STREAM_RUNNING && state != IOC_STREAM_COMPLETED) break;
            streamer->step = IOC_SSTEP_TRANSFER_DATA;
            osal_trace3("IOC_SSTEP_TRANSFER_DATA");
            break;

        case IOC_SSTEP_TRANSFER_DATA:
            if (state != IOC_STREAM_RUNNING && state != IOC_STREAM_COMPLETED)
            {
                osal_trace3("IOC_SSTEP_FAILED, state != RUNNING,COMPLETED");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            buf_sz = signals->buf->n;
            head = (os_int)ioc_gets_int(signals->head, &state_bits);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0 || head < 0 || head >= buf_sz)
            {
                osal_trace3("IOC_SSTEP_FAILED, no head");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            nbytes = osal_streamer_read_internal(signals, buf, buf_sz,
                (os_int)n, head, &streamer->tail);
            if (nbytes < 0)
            {
                osal_trace3("IOC_SSTEP_FAILED, buffer read failed");
                streamer->step = IOC_SSTEP_FAILED;
                goto getout;
            }

            /* If more data to come, break.
             */
            if (state == IOC_STREAM_RUNNING)
            {
                break;
            }

            ioc_sets0_int(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            osal_trace3("IOC_SSTEP_TRANSFER_DONE");
            /* continues... */

        case IOC_SSTEP_TRANSFER_DONE:
            // if (cmd == IOC_STREAM_COMPLETED) break;
            ioc_sets0_int(signals->cmd, IOC_STREAM_IDLE);
            streamer->step = IOC_SSTEP_ALL_COMPLETED;
            osal_trace3("IOC_SSTEP_ALL_COMPLETED");
            /* continues... */

        case IOC_SSTEP_ALL_COMPLETED:
            break;

        case IOC_SSTEP_FAILED:
            ioc_sets0_int(signals->cmd, IOC_STREAM_IDLE);
            // if (cmd == IOC_STREAM_RUNNING || cmd == IOC_STREAM_COMPLETED) break;
            streamer->step = IOC_SSTEP_FAILED_AND_IDLE_SET;
            osal_trace3("IOC_SSTEP_FAILED_AND_IDLE_SET");
            /* continues... */

        case IOC_SSTEP_FAILED_AND_IDLE_SET:
            break;
    }

getout:
    switch (streamer->step)
    {
        case IOC_SSTEP_INITIALIZED:
        case IOC_SSTEP_TRANSFER_DATA:
        case IOC_SSTEP_TRANSFER_DONE:
        case IOC_SSTEP_FAILED:
            s = OSAL_SUCCESS;
            break;

        case IOC_SSTEP_ALL_COMPLETED:
            s = OSAL_STATUS_COMPLETED;
            break;

        default:
            s = OSAL_STATUS_FAILED;
            break;
    }
    *n_read = nbytes;
    return s;
}

#endif


static os_int osal_streamer_read_internal(
    iocStreamerSignals *signals,
    os_char *buf,
    os_int buf_sz,
    os_int n,
    os_int head,
    os_int *tail)
{
    os_int nbytes, rdnow;
    os_char state_bits;

    nbytes = 0;

    if (*tail > head)
    {
        rdnow = buf_sz - *tail;
        if (rdnow > n) rdnow = n;
        if (rdnow > 0)
        {
            state_bits = ioc_moves_array(signals->buf, *tail, buf, rdnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0) return -1;

            *tail += rdnow;
            if (*tail >= buf_sz) *tail = 0;

            ioc_sets0_int(signals->tail, *tail);

            buf += rdnow;
            n -= rdnow;
            nbytes += rdnow;
            osal_trace3_int("DATA MOVED 1, bytes = ", rdnow);
        }
    }

    if (*tail < head)
    {
        rdnow = head - *tail;
        if (rdnow > n) rdnow = n;

        if (rdnow > 0)
        {
            state_bits = ioc_moves_array(signals->buf, *tail, buf, rdnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_DEFAULT);
            if ((state_bits & OSAL_STATE_CONNECTED) == 0) return -1;

            *tail += rdnow;
            ioc_sets0_int(signals->tail, *tail);

            nbytes += rdnow;
            osal_trace3_int("DATA MOVED 2, bytes = ", rdnow);
        }
    }

    return nbytes;
}


static os_int osal_streamer_write_internal(
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
            ioc_moves_array(signals->buf, *head, (os_char*)buf, wrnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);

            *head += wrnow;
            if (*head >= buf_sz) *head = 0;

            ioc_sets0_int(signals->head, *head);

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
            ioc_moves_array(signals->buf, *head, (os_char*)buf, wrnow,
                OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);

            *head += wrnow;
            ioc_sets0_int(signals->head, *head);
            nbytes += wrnow;
        }
    }

    return nbytes;
}


/**
****************************************************************************************************

  @brief Get serial port parameter.
  @anchor osal_streamer_get_parameter

  The osal_streamer_get_parameter() function gets a parameter value.

  @param   stream Stream pointer representing the serial.
  @param   parameter_ix Index of parameter to get. Use OSAL_STREAM_TX_AVAILABLE to get
           how much empty space there is in write buffer.
           See @ref osalStreamParameterIx "stream parameter enumeration" for the list.
  @return  Parameter value.

****************************************************************************************************
*/
os_long osal_streamer_get_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix)
{
    iocStreamer *streamer;
    iocStreamerSignals *signals;
    os_int buf_sz, head, tail, space_available, buffered_bytes;
    os_boolean is_device;
    os_char state_bits;

    if (parameter_ix == OSAL_STREAM_TX_AVAILABLE)
    {
        if (stream == OS_NULL) return 0;
        streamer = (iocStreamer*)stream;

        is_device = streamer->prm->is_device;

        signals = is_device ? &streamer->prm->frd : &streamer->prm->tod;

        buf_sz = signals->buf->n;
        tail = (os_int)ioc_gets_int(signals->tail, &state_bits);

        if ((state_bits & OSAL_STATE_CONNECTED) == 0 || tail < 0 || tail >= buf_sz)
        {
            return 0;
        }
        else
        {
            head = streamer->head;

            buffered_bytes = head - tail;
            if (buffered_bytes < 0) buffered_bytes += buf_sz;

            space_available = buf_sz - buffered_bytes - 1;
        }
        return space_available;
    }

    return osal_stream_default_get_parameter(stream, parameter_ix);
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

    ioc_sets0_int(params->frd.state, 0);
    ioc_sets0_int(params->frd.head, 0);
    ioc_sets0_int(params->tod.state, 0);
    ioc_sets0_int(params->tod.tail, 0);

#if OSAL_DEBUG
    ctrl->initialized = 'I';
#endif
}


/**
****************************************************************************************************

  @brief Keep control stream alive.
  @anchor ioc_run_control_stream

  Streamer for transferring IO device configuration and flash program. The streamer is used
  to transfer a stream using buffer within memory block. This static structure selects which
  signals are used for straming data between the controller and IO device.

  This function must be called from one thread at a time.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
void ioc_run_control_stream(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    iocStreamerState cmd;
    osPersistentBlockNr select;
    os_char state_bits;

    /* Just for debugging, assert here that ioc_init_control_stream() has been called.
     */
    osal_debug_assert(ctrl->initialized == 'I');

    if (ctrl->frd == OS_NULL)
    {
        cmd = (iocStreamerState)ioc_gets_int(params->frd.cmd, &state_bits);
        if (cmd == IOC_STREAM_RUNNING && (state_bits & OSAL_STATE_CONNECTED))
        {
            osal_trace3("IOC_STREAM_RUNNING command");
            ctrl->frd = ioc_streamer_open(OS_NULL, params, OS_NULL, OSAL_STREAM_WRITE);

            if (ctrl->frd)
            {
                select = (osPersistentBlockNr)ioc_gets0_int(params->frd.select);
                ctrl->fdr_persistent = os_persistent_open(select, OSAL_STREAM_READ);
            }
        }
    }

    if (ctrl->frd)
    {
        ioc_ctrl_stream_from_device(ctrl, params);
    }

    if (ctrl->tod  == OS_NULL)
    {
        cmd = (iocStreamerState)ioc_gets_int(params->tod.cmd, &state_bits);
        if (cmd == IOC_STREAM_RUNNING && (state_bits & OSAL_STATE_CONNECTED))
        {
            ctrl->tod = ioc_streamer_open(OS_NULL, params, OS_NULL, OSAL_STREAM_READ);

            if (ctrl->tod)
            {
                select = (osPersistentBlockNr)ioc_gets0_int(params->frd.select);
                ctrl->tod_persistent = os_persistent_open(select, OSAL_STREAM_WRITE);
            }
        }
    }

    if (ctrl->tod)
    {
        ioc_ctrl_stream_to_device(ctrl, params);
    }
}


/**
****************************************************************************************************

  @brief Move data from controller to IO device.
  @anchor ioc_ctrl_stream_from_device

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
void ioc_ctrl_stream_from_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    os_char buf[256];
    os_memsz rdnow, n_written, n_read, bytes;
    osalStatus s;

    if (ctrl->fdr_persistent)
    {
        bytes = osal_streamer_get_parameter(ctrl->frd, OSAL_STREAM_TX_AVAILABLE);
        while (OS_TRUE)
        {
            if (bytes <= 0) return;

            rdnow = bytes;
            if (rdnow > sizeof(buf)) rdnow = sizeof(buf);
            n_read = os_persistent_read(ctrl->fdr_persistent, buf, rdnow);
            if (n_read > 0)
            {
                s = ioc_streamer_write(ctrl->frd, buf, n_read, &n_written, OSAL_STREAM_DEFAULT);
                if (s) break;
                osal_debug_assert(n_written == n_read);
            }

            /* If everythin has been read.
             */
            if (n_read < rdnow) break;
            bytes -= n_read;
        }

        os_persistent_close(ctrl->fdr_persistent, 0);
        ctrl->fdr_persistent = OS_NULL;
    }

    /* Finalize any handshaking signal stuff.
     */
    s = ioc_streamer_flush(ctrl->frd, OSAL_STREAM_FINAL_HANDSHAKE);
    if (s == OSAL_STATUS_PENDING) return;

    /* Close the stream
     */
    ioc_streamer_close(ctrl->frd, OSAL_STREAM_DEFAULT);
    ctrl->frd = OS_NULL;
}


/**
****************************************************************************************************

  @brief Move data from controller to IO device.
  @anchor ioc_ctrl_stream_to_device

  Called repeatedly to run data transfer from controller IO device. The function reads data from
  the stream buffer in memory block (as much as there is) and writes it to persistent storage.
  If the function detects IOC_STREAM_COMPLETED or IOC_STREAM_INTERRUPTED command, or if
  connection has broken, it closes the persistent storage and memory block streamer.
  Closing persistent object is flagged with success OSAL_STREAM_DEFAULT only on
  IOC_STREAM_COMPLETED command, otherwise persistent object is closed with OSAL_STREAM_INTERRUPT
  flag (in this case persient object may not want to use newly received data, especially if
  it is flash program for micro-controller.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
void ioc_ctrl_stream_to_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    os_char buf[256];
    os_memsz n_read;
    osalStatus s;

    do
    {
        s = ioc_streamer_read(ctrl->tod, buf, sizeof(buf), &n_read, OSAL_STREAM_DEFAULT);
        os_persistent_write(ctrl->tod_persistent, buf, n_read);
    }
    while (s == OSAL_SUCCESS);

    os_persistent_close(ctrl->tod_persistent, s == OSAL_STATUS_COMPLETED
        ? OSAL_STREAM_DEFAULT : OSAL_STREAM_INTERRUPT);

    ctrl->tod_persistent = OS_NULL;

    ioc_streamer_close(ctrl->tod, OSAL_STREAM_DEFAULT);
    ctrl->tod = OS_NULL;
}


/** Stream interface for OSAL streamers. This is structure is filled with
    function pointers to memory block streamer implementation.
 */
const osalStreamInterface ioc_streamer_iface
 = {ioc_streamer_open,
    ioc_streamer_close,
    osal_stream_default_accept,
    ioc_streamer_flush,
    osal_stream_default_seek,
    ioc_streamer_write,
    ioc_streamer_read,
    osal_stream_default_write_value,
    osal_stream_default_read_value,
    osal_streamer_get_parameter,
    osal_stream_default_set_parameter,
    osal_stream_default_select};

#endif
