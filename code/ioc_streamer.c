/**

  @file    ioc_streamer.c
  @brief   Data stream trough memory block API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.12.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

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


/**
****************************************************************************************************

  @brief Open a streamer port.
  @anchor ioc_streamer_open

  The ioc_streamer_open() function opens a streamer port.

  Example:
    osalStream handle;
    handle = ioc_streamer_open("ttyS30,baud=115200", OS_NULL, OS_NULL, OSAL_STREAM_SELECT);
    if (handle == OS_NULL)
    {
        osal_debug_error("Unable to open streamer port");
        ...
    }

  @param  parameters Not used.
  @param  option Pointer to streamer set parameter up structure.
  @param  status Pointer to integer into which to store the function status code. Value
          OSAL_SUCCESS (0) indicates success and all nonzero values indicate an error.
          This parameter can be OS_NULL, if no status code is needed.

  @param  flags Flags for creating the streamer. Bit fields, combination of:
          - OSAL_STREAM_NO_SELECT: Open streamer without select functionality.
          - OSAL_STREAM_SELECT: Open streamer with select functionality (default).

          At the moment, select support flag has no impact on Linux. If define ??????????????????????????????????????????????????????????????
          OSAL_STREAMER_SELECT_SUPPORT is 1 and select is called, it works. Anyhow flag should
          be set correctly for compatibility with other operating systems. If there are
          flags which are unknown to this function, these are simply ignored.
          See @ref osalStreamFlags "Flags for Stream Functions" for full list of stream flags.

  @return Stream pointer representing the streamer port, or OS_NULL if memory allocation failed.

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
    streamer->prm = *(iocStreamerParams*)option;
    streamer->hdr.iface = &ioc_streamer_iface;
    streamer->used = OS_TRUE;

    /* Set status code and return stream pointer.
     */
getout:
    if (status) *status = streamer ? OSAL_SUCCESS : OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    return (osalStream)streamer;
}


/**
****************************************************************************************************

  @brief Close streamer port.
  @anchor ioc_streamer_close

  The ioc_streamer_close() function closes a streamer port, earlier opened by the ioc_streamer_open()
  function. All resource related to the streamer port are freed. Any attemp to use the streamer after
  this call may result in crash.

  @param   stream Stream pointer representing the streamer port. After this call stream
           pointer will point to invalid memory location.
  @return  None.

****************************************************************************************************
*/
void ioc_streamer_close(
    osalStream stream)
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

  @brief Flush data to the stream.
  @anchor ioc_streamer_flush

  Some implementations of the ioc_streamer_flush() function flushes data to be written to stream
  or clear the transmit/receive buffers. The Linux implementation can clear RX and TX buffers.

  IMPORTANT, GENERALLY FLUSH MUST BE CALLED: The osal_stream_flush(<stream>, OSAL_STREAM_DEFAULT)
  must be called when select call returns even after writing or even if nothing was written, or
  periodically in in single thread mode. This is necessary even if no data was written
  previously, the stream may have stored buffered data to avoid blocking. This is not necessary
  for every stream implementation, but call it anyhow for code portability.

  @param   stream Stream pointer representing the streamer port.
  @param   flags Bit fields. OSAL_STREAM_CLEAR_RECEIVE_BUFFER clears receive
           buffer and OSAL_STREAM_CLEAR_TRANSMIT_BUFFER transmit buffer.
           See @ref osalStreamFlags "Flags for Stream Functions" for full list of flags.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

****************************************************************************************************
*/
osalStatus ioc_streamer_flush(
    osalStream stream,
    os_int flags)
{
    iocStreamer *streamer;

    /* If called with NULL argument, do nothing.
     */
    if (stream == OS_NULL) return OSAL_STATUS_FAILED;

    /* Cast stream type to streamer structure pointer, get operating system's streamer port handle.
     */
    streamer = (iocStreamer*)stream;
    osal_debug_assert(streamer->hdr.iface == &ioc_streamer_iface);

    if (flags & (OSAL_STREAM_CLEAR_RECEIVE_BUFFER|OSAL_STREAM_CLEAR_TRANSMIT_BUFFER))
    {
        switch (flags & (OSAL_STREAM_CLEAR_RECEIVE_BUFFER|OSAL_STREAM_CLEAR_TRANSMIT_BUFFER))
        {
            /* Flushes data received but not read.
             */
            case OSAL_STREAM_CLEAR_RECEIVE_BUFFER:
                break;

            /* Flushes data written but not transmitted.
             */
            case OSAL_STREAM_CLEAR_TRANSMIT_BUFFER:
                break;

            /* Flushes both data received but not read, and data written but not transmitted.
             */
            default:
                break;
        }
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Write data to streamer port.
  @anchor ioc_streamer_write

  The ioc_streamer_write() function writes up to n bytes of data from buffer to streamer port.

  @param   stream Stream pointer representing the streamer port.
  @param   buf Pointer to the beginning of data to place into the streamer port.
  @param   n Maximum number of bytes to write.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written to streamer port, which may be less than n if there is not enough space
           left in write buffer. If the function fails n_written is set to zero.
  @param   flags Flags for the function, ignored. Set OSAL_STREAM_DEFAULT (0).
           See @ref osalStreamFlags "Flags for Stream Functions" for full list of flags.

  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

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
    if (stream == OS_NULL || buf == OS_NULL || n < 0) return OSAL_STATUS_FAILED;

    /* Cast stream type to streamer structure pointer, get operating system's streamer port handle.
     */
    streamer = (iocStreamer*)stream;
    osal_debug_assert(streamer->hdr.iface == &ioc_streamer_iface);

    /* Move data
     */
    if (streamer->prm.is_device)
    {
        s = ioc_streamer_device_write(streamer, &streamer->prm.frd, buf, n, n_written);
    }
    else
    {
        // s = ioc_streamer_controller_write(streamer, streamer->prm.tod, buf, n, n_written);
    }

    /* Return success/failure code.
     */
    return s;
}


/**
****************************************************************************************************

  @brief Read data from streamer port.
  @anchor ioc_streamer_read

  The ioc_streamer_read() function reads up to n bytes of data from streamer port into buffer.

  @param   stream Stream pointer representing the streamer port.
  @param   buf Pointer to buffer to read into.
  @param   n Maximum number of bytes to read. The data buffer must large enough to hold
           at least this many bytes.
  @param   n_read Pointer to integer into which the function stores the number of bytes read,
           which may be less than n if there are fewer bytes available. If the function fails
           n_read is set to zero.
  @param   flags Flags for the function, ignored. Set OSAL_STREAM_DEFAULT (0).
           See @ref osalStreamFlags "Flags for Stream Functions" for full list of flags.

  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

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

    *n_read = 0;
    if (stream == OS_NULL || buf == OS_NULL || n < 0) return OSAL_STATUS_FAILED;

    /* Cast stream type to streamer structure pointer, get operating system's streamer
       port handle, check function argument.
     */
    streamer = (iocStreamer*)stream;
    osal_debug_assert(streamer->hdr.iface == &ioc_streamer_iface);

    /* Success, set number of bytes read.
     */
    *n_read = 0; // rval;
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Transfer from the IO device
  @anchor ioc_streamer_device_write

  The ioc_streamer_device_write() function implements IO device writing data to stream.
  Stream transfer has three states, IOC_STREAM_IDLE, IOC_STREAM_RUNNING and IOSTREAM_COMPLETED.
  This is in "state" signal.

  If in IOC_STREAM_COMPLETED state:
      If "cmd" is not IOC_STREAM_RUNNING, set "state" = IOSTREAM_IDLE.

  If in IOC_STREAM_IDLE state:
      If "cmd" is not IOC_STREAM_RUNNING, do nothing
      Otherwise set "head" to zero and switch to "state" = IOC_STREAM_RUNNING

  If in IOC_STREAM_RUNNING state:
    If "cmd" is no longer IOSTREAM_RUNNING, interrupt the transfer and switch to "state"
    = IOC_STREAM_IDLE. Same if "cmd" signal is disconnected?
    Otherwise move as much data as we can to head position. Limits are how much data we have
    and how much space there is for data between tail and head. Update head position.

    "select" can choose what to transfer???
    If all data has been transferred, switch to "state" = IOC_STREAM_COMPLETED (this is done
    by close()).

  @param   streamer Pointer to streamer structure.
  @param   signals Pointer to signals structure. These are signals used for the transfer.
  @param   buf Pointer to buffer to write from.
  @param   n Maximum number of bytes to write.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written, which may be less than n if there is not space (yet) in outgoing buffer.
           If the function fails n_written is set to zero.

  @return  OSAL_SUCCESS to indicate success.

****************************************************************************************************
*/
static osalStatus ioc_streamer_device_write(
    iocStreamer *streamer,
    iocStreamerSignals *signals,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written)
{
    iocStreamerState
        cmd;

    os_char
        state_bits;

    os_int
        buf_sz,
        head,
        tail,
        wrnow,
        nbytes;

    cmd = (iocStreamerState)ioc_gets_int(signals->cmd, &state_bits);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0) cmd = IOC_STREAM_IDLE;

    nbytes = 0;
    switch (streamer->frd_state)
    {
        case IOC_STREAM_COMPLETED:
            if (cmd != IOC_STREAM_RUNNING)
            {
                goto switch_to_idle;
            }
            break;

        case IOC_STREAM_IDLE:
            if (cmd != IOC_STREAM_RUNNING) break;

            streamer->frd_head = 0;
            ioc_sets_int0(signals->head, 0);
            streamer->frd_state = IOC_STREAM_RUNNING;
            ioc_sets_int0(signals->state, IOC_STREAM_RUNNING);
            /* continues ... */

        case IOC_STREAM_RUNNING:
            if (cmd != IOC_STREAM_RUNNING)
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
                head = streamer->frd_head;

                if (head >= tail)
                {
                    wrnow = buf_sz - head;
                    if (!tail) wrnow--;
                    if (wrnow > n) wrnow = n;
                    if (wrnow > 0)
                    {
                        ioc_moves_array(signals->buf, head, (os_char*)buf, wrnow,
                            OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);

                        head += wrnow;
                        if (head >= buf_sz) head = 0;

                        streamer->frd_head = head;
                        ioc_sets_int0(signals->head, head);

                        buf += wrnow;
                        n -= wrnow;
                        nbytes += wrnow;
                    }
                }

                if (head < tail)
                {
                    wrnow = tail - head - 1;
                    if (wrnow > n) wrnow = n;

                    if (wrnow > 0)
                    {
                        ioc_moves_array(signals->buf, head, (os_char*)buf, wrnow,
                            OSAL_STATE_CONNECTED, IOC_SIGNAL_WRITE);

                        head += wrnow;
                        streamer->frd_head = head;
                        ioc_sets_int0(signals->head, head);

                        buf += wrnow;
                        n -= wrnow;
                        nbytes += wrnow;
                    }
                }
            }
            break;
    }

    *n_written = nbytes;
    return OSAL_SUCCESS;

switch_to_idle:
    streamer->frd_state = IOC_STREAM_IDLE;
    ioc_sets_int0(signals->state, IOC_STREAM_IDLE);
    return OSAL_STATUS_FAILED;
}


#if OSAL_STREAMER_SELECT_SUPPORT
/**
****************************************************************************************************

  @brief Wait for an event from one of streamers ports and for custom event.
  @anchor ioc_streamer_select

  The ioc_streamer_select() function blocks execution of the calling thread until something
  data is received from streamer port, a last write has not been fully finished and it can
  be continued now, or a custom event occurs.

  @param   streams Array of streams to wait for. These must be streamer ports, no mixing
           of different stream types is supported.
  @param   n_streams Number of stream pointers in "streams" array.
  @param   evnt Custom event to interrupt the select. OS_NULL if not needed.
  @param   selectdata Pointer to structure to fill in with information why select call
           returned. The "stream_nr" member is stream number which triggered the return,
           or OSAL_STREAM_NR_CUSTOM_EVENT if return was triggered by custom evenet given
           as argument. The "errorcode" member is OSAL_SUCCESS if all is fine. Other
           values indicate an error (broken or closed socket, etc). The "eventflags"
           member is planned to to show reason for return. So far value of eventflags
           is not well defined and is different for different operating systems, so
           it should not be relied on.
  @param   timeout_ms Maximum time to wait in select, ms. If zero, timeout is not used.
  @param   flags Ignored, set OSAL_STREAM_DEFAULT (0).

  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

****************************************************************************************************
*/
osalStatus ioc_streamer_select(
    osalStream *streams,
    os_int nstreams,
    osalEvent evnt,
    osalSelectData *selectdata,
    os_int timeout_ms,
    os_int flags)
{
    // IS SELECT FUNCTIONALITY NEEDED FOR STREAMERS? IT IS POSSIBLE, BUT WILL IT HAVE ANY USE?
    return OSAL_SUCCESS;
}
#endif


/**
****************************************************************************************************

  @brief Initialize streamer data structure.
  @anchor ioc_streamer_initialize

  The ioc_streamer_initialize() clears static memory allocated for streamers. This is needed
  in some microcontroller when RAM is not cleared in soft rebooths.

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


/** Stream interface for OSAL streamers. This is structure osalStreamInterface filled with
    function pointers to OSAL streamers implementation.
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
    osal_stream_default_get_parameter,
    osal_stream_default_set_parameter,
#if OSAL_STREAMER_SELECT_SUPPORT
    ioc_streamer_select};
#else
    osal_stream_default_select};
#endif

