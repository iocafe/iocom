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
#if IOC_DYNAMIC_MBLK_CODE


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
    // os_int handle;

    /** Port number for multicasts or listening connections.
     */
    // os_int passive_port;

    /** Stream open flags. Flags which were given to ioc_switchbox_socket_open() or ioc_switchbox_socket_accept()
        function.
     */
    os_int open_flags;

    /** OS_TRUE if this is IPv6 socket.
     */
    // os_boolean is_ipv6;

    /** OS_TRUE if last write to socket has been blocked (would block).
     */
    // os_boolean write2_blocked;

    // os_boolean wrset_enabled;

    /** OS_TRUE if connection has been reported by select.
     */
    /* os_boolean connected; */

    /** Ring buffer, OS_NULL if not used.
     */
    os_char *buf;

    /** Buffer size in bytes.
     */
    os_short buf_sz;

    /** Head index. Position in buffer to which next byte is to be written. Range 0 ... buf_sz-1.
     */
    os_short head;

    /** Tail index. Position in buffer from which next byte is to be read. Range 0 ... buf_sz-1.
     */
    os_short tail;
}
osalSwitchboxSocket;


/* Prototypes for forward referred static functions.
 */
static osalStatus osal_setup_tcp_socket(
    osalSwitchboxSocket *mysocket,
    os_char *iface_addr_bin,
    os_boolean iface_addr_is_ipv6,
    os_int port_nr,
    os_int flags);

static osalStatus ioc_switchbox_socket_write2(
    osalSwitchboxSocket *mysocket,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags);

static void ioc_switchbox_socket_setup_ring_buffer(
    osalSwitchboxSocket *mysocket);


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
    osalSwitchboxSocket *mysocket = OS_NULL;
    os_char iface_addr_bin[OSAL_IP_BIN_ADDR_SZ];
    os_int port_nr;
    os_boolean is_ipv6;
    osalStatus s;
    os_int info_code;

    /* Get global socket data, return OS_NULL if not initialized.
     */
    if (osal_global->socket_global == OS_NULL)
    {
        if (status) *status = OSAL_STATUS_FAILED;
        return OS_NULL;
    }

    /* Get host name or numeric IP address and TCP port number from parameters.
     */
    s = osal_socket_get_ip_and_port(parameters, iface_addr_bin, sizeof(iface_addr_bin),
        &port_nr, &is_ipv6, flags, IOC_DEFAULT_SWITCHBOX_PORT);
    if (s)
    {
        if (status) *status = s;
        return OS_NULL;
    }

    /* Allocate and clear socket structure.
     */
    mysocket = (osalSwitchboxSocket*)os_malloc(sizeof(osalSwitchboxSocket), OS_NULL);
    if (mysocket == OS_NULL)
    {
        s = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        goto getout;
    }
    os_memclear(mysocket, sizeof(osalSwitchboxSocket));

    /* Save socket open flags and interface pointer.
     */
    mysocket->open_flags = flags;
    mysocket->hdr.iface = &ioc_switchbox_socket_iface;

    /* Open UDP multicast socket
     */
    if (flags & OSAL_STREAM_MULTICAST)
    {
        s = OSAL_STATUS_NOT_SUPPORTED;
        goto getout;
    }

    /* Open TCP socket.
     */
    else
    {
        s = osal_setup_tcp_socket(mysocket, iface_addr_bin, is_ipv6, port_nr, flags);
        if (s) goto getout;
        if (flags & OSAL_STREAM_LISTEN) {
            info_code = OSAL_LISTENING_SOCKET_CONNECTED;
        }
        else {
            info_code = OSAL_SOCKET_CONNECTED;
            osal_resource_monitor_increment(OSAL_RMON_SOCKET_CONNECT_COUNT);
        }
    }

    /* Success, inform event handler, set status code and return stream pointer.
     */
    osal_resource_monitor_increment(OSAL_RMON_SOCKET_COUNT);
    osal_info(eosal_mod, info_code, parameters);
    if (status) *status = OSAL_SUCCESS;
    return (osalStream)mysocket;

getout:
    /* If we got far enough to allocate the socket structure.
       Close the event handle (if any) and free memory allocated
       for the socket structure.
     */
    if (mysocket)
    {
        /* Close socket
         */
//        if (mysocket->handle != -1)
        {
            // close(mysocket->handle);
        }

        os_free(mysocket, sizeof(osalSwitchboxSocket));
    }

    /* Set status code and return NULL pointer.
     */
    if (status) *status = s;
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Connect or listen for TCP socket (internal).
  @anchor osal_setup_tcp_socket

  The osal_setup_tcp_socket() function....

  @param  mysocket Pointer to my socket structure.
  @param  iface_addr_bin IP address of network interface to use, binary format, 4 bytes for IPv4
          and 16 bytes for IPv6.
  @param  iface_addr_is_ipv6 OS_TRUE for IPv6, or OS_FALSE for IPv4.
  @param  port_nr TCP port number to listen or connect to.
  @param  flags Flags given to ioc_switchbox_socket_open().

  @return OSAL_SUCCESS (0) if all fine.

****************************************************************************************************
*/
static osalStatus osal_setup_tcp_socket(
    osalSwitchboxSocket *mysocket,
    os_char *iface_addr_bin,
    os_boolean iface_addr_is_ipv6,
    os_int port_nr,
    os_int flags)
{
    osalStatus s;
    int handle = -1;
    // os_int af, sa_sz;
    // int on;


    /* Create socket.
     */
    /* handle = socket(af, SOCK_STREAM,  IPPROTO_TCP);
    if (handle == -1)
    {
        s = OSAL_STATUS_FAILED;
        goto getout;
    } */

    /* Save flags and interface pointer.
     */
    // mysocket->open_flags = flags;
//    mysocket->is_ipv6 = iface_addr_is_ipv6;
    // mysocket->hdr.iface = &ioc_switchbox_socket_iface;

    if (flags & OSAL_STREAM_LISTEN)
    {
        /* if (bind(handle, sa, sa_sz))
        {
            s = OSAL_STATUS_FAILED;
            goto getout;
        } */

        /* Set the listen back log
         */
        /* if (listen(handle, 32) < 0)
        {
            s = OSAL_STATUS_FAILED;
            goto getout;
        } */

        /* Set any nonzero multicast port to indicate to close() function
         * that we do not need to call gracefull connection shutdown stuff.
         */
//        mysocket->passive_port = port_nr;
    }

#if 0
    else
    {
        if (connect(handle, sa, sa_sz))
        {
            if (errno != EWOULDBLOCK && errno != EINPROGRESS)
            {
                s = OSAL_STATUS_FAILED;
                goto getout;
            }
        }

        /* If we work without Nagel.
         */
        if (flags & OSAL_STREAM_TCP_NODELAY)
        {
            ioc_switchbox_socket_setup_ring_buffer(mysocket);
        }
    }

    mysocket->handle = handle;
#endif
    return OSAL_SUCCESS;

//getout:
    /* Close socket
     */
    if (handle != -1)
    {
//        close(handle);
    }

    /* Return status code
     */
    return s;
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
    osalSwitchboxSocket *mysocket;
    // os_int handle;
    char nbuf[OSAL_NBUF_SZ];
    os_int info_code;
    OSAL_UNUSED(flags);

    /* If called with NULL argument, do nothing.
     */
    if (stream == OS_NULL) return;

    /* Cast stream pointer to socket structure pointer, get operating system's socket handle.
     */
    mysocket = (osalSwitchboxSocket*)stream;
    osal_debug_assert(mysocket->hdr.iface == &ioc_switchbox_socket_iface);
    // handle = mysocket->handle;

#if OSAL_DEBUG
    /* Mark socket closed
     */
    mysocket->hdr.iface = OS_NULL;
#endif

    /* If this is not multicast or listening socket.
     */
#if 0
    if (mysocket->passive_port == 0)
    {
        /* Disable sending data. This informs other the end of socket that it is going down now.
         */
        if (shutdown(handle, 2))
        {
            if (errno != ENOTCONN)
            {
                osal_debug_error("shutdown() failed");
            }
        }

        /* Read data to be received until receive buffer is empty.
         */
        do
        {
            n = recv(handle, buf, sizeof(buf), MSG_NOSIGNAL);
            if (n == -1)
            {
#if OSAL_DEBUG

                if (errno != EWOULDBLOCK &&
                    errno != EINPROGRESS &&
                    errno != ENOTCONN)
                {
                    osal_debug_error("reading end failed");
                }
#endif
                break;
            }
        }
        while(n);
    }
#endif

    /* Close the socket.
     */
//    if (close(handle))
    {
        osal_debug_error("closesocket failed");
    }

    /* Report close info even if we report problem closing socket, we need
       keep count of sockets open correct.
     */
    // osal_int_to_str(nbuf, sizeof(nbuf), handle);
    if (mysocket->open_flags & OSAL_STREAM_MULTICAST)
    {
        info_code = OSAL_UDP_SOCKET_DISCONNECTED;
    }
    else if (mysocket->open_flags & OSAL_STREAM_LISTEN)
    {
        info_code = OSAL_LISTENING_SOCKET_DISCONNECTED;
    }
    else
    {
        info_code = OSAL_SOCKET_DISCONNECTED;
    }
    osal_info(eosal_mod, info_code, nbuf);

    /* Free ring buffer, if any, memory allocated for socket structure
       and decrement socket count.
     */
    os_free(mysocket->buf, mysocket->buf_sz);
    os_free(mysocket, sizeof(osalSwitchboxSocket));
    osal_resource_monitor_decrement(OSAL_RMON_SOCKET_COUNT);
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
    osalSwitchboxSocket *mysocket, *newsocket = OS_NULL;
    os_int new_handle = -1;
    // char addrbuf[INET6_ADDRSTRLEN];
    // socklen_t addr_size;

    if (stream)
    {
        /* Cast stream pointer to socket structure pointer, get operating system's socket handle.
         */
        mysocket = (osalSwitchboxSocket*)stream;
        osal_debug_assert(mysocket->hdr.iface == &ioc_switchbox_socket_iface);
        //  handle = mysocket->handle;

        /* Try to accept incoming socket.
         */
//            new_handle = accept(handle, (struct sockaddr*)&sin_remote, &addr_size);

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
            flags = mysocket->open_flags;
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
            ioc_switchbox_socket_setup_ring_buffer(mysocket);
        }

        /* Save socket handle and open flags.
         */
//         newsocket->handle = new_handle;
        newsocket->open_flags = flags;
//         newsocket->is_ipv6 = mysocket->is_ipv6;


        /* Save interface pointer.
         */
        newsocket->hdr.iface = &ioc_switchbox_socket_iface;

        /* Success set status code and cast socket structure pointer to stream pointer
           and return it.
         */
        osal_trace2("socket accepted");
        if (status) *status = OSAL_SUCCESS;
        osal_resource_monitor_increment(OSAL_RMON_SOCKET_COUNT);
        osal_resource_monitor_increment(OSAL_RMON_SOCKET_CONNECT_COUNT);
        return (osalStream)newsocket;
    }

// getout:
    /* Opt out on error. If we got far enough to allocate the socket structure.
       Close the event handle (if any) and free memory allocated  for the socket structure.
     */
    if (newsocket)
    {
        os_free(newsocket, sizeof(osalSwitchboxSocket));
    }

    /* Close socket
     */
    if (new_handle != -1)
    {
//        close(new_handle);
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
    osalSwitchboxSocket *mysocket;
    os_char *buf;
    os_memsz nwr;
    os_short head, tail, wrnow, buf_sz;
    osalStatus status;

    if (stream)
    {
        mysocket = (osalSwitchboxSocket*)stream;
        head = mysocket->head;
        tail = mysocket->tail;

        if (head != tail)
        {
            buf = mysocket->buf;
            buf_sz = mysocket->buf_sz;

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

                status = ioc_switchbox_socket_write2(mysocket, buf + tail, wrnow, &nwr, flags);
                if (status) goto getout;
                if (nwr == wrnow) tail = 0;
                else tail += (os_short)nwr;
            }

            if (head > tail)
            {
                wrnow = head - tail;

                status = ioc_switchbox_socket_write2(mysocket, buf + tail, wrnow, &nwr, flags);
                if (status) goto getout;
                tail += (os_short)nwr;
            }

            if (tail == head)
            {
                tail = head = 0;
            }

            mysocket->head = head;
            mysocket->tail = tail;
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
    osalSwitchboxSocket *mysocket,
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
    handle = mysocket->handle;

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

    mysocket->write2_blocked = (os_boolean)(rval != n);
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
    osalSwitchboxSocket *mysocket;
    osalStatus status;
    os_char *rbuf;
    os_short head, tail, buf_sz, nexthead;
    os_memsz nwr;
    os_boolean all_not_flushed;

    if (stream)
    {
        /* Cast stream pointer to socket structure pointer.
         */
        mysocket = (osalSwitchboxSocket*)stream;
        osal_debug_assert(mysocket->hdr.iface == &ioc_switchbox_socket_iface);

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

        if (mysocket->buf)
        {
            rbuf = mysocket->buf;
            buf_sz = mysocket->buf_sz;
            head = mysocket->head;
            tail = mysocket->tail;
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
                    status = ioc_switchbox_socket_write2(mysocket, rbuf+tail, wrnow, &nwr, flags);
                    if (status) goto getout;
                    if (nwr == wrnow) tail = 0;
                    else tail += (os_short)nwr;
                }

                if (head > tail)
                {
                    wrnow = head - tail;
                    status = ioc_switchbox_socket_write2(mysocket, rbuf+tail, wrnow, &nwr, flags);
                    if (status) goto getout;
                    tail += (os_short)nwr;
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

            mysocket->head = head;
            mysocket->tail = tail;
            *n_written = count;
            return OSAL_SUCCESS;
        }

        return ioc_switchbox_socket_write2(mysocket, buf, n, n_written, flags);
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
    // osalSwitchboxSocket *mysocket;
    // os_int handle, rval;
    osalStatus status;
    OSAL_UNUSED(flags);

#if 0

    if (stream)
    {
        /* Cast stream pointer to socket structure pointer, get operating system's socket handle.
         */
        mysocket = (osalSwitchboxSocket*)stream;
        osal_debug_assert(mysocket->hdr.iface == &ioc_switchbox_socket_iface);
        handle = mysocket->handle;

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
//     osalSwitchboxSocket *mysocket;
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
        mysocket = (osalSwitchboxSocket*)streams[i];
        if (mysocket)
        {
            osal_debug_assert(mysocket->hdr.iface == &ioc_switchbox_socket_iface);
            handle = mysocket->handle;

            FD_SET(handle, &rdset);
            mysocket->wrset_enabled = OS_FALSE;
            if (mysocket->write2_blocked /* || !mysocket->connected */)
            {
                FD_SET(handle, &wrset);
                mysocket->wrset_enabled = OS_TRUE;
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
        mysocket = (osalSwitchboxSocket*)streams[socket_nr];
        if (mysocket)
        {
            handle = mysocket->handle;

            if (FD_ISSET (handle, &exset)) {
                break;
            }

            if (FD_ISSET (handle, &rdset)) {
                break;
            }

            if (mysocket->wrset_enabled) {
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
  @return None.

****************************************************************************************************
*/
static void ioc_switchbox_socket_setup_ring_buffer(
    osalSwitchboxSocket *mysocket)
{
    mysocket->buf_sz = 1420; /* selected for TCP sockets */
    mysocket->buf = (os_char*)os_malloc(mysocket->buf_sz, OS_NULL);
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
