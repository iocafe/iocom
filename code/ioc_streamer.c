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


/**
****************************************************************************************************

  @brief Open a streamer port.
  @anchor osal_streamer_open

  The osal_streamer_open() function opens a streamer port.

  Example:
    osalStream handle;
    handle = osal_streamer_open("ttyS30,baud=115200", OS_NULL, OS_NULL, OSAL_STREAM_SELECT);
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

  @return Stream pointer representing the streamer port, or OS_NULL if the function failed.

****************************************************************************************************
*/
osalStream osal_streamer_open(
    const os_char *parameters,
    void *option,
    osalStatus *status,
    os_int flags)
{


    /* Set status code and return NULL pointer.
     */
    if (status) *status = OSAL_STATUS_FAILED;
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Close streamer port.
  @anchor osal_streamer_close

  The osal_streamer_close() function closes a streamer port, earlier opened by the osal_streamer_open()
  function. All resource related to the streamer port are freed. Any attemp to use the streamer after
  this call may result in crash.

  @param   stream Stream pointer representing the streamer port. After this call stream
           pointer will point to invalid memory location.
  @return  None.

****************************************************************************************************
*/
void osal_streamer_close(
    osalStream stream)
{
    iocStreamer *mystreamer;

    /* If called with NULL argument, do nothing.
     */
    if (stream == OS_NULL) return;

    /* Cast stream pointer to streamer structure pointer. The osal_debug_assert here is used
       to detect access to already closed stream while debugging.
     */
    mystreamer = (iocStreamer*)stream;
    osal_debug_assert(mystreamer->hdr.iface == &osal_streamer_iface);

    /* Close the streamer port.
     */

#if OSAL_DEBUG
    mystreamer->hdr.iface = 0;
#endif

    /* Free memory allocated for the streamer port structure.
     */
    os_free(mystreamer, sizeof(iocStreamer));
}


/**
****************************************************************************************************

  @brief Flush data to the stream.
  @anchor osal_streamer_flush

  Some implementations of the osal_streamer_flush() function flushes data to be written to stream
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
osalStatus osal_streamer_flush(
    osalStream stream,
    os_int flags)
{
    iocStreamer *mystreamer;

    /* If called with NULL argument, do nothing.
     */
    if (stream == OS_NULL) return OSAL_STATUS_FAILED;

    /* Cast stream type to streamer structure pointer, get operating system's streamer port handle.
     */
    mystreamer = (iocStreamer*)stream;
    osal_debug_assert(mystreamer->hdr.iface == &osal_streamer_iface);

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
  @anchor osal_streamer_write

  The osal_streamer_write() function writes up to n bytes of data from buffer to streamer port.

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
osalStatus osal_streamer_write(
    osalStream stream,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags)
{
    iocStreamer *mystreamer;

    if (stream)
    {
        /* Cast stream type to streamer structure pointer, get operating system's streamer port handle.
         */
        mystreamer = (iocStreamer*)stream;
        osal_debug_assert(mystreamer->hdr.iface == &osal_streamer_iface);

        /* If operating system streamer is already closed.
         */
        if (buf == OS_NULL || n < 0)
        {
            goto getout;
        }

        /* Write data to streamer port. Notice that linux handles the case when n is zero.
           If write to streamer port returns an error, we do not really care, we treat it
           as zero bytes written.
         */
        //rval = write(handle, buf, (int)n);
        // if (rval < 0) rval = 0;

        /* Success, set number of bytes written.
         */
        *n_written = 0 ; // rval;
        return OSAL_SUCCESS;
    }

getout:
    *n_written = 0;
    return OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Read data from streamer port.
  @anchor osal_streamer_read

  The osal_streamer_read() function reads up to n bytes of data from streamer port into buffer.

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
osalStatus osal_streamer_read(
    osalStream stream,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags)
{
    iocStreamer *mystreamer;

    if (stream)
    {
        /* Cast stream type to streamer structure pointer, get operating system's streamer
           port handle, check function argument.
         */
        mystreamer = (iocStreamer*)stream;
        osal_debug_assert(mystreamer->hdr.iface == &osal_streamer_iface);
//        handle = mystreamer->handle;
        if (buf == OS_NULL || n < 0) goto getout;

        /* Read from streamer port. Notice that linux handles the case when n is zero.
           If read from streamer port returns an error, we do not really care, we treat
           is as zero bytes received.
         */
 //       rval = read(handle, buf, (int)n);
  //      if (rval < 0) rval = 0;

        /* Success, set number of bytes read.
         */
        *n_read = 0; // rval;
        return OSAL_SUCCESS;
    }

getout:
    *n_read = 0;
    return OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Get streamer port parameter.
  @anchor osal_streamer_get_parameter

  The osal_streamer_get_parameter() function gets a parameter value. Here we just call the default
  implementation for streams.

  @param   stream Stream pointer representing the streamer.
  @param   parameter_ix Index of parameter to get.
           See @ref osalStreamParameterIx "stream parameter enumeration" for the list.
  @return  Parameter value.

****************************************************************************************************
*/
os_long osal_streamer_get_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix)
{
    return osal_stream_default_get_parameter(stream, parameter_ix);
}


/**
****************************************************************************************************

  @brief Set streamer port parameter.
  @anchor osal_streamer_set_parameter

  The osal_streamer_set_parameter() function gets a parameter value. Here we just call the default
  implementation for streams.

  @param   stream Stream pointer representing the streamer.
  @param   parameter_ix Index of parameter to get.
           See @ref osalStreamParameterIx "stream parameter enumeration" for the list.
  @param   value Parameter value to set.
  @return  None.

****************************************************************************************************
*/
void osal_streamer_set_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix,
    os_long value)
{
    osal_stream_default_set_parameter(stream, parameter_ix, value);
}


#if OSAL_STREAMER_SELECT_SUPPORT
/**
****************************************************************************************************

  @brief Wait for an event from one of streamers ports and for custom event.
  @anchor osal_streamer_select

  The osal_streamer_select() function blocks execution of the calling thread until something
  data is received from streamer port, a last write has not been fully finished and it can
  be continued now, or a custom event occurs.

  Custom event and interrupting select: A pipe is generated for the event, and the select
  here monitors the pipe. When some other thread wants to interrupt the select() it
  calls oepal_set_event(), which write a byte to this pipe.

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
osalStatus osal_streamer_select(
    osalStream *streams,
    os_int nstreams,
    osalEvent evnt,
    osalSelectData *selectdata,
    os_int timeout_ms,
    os_int flags)
{
    iocStreamer *mystreamer;
    fd_set rdset, wrset;
    os_int i, handle, streamer_nr, eventflags, errorcode, maxfd, pipefd, rval;
    struct timespec timeout, *to;

    os_memclear(selectdata, sizeof(osalSelectData));

    FD_ZERO(&rdset);
    FD_ZERO(&wrset);

    maxfd = 0;
    for (i = 0; i < nstreams; i++)
    {
        mystreamer = (iocStreamer*)streams[i];
        if (mystreamer)
        {
            osal_debug_assert(mystreamer->hdr.iface == &osal_streamer_iface);
            handle = mystreamer->handle;

            FD_SET(handle, &rdset);
            if (mystreamer->write_blocked)
            {
                FD_SET(handle, &wrset);
            }
            if (handle > maxfd) maxfd = handle;
        }
    }

    pipefd = -1;
    if (evnt)
    {
        pipefd = osal_event_pipefd(evnt);
        if (pipefd > maxfd) maxfd = pipefd;
        FD_SET(pipefd, &rdset);
    }

    to = NULL;
    if (timeout_ms)
    {
        timeout.tv_sec = (time_t)(timeout_ms / 1000);
        timeout.tv_nsec	= (long)((timeout_ms % 1000) * 1000000);
        to = &timeout;
    }

    errorcode = OSAL_SUCCESS;
    rval = pselect(maxfd+1, &rdset, &wrset, NULL, to, NULL);
    if (rval <= 0)
    {
        if (rval == 0)
        {
            selectdata->eventflags = OSAL_STREAM_TIMEOUT_EVENT;
            selectdata->stream_nr = OSAL_STREAM_NR_TIMEOUT_EVENT;
            return OSAL_SUCCESS;
        }
        errorcode = OSAL_STATUS_FAILED;
    }

    if (pipefd >= 0) if (FD_ISSET(pipefd, &rdset))
    {
        osal_event_clearpipe(evnt);

        selectdata->eventflags = OSAL_STREAM_CUSTOM_EVENT;
        selectdata->stream_nr = OSAL_STREAM_NR_CUSTOM_EVENT;
        return OSAL_SUCCESS;
    }

    eventflags = OSAL_STREAM_UNKNOWN_EVENT;

    for (streamer_nr = 0; streamer_nr < nstreams; streamer_nr++)
    {
        mystreamer = (iocStreamer*)streams[streamer_nr];
        if (mystreamer)
        {
            handle = mystreamer->handle;

            if (FD_ISSET (handle, &rdset))
            {
                eventflags = OSAL_STREAM_READ_EVENT;
                break;
            }

            if (mystreamer->write_blocked)
            {
                if (FD_ISSET (handle, &wrset))
                {
                    eventflags = OSAL_STREAM_WRITE_EVENT;
                    break;
                }
            }
        }
    }

    if (streamer_nr == nstreams)
    {
        streamer_nr = OSAL_STREAM_NR_UNKNOWN_EVENT;
    }

    selectdata->eventflags = eventflags;
    selectdata->stream_nr = streamer_nr;
    selectdata->errorcode = errorcode;

    return OSAL_SUCCESS;
}
#endif




/**
****************************************************************************************************

  @brief Initialize streamer communication.
  @anchor osal_streamer_initialize

  The osal_streamer_initialize() initializes the underlying streamer communication library.
  This is not needed for linux, just empty function to allow linking with code which
  calls this function for some other OS.

  @return  None.

****************************************************************************************************
*/
void osal_streamer_initialize(
    void)
{
}


/**
****************************************************************************************************

  @brief Shut down the streamer communication.
  @anchor osal_streamer_shutdown

  The osal_streamer_shutdown() shuts down the underlying streamer communication library.
  This is not needed for linux, just empty function to allow linking with code which
  calls this function for some other OS.

  @return  None.

****************************************************************************************************
*/
void osal_streamer_shutdown(
    void)
{
}



/** Stream interface for OSAL streamers. This is structure osalStreamInterface filled with
    function pointers to OSAL streamers implementation.
 */
const osalStreamInterface osal_streamer_iface
 = {osal_streamer_open,
    osal_streamer_close,
    osal_stream_default_accept,
    osal_streamer_flush,
    osal_stream_default_seek,
    osal_streamer_write,
    osal_streamer_read,
    osal_stream_default_write_value,
    osal_stream_default_read_value,
    osal_streamer_get_parameter,
    osal_streamer_set_parameter,
#if OSAL_STREAMER_SELECT_SUPPORT
    osal_streamer_select};
#else
    osal_stream_default_select};
#endif

