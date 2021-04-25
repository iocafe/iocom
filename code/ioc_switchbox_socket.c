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

    /** Stream open flags. Flags which were given to ioc_switchbox_socket_open() or
        ioc_switchbox_socket_accept() function.
     */
    os_int open_flags;

    /** True if this is end point object, which connects to the switchbox service.
     */
    os_boolean is_shared_socket;

    /** Client identifier, a number from 1 to 0xFFFF which uniquely identifies client
        connection. Zero for shared socket.
     */
    os_ushort client_id;

    os_boolean handshake_ready;
    os_boolean authentication_received;
    os_boolean authentication_sent;

    // os_boolean emulated_mark_byte_done;

    /** Handshake state structure (switbox cloud net name and copying trust certificate).
     */
    iocHandshakeState handshake;

    /** Authentication buffers.
     */
    iocSwitchboxAuthenticationFrameBuffer *auth_recv_buf;
    iocSwitchboxAuthenticationFrameBuffer *auth_send_buf;

    /** Ring buffer for incoming data.
     */
    osalRingBuf incoming;

    /** Ring buffer for outgoing data.
     */
    osalRingBuf outgoing;

    /** Stream has broken flag, OSAL_SUCCESS as long as all is fine, other values
        indicate an error.
     */
    osalStatus status;

    /** Linked list of swichbox socket objects sharing one TLS switchbox connection.
     */
    union {
        switchboxSocketList head;   /* Service connection holds head of the list */
        switchboxSocketLink clink;  /* Client connections link together */
    }
    list;

    /** Triggering thread select: Event given as argument to select, OS_NULL if
        not within select call or no event was given. os_lock() must be on to access.
     */
    osalEvent select_event;

    /** Triggering thread select: Memorized trig when thread was triggered while not
        within select.
     */
    volatile os_boolean trig_select;

    /** Flush writes to shared socket.
     */
    volatile os_boolean flush_writes;

    /** Shared socket: Invidual socket index to get data from first. This shares
        bandwith between individual connections, if data is generated faster than
        what can be written to shared connection.
     */
    os_int current_individual_socket_ix;

    /** Shared socket: Message header received, now expecting "incoming_bytes" of data
        for "incoming_client_id". incoming_bytes == 0 if expecting message header.
     */
    os_int incoming_bytes;
    os_ushort incoming_client_id;
}
switchboxSocket;


/* Prototypes for forward referred static functions.
 */
static osalStatus ioc_switchbox_socket_setup_ring_buffer(
    switchboxSocket *thiso);

static void ioc_switchbox_set_select_event(
    switchboxSocket *thiso);

static void ioc_switchbox_set_shared_select_event(
    switchboxSocket *thiso,
    os_boolean flush_writes);

static void ioc_switchbox_socket_link(
    switchboxSocket *thiso,
    switchboxSocket *ssock);

static void ioc_switchbox_socket_unlink(
    switchboxSocket *thiso);

static osalStatus ioc_switchbox_shared_socket_handshake(
    switchboxSocket *thiso);

static osalStatus ioc_switchbox_run_shared_socket(
    switchboxSocket *thiso,
    switchboxSocket **newsocket);

static osalStatus ioc_write_to_shared_switchbox_socket(
    switchboxSocket *thiso);

static osalStatus ioc_read_from_shared_switchbox_socket(
    switchboxSocket *thiso);

static void ioc_save_switchbox_trust_certificate(
    const os_uchar *cert,
    os_memsz cert_sz,
    void *context);


/**
****************************************************************************************************

  @brief Open a socket.
  @anchor ioc_switchbox_socket_open

  The ioc_switchbox_socket_open() function opens listening end point at switchbox service.
  The function connects TLS socket to switchbox service and transfers commands and data
  trough it.

  @param  parameters IP address and optionally port of switchbox service to connect to.
  SwitchboxSocket parameters, a list string or direct value.
          Address and port to connect to, or interface and port to listen for.
          SwitchboxSocket IP address and port can be specified either as value of "addr" item
          or directly in parameter sstring. For example "192.168.1.55:20" or "localhost:12345"
          specify IPv4 addressed. If only port number is specified, which is often
          useful for listening socket, for example ":12345".
          IPv6 address is automatically recognized from numeric address like
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
    osalStatus s;

    osal_debug_assert(flags & OSAL_STREAM_LISTEN);

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
    thiso->is_shared_socket = OS_TRUE;
    thiso->hdr.iface = &ioc_switchbox_socket_iface;
    thiso->switchbox_stream = switchbox_stream;

    s = ioc_switchbox_socket_setup_ring_buffer(thiso);
    if (s) {
        if (status) {
            *status = s;
        }
        os_free(thiso, sizeof(switchboxSocket));
        return OS_NULL;
    }

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
    os_free(thiso->incoming.buf, thiso->incoming.buf_sz);
    os_free(thiso->outgoing.buf, thiso->outgoing.buf_sz);
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
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);

    s = ioc_switchbox_shared_socket_handshake(thiso);
    if (s) {
        if (status) *status = (s == OSAL_PENDING) ? OSAL_NO_NEW_CONNECTION : s;
        return OS_NULL;
    }

    s = ioc_switchbox_run_shared_socket(thiso, &newsocket);
    switch (s) {
        case OSAL_SUCCESS:
            break;

        case OSAL_WORK_DONE:
            ioc_switchbox_set_select_event(thiso);
            break;

        default:
            if (OSAL_IS_ERROR(s)) {
                if (status) *status = s;
                return OS_NULL;
            }
            break;
    }

    if (newsocket == OS_NULL) {
        *status = OSAL_NO_NEW_CONNECTION;
        return OS_NULL;
    }

    s = ioc_switchbox_socket_setup_ring_buffer(newsocket);
    if (s) {
        if (status) {
            *status = s;
        }
        os_free(newsocket, sizeof(switchboxSocket));
        return OS_NULL;
    }

    newsocket->hdr.iface = &ioc_switchbox_socket_iface;
    newsocket->open_flags = flags == OSAL_STREAM_DEFAULT ? thiso->open_flags : flags;

//    ioc_generate_emulated_client_handshake_message(newsocket);

    os_lock();
    ioc_switchbox_socket_link(newsocket, thiso);
    os_unlock();

    /* Success set status code and cast socket structure pointer to stream pointer
       and return it.
     */
    osal_trace2("switchbox socket accepted");
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
    OSAL_UNUSED(flags);

    if (stream == OS_NULL) {
        return OSAL_STATUS_FAILED;
    }
    thiso = (switchboxSocket*)stream;
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);
    osal_debug_assert(!thiso->is_shared_socket);

    ioc_switchbox_set_shared_select_event(thiso, OS_TRUE);

    if (thiso->status) {
        return thiso->status;
    }

    return OSAL_SUCCESS;
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
    switchboxSocket *thiso;
    os_int count;
    osalStatus s;
    OSAL_UNUSED(flags);

    if (stream == OS_NULL) {
        s = OSAL_STATUS_FAILED;
        goto getout;
    }
    thiso = (switchboxSocket*)stream;
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);
    osal_debug_assert(!thiso->is_shared_socket);

    if (thiso->status) {
        s = thiso->status;
        goto getout;
    }

    count = 0;
    if (n > 0) {
        count = osal_ringbuf_put(&thiso->incoming, buf, n);
        if (count) {
            ioc_switchbox_set_shared_select_event(thiso, OS_FALSE);
        }
    }

    *n_written = count;
    return OSAL_SUCCESS;

getout:
    *n_written = 0;
    return s;
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
           indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_socket_read(
    osalStream stream,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags)
{
    switchboxSocket *thiso;
    os_int count;
    osalStatus s;
    OSAL_UNUSED(flags);

    if (stream == OS_NULL) {
        s = OSAL_STATUS_FAILED;
        goto getout;
    }
    thiso = (switchboxSocket*)stream;
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);
    osal_debug_assert(!thiso->is_shared_socket);

    if (thiso->status) {
        s = thiso->status;
        goto getout;
    }

    count = 0;
    if (n > 0) {
        count = osal_ringbuf_get(&thiso->outgoing, buf, n);
        if (count) {
            ioc_switchbox_set_shared_select_event(thiso, OS_FALSE);
        }
    }

    *n_read = count;
    return OSAL_SUCCESS;

getout:
    osal_trace2("switchbox socket read failed");
    *n_read = 0;
    return s;
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

  @param   streams Array of streams to wait for. For switchbox this must be array of exactly
           one swithbox socket.
           types cannot be mixed in select.
  @param   n_streams This must be always 1 for the swithbox stream.
  @param   evnt Custom event to interrupt the select. OS_NULL if not needed.
  @param   selectdata Pointer to structure to fill in with information why select call
           returned. The "stream_nr" member is stream number which triggered the return:
           0 = first stream, 1 = second stream... Or one of OSAL_STREAM_NR_CUSTOM_EVENT,
           OSAL_STREAM_NR_TIMEOUT_EVENT or OSAL_STREAM_NR_UNKNOWN_EVENT. These indicate
           that event was triggered, wait timeout, and that stream implementation did
           not provide reason.
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
    osalStatus s;
    OSAL_UNUSED(flags);

    os_memclear(selectdata, sizeof(osalSelectData));

    if (nstreams != 1) {
        return OSAL_STATUS_FAILED;
    }

    thiso = (switchboxSocket*)streams[0];
    osal_debug_assert(thiso->hdr.iface == &ioc_switchbox_socket_iface);

    if (evnt) {
        /* Lock is necessary even this is atomic variable set, because the thread which triggers
         * this must get this pointer and set event within lock.
         */
        os_lock();
        thiso->select_event = evnt;
        if (thiso->trig_select) {
            thiso->trig_select = OS_FALSE;
            osal_event_set(evnt);
        }
        os_unlock();
    }

    /* Is this shared service socket or individual emulated one.
     */
    if (thiso->is_shared_socket)
    {
        s = osal_stream_select(&thiso->switchbox_stream, 1, evnt, selectdata,
            timeout_ms, OSAL_STREAM_DEFAULT);
    }
    else
    {
        if (evnt) {
            if (osal_event_wait(evnt, timeout_ms ? timeout_ms : OSAL_EVENT_INFINITE) == OSAL_STATUS_TIMEOUT)
            {
                selectdata->stream_nr = OSAL_STREAM_NR_TIMEOUT_EVENT;
            }
            else {
                selectdata->stream_nr = 0;
            }
        }
        else {
            os_timeslice();
        }

        s = OSAL_SUCCESS;
    }

    if (evnt) {
        os_lock();
        thiso->select_event = OS_NULL;
        os_unlock();
    }

    return s;
}


/**
****************************************************************************************************

  @brief Set select event.

  If select is ongoing, the function sets select event. The function sets trig_select variable
  to mark that select is imminent.

  Note: Use of os_lock() within the function is important.

  @param   thiso Pointer to the client socket object.

****************************************************************************************************
*/
static void ioc_switchbox_set_select_event(
    switchboxSocket *thiso)
{
    os_lock();
    thiso->trig_select = OS_TRUE;
    if (thiso->select_event) {
        osal_event_set(thiso->select_event);
    }
    os_unlock();
}


/**
****************************************************************************************************

  @brief Set shared select event.

  Called by individual switchbox socket to set shared socket event.

  Note: Use of os_lock() within the function is important.

  @param   thiso Pointer to the client socket object.
  @param   flush_writes Actually flush writes from ring buffer to socket

****************************************************************************************************
*/
static void ioc_switchbox_set_shared_select_event(
    switchboxSocket *thiso,
    os_boolean flush_writes)
{
    switchboxSocket *shared;

    osal_debug_assert(!thiso->is_shared_socket);

    os_lock();
    shared = thiso->list.clink.scon;
    if (shared) {
        /* flush_writes = 1: this function is called by flush(), it should trigger the thread only
           if we have something in ring buffer to write.
           flush_writes = 0: this function is called by write(), it should trigger shared thread
           always, but regardless if there is anything in outgoing buffer.
         */
        // if (!osal_ringbuf_is_empty(&shared->outgoing) || !flush_writes) {
            shared->trig_select = OS_TRUE;
            if (flush_writes) {
                shared->flush_writes = flush_writes;
            }
            if (shared->select_event) {
                osal_event_set(shared->select_event);
            }
        // }
    }
    os_unlock();
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
    osal_debug_assert(ssock->is_shared_socket);

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
    if (thiso->is_shared_socket) {
        for (c = thiso->list.head.first; c; c = next_c) {
            next_c = c->list.clink.next;
            c->status = OSAL_STATUS_STREAM_CLOSED;
            ioc_switchbox_set_select_event(c);
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

  @brief Make client handshake message (socket client only).
  @anchor ioc_send_client_handshake_message

  The ioc_generate_emulated_client_handshake_message() generates locally data to emulate a received
  handshare message. The function ioc_send_client_handshake_message() generates real client
  end hansshake message, this just empty framing without switchbox information to fill in.

  @param   thiso Pointer to the invidual emulated socket structure.

****************************************************************************************************
*/
/* static void ioc_generate_emulated_client_handshake_message(
    switchboxSocket *thiso)
{
    const os_char one_byte_handshake = IOC_HANDSHAKE_CLIENT;
    osal_ringbuf_put(&thiso->outgoing, &one_byte_handshake, 1);
} */


/**
****************************************************************************************************

  @brief Do first handshake for to connect to switchbox.
  @anchor ioc_first_handshake

  @param   thiso Pointer to the client socket service.
  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_switchbox_shared_socket_handshake(
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

    /* We need to receive authentication frame.
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

    return OSAL_SUCCESS;
}




/**
****************************************************************************************************

  @brief Read and write shared socket and move data.

  Receives data from client socket to incoming buffer and sends data from outgoing buffer to
  client socket.

  @param   thiso Pointer to the shared socket structure.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates that there is no error but
           nothing was done, value OSAL_WORK_DONE that work was done and more work may
           be there to do. All other nonzero values indicate a broken socket or other error.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_run_shared_socket(
    switchboxSocket *thiso,
    switchboxSocket **newsocket)
{
    switchboxSocket *c, *current_c, *next_c;
    switchboxSocket *news;
    os_int i, outbuf_space, bytes;
    osalStatus s;
    os_boolean work_done = OS_FALSE;
    os_short client_id;

    *newsocket = OS_NULL;

    /* Receive data from shared socket
     */
    s = ioc_read_from_shared_switchbox_socket(thiso);
    if (s == OSAL_WORK_DONE) {
        work_done = OS_TRUE;
    }
    else if (OSAL_IS_ERROR(s)) {
        return s;
    }

    /* Synchronize.
     */
    os_lock();

    /* Loop trough to find individual socket in turn to serve first.
     */
    current_c = OS_NULL;
    thiso->current_individual_socket_ix++;
    for (c = thiso->list.head.first, i = 0; c; c = c->list.clink.next, i++)
    {
        if (i == thiso->current_individual_socket_ix) {
            current_c = c;
        }
    }
    if (current_c == OS_NULL) {
        current_c = thiso->list.head.first;
    }

    /* Loop trough individual emulated sockets to move data to shared socket.
     */
    if (current_c) {
        c = current_c;
        do {
            /* If we do not have space in outgoing buffer for header + one byte,
               waste no time here.
             */
            outbuf_space = osal_ringbuf_space(&thiso->outgoing);
            if (outbuf_space < SBOX_HDR_SIZE + 1) continue;

            next_c = c->list.clink.next;
            if (next_c == OS_NULL) {
                next_c = thiso->list.head.first;
            }

            if (osal_ringbuf_is_empty(&c->incoming)) {
                goto nextcon;
            }

            bytes = osal_ringbuf_bytes(&c->incoming);
            if (bytes > outbuf_space - SBOX_HDR_SIZE) {
                bytes = outbuf_space - SBOX_HDR_SIZE;
            }
            ioc_switchbox_store_msg_header_to_ringbuf(&thiso->outgoing, c->client_id, bytes);
            ioc_switchbox_ringbuf_move(&thiso->outgoing, &c->incoming, bytes);
            work_done = OS_TRUE;
            ioc_switchbox_set_select_event(c);
nextcon:
            c = next_c;
        }
        while (c != current_c);
    }

    /* Move data from shared socket to client connections. If we have no data bytes to move
       from incoming shared socket, see first if we have message header.
     */
    if (thiso->incoming_bytes == 0) {
        s = ioc_switchbox_get_msg_header_from_ringbuf(&thiso->incoming, &client_id, &bytes);
        if (s == OSAL_SUCCESS) {
            if (bytes > 0) {
                thiso->incoming_client_id = client_id;
                thiso->incoming_bytes = bytes;
            }
            else switch(bytes) {
                case IOC_SWITCHBOX_NEW_CONNECTION:
                    news = (switchboxSocket*)os_malloc(sizeof(switchboxSocket), OS_NULL);
                    if (news == OS_NULL) {
                        os_unlock();
                        return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
                    }
                    os_memclear(news, sizeof(switchboxSocket));
                    news->client_id = client_id;
                    *newsocket = news;
                    break;

                case IOC_SWITCHBOX_CONNECTION_DROPPED:
                    for (c = thiso->list.head.first; c; c = c->list.clink.next)
                    {
                        if (c->client_id == thiso->incoming_client_id) {
                            // c->worker.stop_thread = OS_TRUE;
                            // c->connection_dropped_message_done = OS_TRUE;
                            ioc_switchbox_set_select_event(c);
                        }
                    }
                    break;

                default:
                    osal_debug_error_int("service con received unknown command ", bytes);
                    break;

            }
            work_done = OS_TRUE;
        }
    }

    /* If we have data bytes to move, do it.
     */
    if (thiso->incoming_bytes) {
        current_c = OS_NULL;
        for (c = thiso->list.head.first; c; c = c->list.clink.next)
        {
            if (c->client_id == thiso->incoming_client_id) {
                current_c = c;
                break;
            }
        }

        bytes = osal_ringbuf_bytes(&thiso->incoming);
        if (thiso->incoming_bytes < bytes) {
            bytes = thiso->incoming_bytes;
        }
        if (current_c) {
            i = osal_ringbuf_space(&current_c->outgoing);
            if (i < bytes) {
                bytes = i;
            }
            if (bytes) {
                ioc_switchbox_ringbuf_move(&current_c->outgoing, &thiso->incoming, bytes);
                thiso->incoming_bytes -= bytes;
                work_done = OS_TRUE;
                ioc_switchbox_set_select_event(current_c);
            }
        }
        else if (bytes) {
            /* Client connection dropped, drop received bytes away.
             */
            ioc_switchbox_ringbuf_skip_data(&thiso->incoming, bytes);

            thiso->incoming_bytes -= bytes;
            if (thiso->incoming_bytes == 0) {
                ioc_switchbox_store_msg_header_to_ringbuf(&thiso->outgoing,
                    thiso->incoming_client_id, IOC_SWITCHBOX_CONNECTION_DROPPED);
            }

            work_done = OS_TRUE;
        }
    }

    /* If something done, set event to come here again quickly.
     */
    os_unlock();

    /* Send data to shared socket
     */
    s = ioc_write_to_shared_switchbox_socket(thiso);
    if (s == OSAL_WORK_DONE) {
        work_done = OS_TRUE;
    }
    else if (OSAL_IS_ERROR(s)) {
        if (*newsocket) {
            os_free(*newsocket, sizeof(switchboxSocket));
        }
        *newsocket = OS_NULL;
        return s;
    }

    if (thiso->flush_writes) {
        thiso->flush_writes = OS_FALSE;
        osal_stream_flush(thiso->switchbox_stream, OSAL_STREAM_DEFAULT);
    }

    return work_done ? OSAL_WORK_DONE : OSAL_SUCCESS;
}




/**
****************************************************************************************************

  @brief Write data to shared socket connected to switchbox.
  @anchor ioc_write_to_shared_switchbox_socket

  Service socket only: Write data from outgoing ring buffer to shared socket.

  @param   thiso Stream pointer representing the listening socket.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates that there is no error but
           no data was written, value OSAL_WORK_DONE that some data was written. All other
           nonzero values indicate a broken socket.

****************************************************************************************************
*/
static osalStatus ioc_write_to_shared_switchbox_socket(
    switchboxSocket *thiso)
{
    os_memsz n_written;
    os_int n, tail;
    osalStatus s;

    osal_debug_assert(thiso->is_shared_socket);
    if (osal_ringbuf_is_empty(&thiso->outgoing)) {
        return OSAL_SUCCESS;
    }

    tail = thiso->outgoing.tail;
    n = osal_ringbuf_continuous_bytes(&thiso->outgoing);
    s = osal_stream_write(thiso->switchbox_stream, thiso->outgoing.buf + tail,
        n, &n_written, OSAL_STREAM_DEFAULT);
    if (s) {
        return s;
    }
    if (n_written == 0) {
        return OSAL_SUCCESS;
    }
    tail += (os_int)n_written;
    if (tail >= thiso->outgoing.buf_sz) {
        tail = 0;

        n = thiso->outgoing.head;
        if (n) {
            s = osal_stream_write(thiso->switchbox_stream, thiso->outgoing.buf + tail,
                n, &n_written, OSAL_STREAM_DEFAULT);
            if (s) {
                return s;
            }

            tail += (os_int)n_written;
        }
    }

    if (thiso->outgoing.tail == tail) {
        return OSAL_SUCCESS;
    }
    thiso->outgoing.tail = tail;
    return OSAL_WORK_DONE;
}


/**
****************************************************************************************************

  @brief Read data from shared socket connected to switchbox.
  @anchor ioc_read_from_shared_switchbox_socket

  Service socket only: Read data from shared socket to incoming ring buffer.

  @param   thiso Stream pointer representing the listening socket.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates that there is no error but
           nothing was done. Value OSAL_WORK_DONE is work was done and more work may
           be there to do. All other nonzero values indicate a broken socket.

****************************************************************************************************
*/
static osalStatus ioc_read_from_shared_switchbox_socket(
    switchboxSocket *thiso)
{
    os_memsz n_read;
    os_int n, head;
    osalStatus s;

    osal_debug_assert(thiso->is_shared_socket);
    if (osal_ringbuf_is_full(&thiso->incoming)) {
        return OSAL_SUCCESS;
    }

    head = thiso->incoming.head;
    n = osal_ringbuf_continuous_space(&thiso->incoming);
    s = osal_stream_read(thiso->switchbox_stream, thiso->incoming.buf + head,
        n, &n_read, OSAL_STREAM_DEFAULT);
    if (s) {
        return s;
    }
    if (n_read == 0) {
        return OSAL_SUCCESS;
    }
    head += (os_int)n_read;
    if (head >= thiso->incoming.buf_sz) {
        head = 0;

        n = thiso->incoming.tail - 1;
        if (n > 0) {
            s = osal_stream_read(thiso->switchbox_stream, thiso->incoming.buf + head,
                n, &n_read, OSAL_STREAM_DEFAULT);
            if (s) {
                return s;
            }

            head += (os_int)n_read;
        }
    }

    if (thiso->incoming.head == head) {
        return OSAL_SUCCESS;
    }

    thiso->incoming.head = head;
    return OSAL_WORK_DONE;
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

  @brief Set up ring buffers for outgoing and incoming data.
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
    switchboxSocket *thiso)
{
    os_memsz sz, buf_sz;

    os_memclear(&thiso->incoming, sizeof(osalRingBuf));
    sz = 3000;
    thiso->incoming.buf = (os_char*)os_malloc(sz, &buf_sz);
    thiso->incoming.buf_sz = (os_int)buf_sz;
    if (thiso->incoming.buf == OS_NULL) {
        return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    }

    os_memclear(&thiso->outgoing, sizeof(osalRingBuf));
    sz = 3000;
    thiso->outgoing.buf = (os_char*)os_malloc(sz, &buf_sz);
    thiso->outgoing.buf_sz = (os_int)buf_sz;
    if (thiso->outgoing.buf == OS_NULL) {
        os_free(thiso->incoming.buf, thiso->incoming.buf_sz);
        return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    }
    return OSAL_SUCCESS;
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
