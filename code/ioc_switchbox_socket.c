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
#if IOC_SWITCHBOX_SUPPORT

struct switchboxSocket;

/**
****************************************************************************************************
    Linked list of switchbox client sockets for one switchbox end point socket.
****************************************************************************************************
*/
typedef struct switchboxSocketLink
{
    /** Pointer to the end point socket.
     */
    struct switchboxSocket *scon;

    /** Pointer to the next client socket in linked list.
     */
    struct switchboxSocket *next;

    /** Pointer to the previous client socket in linked list.
     */
    struct switchboxSocket *prev;
}
switchboxSocketLink;

/**
****************************************************************************************************
    Service socket object uses this structure to hold linked list of client socket objects.
****************************************************************************************************
*/
typedef struct switchboxSocketList
{
    /** Pointer to the next first client socket object.
     */
    struct switchboxSocket *first;

    /** Pointer to the previous client socket object.
     */
    struct switchboxSocket *last;
}
switchboxSocketList;


/**
****************************************************************************************************
    Switchbox socket structure.
****************************************************************************************************
*/
typedef struct switchboxSocket
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

    /** True if this is end point object, which connects to the switchbox service.
     */
    os_boolean is_service_socket;

    os_boolean handshake_ready;
    os_boolean authentication_received;
    os_boolean authentication_sent;


    /** Handshake state structure (switbox cloud net name and copying trust certificate).
     */
    iocHandshakeState handshake;

    /** Authentication buffers.
     */
    iocSwitchboxAuthenticationFrameBuffer *auth_recv_buf;
    iocSwitchboxAuthenticationFrameBuffer *auth_send_buf;

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
    union {
        switchboxSocketList head;   /* Service connection holds head of the list */
        switchboxSocketLink clink;  /* Client connections link together */
    }
    list;
}
switchboxSocket;


/* Prototypes for forward referred static functions.
 */
static osalStatus ioc_switchbox_socket_write2(
    switchboxSocket *thiso,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags);

/* static osalStatus ioc_switchbox_socket_setup_ring_buffer(
    switchboxSocket *thiso); */

static void ioc_switchbox_socket_link(
    switchboxSocket *thiso,
    switchboxSocket *ssock);

static void ioc_switchbox_socket_unlink(
    switchboxSocket *thiso);

static osalStatus ioc_switchbox_socket_handshake(
    switchboxSocket *thiso);

static void ioc_save_switchbox_trust_certificate(
    const os_uchar *cert,
    os_memsz cert_sz,
    void *context);


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
    switchboxSocket *thiso = OS_NULL;
    osalStream switchbox_stream;

    /* Open shared connection.
     */
    switchbox_stream = osal_stream_open(OSAL_TLS_IFACE, parameters, option,
        status, OSAL_STREAM_CONNECT);
    if (switchbox_stream == OS_NULL) {
        return OS_NULL;
    }

    /* Allocate and clear stream structure.
     */
    thiso = (switchboxSocket*)os_malloc(sizeof(switchboxSocket), OS_NULL);
    if (thiso == OS_NULL)
    {
        if (status) {
            *status = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        }
        osal_stream_close(switchbox_stream, OSAL_STREAM_DEFAULT);
        return OS_NULL;
    }
    os_memclear(thiso, sizeof(switchboxSocket));

    /* Save flags, interface pointer and steam.
     */
    thiso->open_flags = flags;
    thiso->is_service_socket = OS_TRUE;
    thiso->hdr.iface = &ioc_switchbox_socket_iface;
    thiso->switchbox_stream = switchbox_stream;

    ioc_initialize_handshake_state(&thiso->handshake);

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
    switchboxSocket *thiso;
    OSAL_UNUSED(flags);

    /* If called with NULL argument, do nothing.
     */
    if (stream == OS_NULL) return;

    /* Cast stream pointer.
     */
    thiso = (switchboxSocket*)stream;
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);

    /* Send disconnect message.
     */

    /* Detach form chain, synchronization necessary.
     */
    os_lock();
    ioc_switchbox_socket_unlink(thiso);
    os_unlock();

    if (thiso->switchbox_stream) {
        osal_stream_close(thiso->switchbox_stream, OSAL_STREAM_DEFAULT);
        thiso->switchbox_stream = OS_NULL;
    }
    thiso->hdr.iface = OS_NULL;


    /* Free hand shake state, ring buffer and memory allocated for socket structure.
     */
    ioc_release_handshake_state(&thiso->handshake);
    os_free(thiso->auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
    os_free(thiso->auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
    os_free(thiso->buf, thiso->buf_sz);
    os_free(thiso, sizeof(switchboxSocket));
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
    switchboxSocket *thiso, *newsocket;
    osalStatus s;

    if (remote_ip_addr) {
        *remote_ip_addr = '\0';
    }

    if (stream == OS_NULL) {
        if (status) *status = OSAL_STATUS_FAILED;
        return OS_NULL;
    }
    thiso = (switchboxSocket*)stream;

    s = ioc_switchbox_socket_handshake(thiso);
    if (s) {
        if (status) *status = (s == OSAL_PENDING) ? OSAL_NO_NEW_CONNECTION : s;
        return OS_NULL;
    }

*status = OSAL_NO_NEW_CONNECTION;
return OS_NULL
;
    /* Cast stream pointer to switchbox socket structure pointer.
     */
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);

#if 0
    /* If no new connection, do nothing more.
     */
    if (new_handle == -1)
    {
        if (status) *status = OSAL_NO_NEW_CONNECTION;
        return OS_NULL;
    }
#endif

    /* Set socket reuse, blocking mode, and nagle.
     */
    if (flags == OSAL_STREAM_DEFAULT) {
        flags = thiso->open_flags;
    }


    /* Allocate and clear socket structure.
     */
    newsocket = (switchboxSocket*)os_malloc(sizeof(switchboxSocket), OS_NULL);
    if (newsocket == OS_NULL)
    {
        // close(new_handle);
        if (status) *status = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        return OS_NULL;
    }
    os_memclear(newsocket, sizeof(switchboxSocket));

#if 0
    if (flags & OSAL_STREAM_TCP_NODELAY) {
        ioc_switchbox_socket_setup_ring_buffer(newsocket);
    }
#endif

    newsocket->hdr.iface = &ioc_switchbox_socket_iface;
    newsocket->open_flags = flags;

    os_lock();
    ioc_switchbox_socket_link(newsocket, thiso);
    os_unlock();

    /* Success set status code and cast socket structure pointer to stream pointer
       and return it.
     */
    osal_trace2("socket accepted");
    if (status) *status = OSAL_SUCCESS;
    return (osalStream)newsocket;
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
    switchboxSocket *thiso;
    os_char *buf;
    os_memsz nwr;
    os_int head, tail, wrnow, buf_sz;
    osalStatus status;

    if (stream)
    {
        thiso = (switchboxSocket*)stream;
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
    switchboxSocket *thiso,
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
    switchboxSocket *thiso;
    osalStatus status;
    os_char *rbuf;
    os_int head, tail, buf_sz, nexthead;
    os_memsz nwr;
    os_boolean all_not_flushed;

    if (stream)
    {
        /* Cast stream pointer to socket structure pointer.
         */
        thiso = (switchboxSocket*)stream;
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
    // switchboxSocket *thiso;
    // os_int handle, rval;
    osalStatus status;
    OSAL_UNUSED(flags);

#if 0

    if (stream)
    {
        /* Cast stream pointer to socket structure pointer, get operating system's socket handle.
         */
        thiso = (switchboxSocket*)stream;
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
    status = OSAL_SUCCESS;

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
    switchboxSocket *thiso;
    os_int i; // , handle, socket_nr, maxfd, pipefd, rval;
    OSAL_UNUSED(flags);

    os_memclear(selectdata, sizeof(osalSelectData));

    if (nstreams < 1 || nstreams > OSAL_SOCKET_SELECT_MAX)
        return OSAL_STATUS_FAILED;

    for (i = 0; i < nstreams; i++) {
        thiso = (switchboxSocket*)streams[i];
        if (!thiso->handshake_ready || !thiso->authentication_received || !thiso->authentication_sent) {
            os_timeslice();
            return OSAL_SUCCESS;
        }
    }

#if 0
    FD_ZERO(&rdset);
    FD_ZERO(&wrset);
    FD_ZERO(&exset);

    maxfd = 0;
    for (i = 0; i < nstreams; i++)
    {
        thiso = (switchboxSocket*)streams[i];
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
        thiso = (switchboxSocket*)streams[socket_nr];
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

  @brief Add a client socket object to service socket object's linked list.

  Note: os_lock must be on when this function is called.

  @param   thiso Pointer to the client socket object.
  @param   ssock Pointer to the service socket object.

****************************************************************************************************
*/
static void ioc_switchbox_socket_link(
    switchboxSocket *thiso,
    switchboxSocket *ssock)
{
    osal_debug_assert(ssock->is_service_socket);

    /* Join to list of client connections for the server connection.
     */
    thiso->list.clink.prev = ssock->list.head.last;
    thiso->list.clink.next = OS_NULL;
    thiso->list.clink.scon = ssock;
    if (ssock->list.head.last) {
        ssock->list.head.last->list.clink.next = thiso;
    }
    else {
        ssock->list.head.first = thiso;
    }
    ssock->list.head.last = thiso;
}


/**
****************************************************************************************************

  @brief Remove from service socket's linked list.

  If thiso is service socket, all client sockets are unlinked and requested to terminate.

  Note: os_lock must be on when this function is called.

  @param   thiso Pointer to the switchbox socket object to unlink. Can be service or client socket.

****************************************************************************************************
*/
static void ioc_switchbox_socket_unlink(
    switchboxSocket *thiso)
{
    switchboxSocket *c, *next_c, *scon;

    /* thiso is service connection.
     */
    if (thiso->is_service_socket) {
        for (c = thiso->list.head.first; c; c = next_c) {
            next_c = c->list.clink.next;
         //   c->worker.stop_thread = OS_TRUE; XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
         //   osal_event_set(c->worker.trig);
            c->list.clink.next = c->list.clink.prev = c->list.clink.scon = OS_NULL;
        }
        thiso->list.head.first = thiso->list.head.last = OS_NULL;
    }

    /* thiso is client connection.
     */
    else {
        scon = thiso->list.clink.scon;
        if (scon) {
            if (thiso->list.clink.prev) {
                thiso->list.clink.prev->list.clink.next = thiso->list.clink.next;
            }
            else {
                scon->list.head.first = thiso->list.clink.next;
            }
            if (thiso->list.clink.next) {
                thiso->list.clink.next->list.clink.prev = thiso->list.clink.prev;
            }
            else {
                scon->list.head.last = thiso->list.clink.prev;
            }
            thiso->list.clink.next = thiso->list.clink.prev = thiso->list.clink.scon = OS_NULL;
        }
    }
}


/**
****************************************************************************************************

  @brief Do first handshake for to connect to switchbox.
  @anchor ioc_first_handshake

  @param   thiso Pointer to the client socket service.
  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_handshake(
    switchboxSocket *thiso)
{
    osalStatus s;

os_boolean cert_match = OS_TRUE;

    if (thiso->handshake_ready && thiso->authentication_received && thiso->authentication_sent) {
        return OSAL_SUCCESS;
    }

    if (!thiso->handshake_ready)
    {
        s = ioc_client_handshake(&thiso->handshake, IOC_HANDSHAKE_NETWORK_SERVICE,
            "kepuli", !cert_match, thiso->switchbox_stream,
            ioc_save_switchbox_trust_certificate, thiso);

        osal_stream_flush(thiso->switchbox_stream, OSAL_STREAM_DEFAULT);

        if (s) {
            return s;
        }

        thiso->handshake_ready = OS_TRUE;
    }

    /* Service connection: We need to receive authentication frame.
     */
    if (!thiso->authentication_received) {
        if (thiso->auth_recv_buf == OS_NULL) {
            thiso->auth_recv_buf = (iocSwitchboxAuthenticationFrameBuffer*)
                os_malloc(sizeof(iocSwitchboxAuthenticationFrameBuffer), OS_NULL);
            os_memclear(thiso->auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        }

        iocAuthenticationResults results;
        s = icom_switchbox_process_authentication_frame(thiso->switchbox_stream, thiso->auth_recv_buf, &results);
        if (s == OSAL_COMPLETED) {
            os_free(thiso->auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
            thiso->auth_recv_buf = OS_NULL;
            thiso->authentication_received = OS_TRUE;
        }
        else if (s != OSAL_PENDING) {
            osal_debug_error("eConnection: Valid authentication frame was not received");
            return OSAL_STATUS_FAILED;
        }
    }

    /* Service connection: We need to send responce(s).
     */
    if (!thiso->authentication_sent)
    {
        iocSwitchboxAuthenticationParameters prm;
        os_memclear(&prm, sizeof(prm));

        if (thiso->auth_send_buf == OS_NULL) {
            thiso->auth_send_buf = (iocSwitchboxAuthenticationFrameBuffer*)
                os_malloc(sizeof(iocSwitchboxAuthenticationFrameBuffer), OS_NULL);
            os_memclear(thiso->auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));

            prm.network_name = "sb";
            prm.user_name = "srv";
            prm.password = "pw";
        }

        s = ioc_send_switchbox_authentication_frame(thiso->switchbox_stream, thiso->auth_send_buf, &prm);
        if (s == OSAL_COMPLETED) {
            os_free(thiso->auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
            thiso->auth_send_buf = OS_NULL;
            thiso->authentication_sent = OS_TRUE;
            osal_stream_flush(thiso->switchbox_stream, OSAL_STREAM_DEFAULT);
        }
        else if (s != OSAL_PENDING) {
            osal_debug_error("eConnection: Failed to send authentication frame");
            return OSAL_STATUS_FAILED;
        }
    }

    osal_stream_flush(thiso->switchbox_stream, OSAL_STREAM_DEFAULT);

    if (!thiso->authentication_sent ||
        !thiso->authentication_received)
    {
        os_timeslice();
        return OSAL_PENDING;
    }

    /* switch (htype)
    {
        case IOC_HANDSHAKE_NETWORK_SERVICE:
            s = ioc_switchbox_setup_service_connection(thiso);
            break;

        case IOC_HANDSHAKE_CLIENT:
            s = ioc_switchbox_setup_client_connection(thiso);
            break;

        default:
            s = OSAL_STATUS_FAILED;
            osal_debug_error("switchbox: unknown incoming connection type");
            break;
    } */

    return OSAL_SUCCESS;
}


/* Save received certificate (client only).
 */
static void ioc_save_switchbox_trust_certificate(
    const os_uchar *cert,
    os_memsz cert_sz,
    void *context)
{
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
#if 0
static osalStatus ioc_switchbox_socket_setup_ring_buffer(
    switchboxSocket *thiso)
{
    os_memsz buf_sz;

    /* 16768 selected for TCP sockets */
    thiso->buf = (os_char*)os_malloc(16768, &buf_sz);
    thiso->buf_sz = (os_int)buf_sz;
    return thiso->buf ? OSAL_SUCCESS : OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
}
#endif


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
