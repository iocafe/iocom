/**

  @file    ioc_switchbox_socket.c
  @brief   Stream class to route an IO sevice end point to switchbox cloud server.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE && OSAL_TLS_SUPPORT


/** Linux specific socket data structure. OSAL functions cast their own stream structure
    pointers to osalStream pointers.
 */
typedef struct osalSwitchboxSocket
{
    /** A stream structure must start with this generic stream header structure, which contains
        parameters common to every stream.
     */
    osalStreamHeader hdr;

    /** Operating system's socket handle.
     */
    osalStream switchbox_stream;

    /** Stream open flags. Flags which were given to ioc_switchbox_socket_open() or ioc_switchbox_socket_accept()
        function.
     */
    os_int open_flags;

    /** Ring buffer, OS_NULL if not used.
     */
    os_char *buf;

    /** Buffer size in bytes.
     */
    os_int buf_sz;

    /** Head index. Position in buffer to which next byte is to be written. Range 0 ... buf_sz-1.
     */
    os_int head;

    /** Tail index. Position in buffer from which next byte is to be read. Range 0 ... buf_sz-1.
     */
    os_int tail;

    /** Linked list of swichbox socket objects sharing one TLS switchbox connection.
     */
    struct osalSwitchboxSocket *next, *prev;
}
osalSwitchboxSocket;


/* Prototypes for forward referred static functions.
 */
static osalStatus ioc_switchbox_socket_write2(
    osalSwitchboxSocket *thiso,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags);

static osalStatus ioc_switchbox_socket_setup_ring_buffer(
    osalSwitchboxSocket *thiso);


/**
****************************************************************************************************

  @brief Open a socket.
  @anchor ioc_switchbox_socket_open

  The ioc_switchbox_socket_open() function opens a socket. The socket can be either listening TCP
  socket, connecting TCP socket or UDP multicast socket.

  @param  parameters SwitchboxSocket parameters, a list string or direct value.
          Address and port to connect to, or interface and port to listen for.
          SwitchboxSocket IP address and port can be specified either as value of "addr" item
          or directly in parameter sstring. For example "192.168.1.55:20" or "localhost:12345"
          specify IPv4 addressed. If only port number is specified, which is often
          useful for listening socket, for example ":12345".
          IPv4 address is automatically recognized from numeric address like
          "2001:0db8:85a3:0000:0000:8a2e:0370:7334", but not when address is specified as string
          nor for empty IP specifying only port to listen. Use brackets around IP address
          to mark IPv6 address, for example "[localhost]:12345", or "[]:12345" for empty IP.

  @param  option Not used for sockets, set OS_NULL.

  @param  status Pointer to integer into which to store the function status code. Value
          OSAL_SUCCESS (0) indicates success and all nonzero values indicate an error.
          See @ref osalStatus "OSAL function return codes" for full list.
          This parameter can be OS_NULL, if no status code is needed.

  @param  flags Flags for creating the socket. Bit fields, combination of:
          - OSAL_STREAM_CONNECT: Connect to specified socket port at specified IP address.
          - OSAL_STREAM_LISTEN: Open a socket to listen for incoming connections.
          - OSAL_STREAM_MULTICAST: Open a UDP multicast socket. Can be combined
            with OSAL_STREAM_LISTEN to listen for multicasts.
          - OSAL_STREAM_NO_SELECT: Open socket without select functionality.
          - OSAL_STREAM_SELECT: Open serial with select functionality.
          - OSAL_STREAM_TCP_NODELAY: Disable Nagle's algorithm on TCP socket. If this flag is set,
            ioc_switchbox_socket_flush() must be called to actually transfer data.
          - OSAL_STREAM_NO_REUSEADDR: Disable reusability of the socket descriptor.

          See @ref osalStreamFlags "Flags for Stream Functions" for full list of stream flags.

  @return Stream pointer representing the socket, or OS_NULL if the function failed.

****************************************************************************************************
*/
static osalStream ioc_switchbox_socket_open(
    const os_char *parameters,
    void *option,
    osalStatus *status,
    os_int flags)
{
    osalSwitchboxSocket *thiso = OS_NULL;
    osalStream switchbox_stream;

    /* Open shared connection.
     */
    switchbox_stream = osal_stream_open(OSAL_TLS_IFACE, parameters, option, status, flags);
    if (switchbox_stream == OS_NULL) {
        return OS_NULL;
    }

    /* Allocate and clear stream structure.
     */
    thiso = (osalSwitchboxSocket*)os_malloc(sizeof(osalSwitchboxSocket), OS_NULL);
    if (thiso == OS_NULL)
    {
        if (status) {
            *status = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        }
        osal_stream_close(switchbox_stream, OSAL_STREAM_DEFAULT);
        return OS_NULL;
    }
    os_memclear(thiso, sizeof(osalSwitchboxSocket));

    /* Save flags, interface pointer and steam.
     */
    thiso->open_flags = flags;
    thiso->hdr.iface = &ioc_switchbox_socket_iface;
    thiso->switchbox_stream = switchbox_stream;

    /* Success.
     */
    if (status) *status = OSAL_SUCCESS;
    return (osalStream)thiso;
}


/**
****************************************************************************************************

  @brief Close socket.
  @anchor ioc_switchbox_socket_close

  The ioc_switchbox_socket_close() function closes a socket, which was opened by ioc_switchbox_socket_open()
  or osal_stream_accept() function. All resource related to the socket are freed. Any attemp
  to use the socket after this call may result in crash.

  @param   stream Stream pointer representing the socket. After this call stream pointer will
           point to invalid memory location.
  @param   flags Reserver, set OSAL_STREAM_DEFAULT (0) for now.
  @return  None.

****************************************************************************************************
*/
static void ioc_switchbox_socket_close(
    osalStream stream,
    os_int flags)
{
    osalSwitchboxSocket *thiso;
    OSAL_UNUSED(flags);

    /* If called with NULL argument, do nothing.
     */
    if (stream == OS_NULL) return;

    /* Cast stream pointer.
     */
    thiso = (osalSwitchboxSocket*)stream;
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);

    /* Send disconnect message.
     */

    /* Detach form chain, synchronization necessary.
     */
    os_lock();
    if (thiso->next) {
        thiso->next->prev = thiso->prev;
    }
    if (thiso->prev) {
        thiso->prev->next = thiso->next;
    }
    thiso->next = thiso->prev = OS_NULL;
    os_unlock();


    if (thiso->switchbox_stream) {
        osal_stream_close(thiso->switchbox_stream, OSAL_STREAM_DEFAULT);
        thiso->switchbox_stream = OS_NULL;
    }
    thiso->hdr.iface = OS_NULL;

    /* Free ring buffer, if any, memory allocated for socket structure
       and decrement socket count.
     */
    os_free(thiso->buf, thiso->buf_sz);
    os_free(thiso, sizeof(osalSwitchboxSocket));
}


/**
****************************************************************************************************

  @brief Accept connection to listening socket.
  @anchor ioc_switchbox_socket_accept

  The ioc_switchbox_socket_accept() function accepts an incoming connection from listening socket.

  @param   stream Stream pointer representing the listening socket.
  @param   remote_ip_address Pointer to string buffer into which to store the IP address
           from which the incoming connection was accepted. Can be OS_NULL if not needed.
  @param   remote_ip_addr_sz Size of remote IP address buffer in bytes.
  @param   status Pointer to integer into which to store the function status code. Value
           OSAL_SUCCESS (0) indicates that new connection was successfully accepted.
           The value OSAL_NO_NEW_CONNECTION indicates that no new incoming
           connection, was accepted.  All other nonzero values indicate an error,
           See @ref osalStatus "OSAL function return codes" for full list.
           This parameter can be OS_NULL, if no status code is needed.
  @param   flags Flags for creating the socket. Define OSAL_STREAM_DEFAULT for normal operation.
           See @ref osalStreamFlags "Flags for Stream Functions" for full list of flags.
  @return  Stream pointer (handle) representing the stream, or OS_NULL if no new connection
           was accepted.

****************************************************************************************************
*/
static osalStream ioc_switchbox_socket_accept(
    osalStream stream,
    os_char *remote_ip_addr,
    os_memsz remote_ip_addr_sz,
    osalStatus *status,
    os_int flags)
{
    osalSwitchboxSocket *thiso, *newsocket = OS_NULL;

    if (stream)
    {
        /* Cast stream pointer to switchbox socket structure pointer.
         */
        thiso = (osalSwitchboxSocket*)stream;
        osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);

#if 0
        /* If no new connection, do nothing more.
         */
        if (new_handle == -1)
        {
            if (status) *status = OSAL_NO_NEW_CONNECTION;
            return OS_NULL;
        }

        /* Set socket reuse, blocking mode, and nagle.
         */
        if (flags == OSAL_STREAM_DEFAULT) {
            flags = thiso->open_flags;
        }

        /* Allocate and clear socket structure.
         */
        newsocket = (osalSwitchboxSocket*)os_malloc(sizeof(osalSwitchboxSocket), OS_NULL);
        if (newsocket == OS_NULL)
        {
            // close(new_handle);
            if (status) *status = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
            return OS_NULL;
        }
        os_memclear(newsocket, sizeof(osalSwitchboxSocket));

        if (flags & OSAL_STREAM_TCP_NODELAY) {
            ioc_switchbox_socket_setup_ring_buffer(thiso);
        }

        /* Save open flags.
         */
        newsocket->open_flags = flags;


        /* Save interface pointer.
         */
        newsocket->hdr.iface = &ioc_switchbox_socket_iface;

        /* Success set status code and cast socket structure pointer to stream pointer
           and return it.
         */
        osal_trace2("socket accepted");
        if (status) *status = OSAL_SUCCESS;
#endif
        return (osalStream)newsocket;
    }

    /* Opt out on error. If we got far enough to allocate the socket structure.
       Close the event handle (if any) and free memory allocated  for the socket structure.
     */
    if (newsocket)
    {
        os_free(newsocket, sizeof(osalSwitchboxSocket));
    }

    /* Set status code and return NULL pointer.
     */
    if (status) *status = OSAL_STATUS_FAILED;
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Flush the socket.
  @anchor ioc_switchbox_socket_flush

  The ioc_switchbox_socket_flush() function flushes data to be written to stream.

  IMPORTANT, FLUSH MUST BE CALLED: The osal_stream_flush(<stream>, OSAL_STREAM_DEFAULT) must
  be called when select call returns even after writing or even if nothing was written, or
  periodically in in single thread mode. This is necessary even if no data was written
  previously, the socket may have stored buffered data to avoid blocking.

  @param   stream Stream pointer representing the socket.
  @param   flags Often OSAL_STREAM_DEFAULT. See @ref osalStreamFlags "Flags for Stream Functions"
           for full list of flags.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_flush(
    osalStream stream,
    os_int flags)
{
    osalSwitchboxSocket *thiso;
    os_char *buf;
    os_memsz nwr;
    os_int head, tail, wrnow, buf_sz;
    osalStatus status;

    if (stream)
    {
        thiso = (osalSwitchboxSocket*)stream;
        head = thiso->head;
        tail = thiso->tail;

        if (head != tail)
        {
            buf = thiso->buf;
            buf_sz = thiso->buf_sz;

            /* Never split to two TCP packets.
             */
            if (head < tail && head)
            {
                /* tmpbuf = alloca(buf_sz); */
                os_char tmpbuf[buf_sz];
                wrnow = buf_sz - tail;
                os_memcpy(tmpbuf, buf + tail, wrnow);
                os_memcpy(tmpbuf + wrnow, buf, head);
                tail = 0;
                head += wrnow;
                os_memcpy(buf, tmpbuf, head);
            }

            if (head < tail)
            {
                wrnow = buf_sz - tail;

                status = ioc_switchbox_socket_write2(thiso, buf + tail, wrnow, &nwr, flags);
                if (status) goto getout;
                if (nwr == wrnow) tail = 0;
                else tail += (os_int)nwr;
            }

            if (head > tail)
            {
                wrnow = head - tail;

                status = ioc_switchbox_socket_write2(thiso, buf + tail, wrnow, &nwr, flags);
                if (status) goto getout;
                tail += (os_int)nwr;
            }

            if (tail == head)
            {
                tail = head = 0;
            }

            thiso->head = head;
            thiso->tail = tail;
        }
    }

    return OSAL_SUCCESS;

getout:
    return status;
}


/**
****************************************************************************************************

  @brief Write data to socket.
  @anchor ioc_switchbox_socket_write

  The ioc_switchbox_socket_write() function writes up to n bytes of data from buffer to socket.

  @param   stream Stream pointer representing the socket.
  @param   buf Pointer to the beginning of data to place into the socket.
  @param   n Maximum number of bytes to write.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written to socket,  which may be less than n if there is not enough space
           left in the socket. If the function fails n_written is set to zero.
  @param   flags Flags for the function.
           See @ref osalStreamFlags "Flags for Stream Functions" for full list of flags.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_write2(
    osalSwitchboxSocket *thiso,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags)
{
    os_int rval;
    osalStatus status;
    OSAL_UNUSED(flags);

    /* Check for errorneous arguments.
     */
    if (n < 0 || buf == OS_NULL)
    {
        status = OSAL_STATUS_FAILED;
        goto getout;
    }

    /* If nothing to write.
     */
    if (n == 0)
    {
        status = OSAL_SUCCESS;
        goto getout;
    }
#if 0
    /* get operating system's socket handle.
     */
    handle = thiso->handle;

    rval = send(handle, buf, (int)n, MSG_NOSIGNAL);

    if (rval < 0)
    {
        /* This matches with net_sockets.c
         */
        switch (errno)
        {
            case EWOULDBLOCK:
            case EINPROGRESS:
            case EINTR:
                break;

            case ECONNREFUSED:
                status = OSAL_STATUS_CONNECTION_REFUSED;
                goto getout;

            case ECONNRESET:
            case EPIPE:
                status = OSAL_STATUS_CONNECTION_RESET;
                goto getout;

            default:
                status = OSAL_STATUS_FAILED;
                goto getout;
        }

        rval = 0;
    }

    thiso->write2_blocked = (os_boolean)(rval != n);
#endif

rval = OSAL_SUCCESS;
    osal_resource_monitor_update(OSAL_RMON_TX_TCP, rval);
    *n_written = rval;
    return OSAL_SUCCESS;

getout:
    *n_written = 0;
    return status;
}


/**
****************************************************************************************************

  @brief Write data to socket.
  @anchor ioc_switchbox_socket_write

  The ioc_switchbox_socket_write() function writes up to n bytes of data from buffer to socket.

  @param   stream Stream pointer representing the socket.
  @param   buf Pointer to the beginning of data to place into the socket.
  @param   n Maximum number of bytes to write.
  @param   n_written Pointer to integer into which the function stores the number of bytes
           actually written to socket,  which may be less than n if there is not enough space
           left in the socket. If the function fails n_written is set to zero.
  @param   flags Flags for the function.
           See @ref osalStreamFlags "Flags for Stream Functions" for full list of flags.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_write(
    osalStream stream,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags)
{
    int count, wrnow;
    osalSwitchboxSocket *thiso;
    osalStatus status;
    os_char *rbuf;
    os_int head, tail, buf_sz, nexthead;
    os_memsz nwr;
    os_boolean all_not_flushed;

    if (stream)
    {
        /* Cast stream pointer to socket structure pointer.
         */
        thiso = (osalSwitchboxSocket*)stream;
        osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);

        /* Check for errorneous arguments.
         */
        if (n < 0 || buf == OS_NULL)
        {
            status = OSAL_STATUS_FAILED;
            goto getout;
        }

        /* Special case. Writing 0 bytes will trigger write callback by worker thread.
         */
        if (n == 0)
        {
            status = OSAL_SUCCESS;
            goto getout;
        }

        if (thiso->buf)
        {
            rbuf = thiso->buf;
            buf_sz = thiso->buf_sz;
            head = thiso->head;
            tail = thiso->tail;
            all_not_flushed = OS_FALSE;
            count = 0;

            while (OS_TRUE)
            {
                while (n > 0)
                {
                    nexthead = head + 1;
                    if (nexthead >= buf_sz) nexthead = 0;
                    if (nexthead == tail) break;
                    rbuf[head] = *(buf++);
                    head = nexthead;
                    n--;
                    count++;
                }

                if (n == 0 || all_not_flushed)
                {
                    break;
                }

                /* Never split to two TCP packets.
                 */
                if (head < tail && head)
                {
                    os_char tmpbuf[buf_sz];
                    wrnow = buf_sz - tail;
                    os_memcpy(tmpbuf, rbuf + tail, wrnow);
                    os_memcpy(tmpbuf + wrnow, rbuf, head);
                    tail = 0;
                    head += wrnow;
                    os_memcpy(rbuf, tmpbuf, head);
                }

                if (head < tail)
                {
                    wrnow = buf_sz - tail;
                    status = ioc_switchbox_socket_write2(thiso, rbuf+tail, wrnow, &nwr, flags);
                    if (status) goto getout;
                    if (nwr == wrnow) tail = 0;
                    else tail += (os_int)nwr;
                }

                if (head > tail)
                {
                    wrnow = head - tail;
                    status = ioc_switchbox_socket_write2(thiso, rbuf+tail, wrnow, &nwr, flags);
                    if (status) goto getout;
                    tail += (os_int)nwr;
                }

                if (tail == head)
                {
                    tail = head = 0;
                }
                else
                {
                    all_not_flushed = OS_TRUE;
                }
            }

            thiso->head = head;
            thiso->tail = tail;
            *n_written = count;
            return OSAL_SUCCESS;
        }

        return ioc_switchbox_socket_write2(thiso, buf, n, n_written, flags);
    }
    status = OSAL_STATUS_FAILED;

getout:
    *n_written = 0;
    return status;
}


/**
****************************************************************************************************

  @brief Read data from socket.
  @anchor ioc_switchbox_socket_read

  The ioc_switchbox_socket_read() function reads up to n bytes of data from socket into buffer.

  @param   stream Stream pointer representing the socket.
  @param   buf Pointer to buffer to read into.
  @param   n Maximum number of bytes to read. The data buffer must large enough to hold
           at least this many bytes.
  @param   n_read Pointer to integer into which the function stores the number of bytes read,
           which may be less than n if there are fewer bytes available. If the function fails
           n_read is set to zero.
  @param   flags Flags for the function, use OSAL_STREAM_DEFAULT (0) for default operation.

  @return  Function status code. Value OSAL_SUCCESS (0) indicates success and all nonzero values
           indicate an error. See @ref osalStatus "OSAL function return codes" for full list.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_read(
    osalStream stream,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags)
{
    // osalSwitchboxSocket *thiso;
    // os_int handle, rval;
    osalStatus status;
    OSAL_UNUSED(flags);

#if 0

    if (stream)
    {
        /* Cast stream pointer to socket structure pointer, get operating system's socket handle.
         */
        thiso = (osalSwitchboxSocket*)stream;
        osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);
        handle = thiso->handle;

        /* Check for errorneous arguments.
         */
        if (n < 0 || buf == OS_NULL)
        {
            status = OSAL_STATUS_FAILED;
            goto getout;
        }

        rval = recv(handle, buf, (int)n, MSG_NOSIGNAL);

        /* If other end has gracefylly closed.
         */
        if (rval == 0)
        {
            osal_trace2("socket gracefully closed");
            *n_read = 0;
            return OSAL_STATUS_STREAM_CLOSED;
        }

        if (rval == -1)
        {
            /* This matches with net_sockets.c
             */
            switch (errno)
            {
                case EWOULDBLOCK:
                case EINPROGRESS:
                case EINTR:
                    break;

                case ECONNREFUSED:
                    status = OSAL_STATUS_CONNECTION_REFUSED;
                    goto getout;

                case ECONNRESET:
                case EPIPE:
                    status = OSAL_STATUS_CONNECTION_RESET;
                    goto getout;

                default:
                    status = OSAL_STATUS_FAILED;
                    goto getout;
            }
            rval = 0;
        }

        osal_resource_monitor_update(OSAL_RMON_RX_TCP, rval);
        *n_read = rval;
        return OSAL_SUCCESS;
    }
#endif

    status = OSAL_STATUS_FAILED;

// getout:
    osal_trace2("socket read failed");
    *n_read = 0;
    return status;
}


/**
****************************************************************************************************

  @brief Wait for an event from one of sockets.
  @anchor ioc_switchbox_socket_select

  The ioc_switchbox_socket_select() function blocks execution of the calling thread until something
  happens with listed sockets, or event given as argument is triggered.

  Interrupting select: The easiest way is probably to use pipe(2) to create a pipe and add the
  read end to readfds. When the other thread wants to interrupt the select() just write a byte
  to it, then consume it afterward.

  @param   streams Array of streams to wait for. These must be serial ports, no mixing
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
  @param   timeout_ms Maximum time to wait in select, ms. If zero, timeout is not used (infinite).
  @param   flags Ignored, set OSAL_STREAM_DEFAULT (0).
  @return  If successful, the function returns OSAL_SUCCESS (0) and the selectdata tells which
           socket or event triggered the thread to continue. Other return values indicate an error.
           See @ref osalStatus "OSAL function return codes" for full list.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_select(
    osalStream *streams,
    os_int nstreams,
    osalEvent evnt,
    osalSelectData *selectdata,
    os_int timeout_ms,
    os_int flags)
{
//     osalSwitchboxSocket *thiso;
   //  os_int i, handle, socket_nr, maxfd, pipefd, rval;
    OSAL_UNUSED(flags);

    os_memclear(selectdata, sizeof(osalSelectData));
#if 0
    if (nstreams < 1 || nstreams > IOC_SWITCHBOX_SOCKET_SELECT_MAX)
        return OSAL_STATUS_FAILED;

    FD_ZERO(&rdset);
    FD_ZERO(&wrset);
    FD_ZERO(&exset);

    maxfd = 0;
    for (i = 0; i < nstreams; i++)
    {
        thiso = (osalSwitchboxSocket*)streams[i];
        if (thiso)
        {
            osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);
            handle = thiso->handle;

            FD_SET(handle, &rdset);
            thiso->wrset_enabled = OS_FALSE;
            if (thiso->write2_blocked /* || !thiso->connected */)
            {
                FD_SET(handle, &wrset);
                thiso->wrset_enabled = OS_TRUE;
            }
            FD_SET(handle, &exset);
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

    rval = pselect(maxfd+1, &rdset, &wrset, &exset, to, NULL);
    if (rval <= 0)
    {
        if (rval == 0)
        {
            selectdata->stream_nr = OSAL_STREAM_NR_TIMEOUT_EVENT;
            return OSAL_SUCCESS;
        }
    }

    if (pipefd >= 0) if (FD_ISSET(pipefd, &rdset))
    {
        osal_event_clearpipe(evnt);

        selectdata->stream_nr = OSAL_STREAM_NR_CUSTOM_EVENT;
        return OSAL_SUCCESS;
    }

    /* Find out socket number
     */
    for (socket_nr = 0; socket_nr < nstreams; socket_nr++)
    {
        thiso = (osalSwitchboxSocket*)streams[socket_nr];
        if (thiso)
        {
            handle = thiso->handle;

            if (FD_ISSET (handle, &exset)) {
                break;
            }

            if (FD_ISSET (handle, &rdset)) {
                break;
            }

            if (thiso->wrset_enabled) {
                if (FD_ISSET (handle, &wrset)) {
                    break;
                }
            }
        }
    }

    if (socket_nr == nstreams)
    {
        socket_nr = OSAL_STREAM_NR_UNKNOWN_EVENT;
    }

    selectdata->stream_nr = socket_nr;
#endif
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Set up ring buffer for sends.
  @anchor ioc_switchbox_socket_setup_ring_buffer

  The ring buffer is used to control sending of TCP packets. Writes are first collected to
  the ring buffer and then flushed.

  @param  handle SwitchboxSocket handle.
  @param  state Nonzero to disable Nagle's algorithm (no delay mode), zero to enable it.
  @return OSAL_SUCCESS if all is fine, OSAL_STATUS_MEMORY_ALLOCATION_FAILED if memory
          allocation failed.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_setup_ring_buffer(
    osalSwitchboxSocket *thiso)
{
    os_memsz buf_sz;

    /* 16768 selected for TCP sockets */
    thiso->buf = (os_char*)os_malloc(16768, &buf_sz);
    thiso->buf_sz = (os_int)buf_sz;
    return thiso->buf ? OSAL_SUCCESS : OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
}


/** Stream interface for OSAL sockets. This is structure osalStreamInterface filled with
    function pointers to OSAL sockets implementation.
 */
OS_CONST osalStreamInterface ioc_switchbox_socket_iface
 = {OSAL_STREAM_IFLAG_NONE,
    ioc_switchbox_socket_open,
    ioc_switchbox_socket_close,
    ioc_switchbox_socket_accept,
    ioc_switchbox_socket_flush,
    osal_stream_default_seek,
    ioc_switchbox_socket_write,
    ioc_switchbox_socket_read,
    osal_stream_default_write_value,
    osal_stream_default_read_value,
    osal_stream_default_get_parameter,
    osal_stream_default_set_parameter,
    ioc_switchbox_socket_select,
    OS_NULL,
    OS_NULL};

#endif
