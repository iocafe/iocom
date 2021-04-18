/**

  @file    switchbox_connection.c
  @brief   Switchbox connection object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "switchbox.h"


/* Forward referred static functions.
 */
static void ioc_switchbox_connection_thread(
    void *prm,
    osalEvent done);

static void ioc_switchbox_close_stream(
    switchboxConnection *con);

static osalStatus ioc_switchbox_handshake_and_authentication(
    switchboxConnection *con);

static osalStatus ioc_switchbox_setup_service_connection(
    switchboxConnection *con);

static osalStatus ioc_switchbox_write_socket(
    switchboxConnection *con);

static osalStatus ioc_switchbox_read_socket(
    switchboxConnection *con);

static osalStatus ioc_switchbox_setup_client_connection(
    switchboxConnection *con);

static osalStatus ioc_switchbox_service_con_run(
    switchboxConnection *scon);

static osalStatus ioc_switchbox_client_run(
    switchboxConnection *ccon);

static void ioc_switchbox_link_connection(
    switchboxConnection *con,
    switchboxConnection *scon);

static void ioc_switchbox_unlink_connection(
    switchboxConnection *con);


/**
****************************************************************************************************

  @brief Initialize connection.
  @anchor ioc_initialize_switchbox_connection

  The ioc_initialize_switchbox_connection() function initializes a connection. A connection can always
  be allocated global variable. In this case pointer to memory to be initialized is given as
  argument and return value is the same pointer. If  the con argument is OS_NULL, memory for
  the connection object is allocated by the function.

  @param   con Pointer to static connection structure, or OS_NULL to allocate connection
           object dynamically or from pool.
  @param   root Pointer to the root object.
  @return  Pointer to initialized connection object. OE_NULL if the function failed (memory
           allocation fails).

****************************************************************************************************
*/
switchboxConnection *ioc_initialize_switchbox_connection(
    switchboxRoot *root)
{
    switchboxConnection *con;

    /* Synchronize.
     */
    ioc_switchbox_lock(root);

    con = (switchboxConnection*)os_malloc(sizeof(switchboxConnection), OS_NULL);
    if (con == OS_NULL)
    {
        ioc_switchbox_unlock(root);
        return OS_NULL;
    }
    os_memclear(con, sizeof(switchboxConnection));

    /* Save pointer to root object and join to linked list of connections.
     */
    con->link.root = root;
    con->link.prev = root->con.last;
    if (root->con.last)
    {
        root->con.last->link.next = con;
    }
    else
    {
        root->con.first = con;
    }
    root->con.last = con;

    ioc_initialize_handshake_state(&con->handshake);
    con->handshake_ready = OS_FALSE;

    /* End syncronization.
     */
    ioc_switchbox_unlock(root);

    /* Return pointer to initialized connection.
     */
    osal_trace("connection: initialized");
    return con;
}


/**
****************************************************************************************************

  @brief Release connection.
  @anchor ioc_release_switchbox_connection

  The ioc_release_switchbox_connection() function releases resources allocated for the connection
  object. Memory allocated for the connection object is freed, if it was allocated by
  ioc_initialize_switchbox_connection().

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_switchbox_connection(
    switchboxConnection *con)
{
    switchboxRoot
        *root;

    /* Synchronize.
     */
    root = con->link.root;
    ioc_switchbox_lock(root);

    /* If stream is open, close it.
     */
    ioc_switchbox_close_stream(con);

    /* Remove connection from linked list.
     */
    if (con->link.prev)
    {
        con->link.prev->link.next = con->link.next;
    }
    else
    {
        con->link.root->con.first = con->link.next;
    }
    if (con->link.next)
    {
        con->link.next->link.prev = con->link.prev;
    }
    else
    {
        con->link.root->con.last = con->link.prev;
    }

    /* Release hand shake structure.
     */
    ioc_release_handshake_state(&con->handshake);

    /* Release authentication buffers, if any.
     */
    if (con->auth_send_buf) {
        os_free(con->auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        con->auth_send_buf = OS_NULL;
    }
    if (con->auth_recv_buf) {
        os_free(con->auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
        con->auth_recv_buf = OS_NULL;
    }

    os_free(con->incoming.buf, con->incoming.buf_sz);
    os_free(con->outgoing.buf, con->outgoing.buf_sz);
    os_free(con, sizeof(switchboxConnection));

    /* End syncronization.
     */
    ioc_switchbox_unlock(root);
    osal_trace("connection: released");
}



/**
****************************************************************************************************

  @brief Start or prepare the connection.
  @anchor ioc_switchbox_connect

  The ioc_switchbox_connect() function sets up for socket or serial connection. Actual socket
  or serial port is opened when connection runs.

  @param   con Pointer to the connection object.
  @param   prm Parameter structure. Clear parameter structure using os_memclear() and
           set the members needed. Members:
           - parameters Depending on connection type, can be "127.0.0.1:8817" for TCP socket.
           - newsocket If socket connection is accepted by listening end point, this is
             the socket handle. Otherwise this argument needs to be OS_NULL.
           - frame_out_buf Pointer to static frame buffer. OS_NULL to allocate the frame buffer.
           - frame_out_buf_sz Size of static frame buffer, either IOC_SOCKET_FRAME_SZ or
             IOC_SERIAL_FRAME_SZ. Leave to zero for frame buffer allocated by this call.
           - frame_in_buf Pointer to static frame buffer. OS_NULL to allocate the frame buffer.
           - frame_in_buf_sz Size of static frame buffer, either IOC_SOCKET_FRAME_SZ or
             IOC_SERIAL_FRAME_SZ. Leave to zero for frame buffer allocated by this call.
           - flags Bit fields: IOC_SOCKET Connect with TCP socket. IOC_CREATE_THREAD Create
             thread to run connection (multithread support needed).

  @return  OSAL_SUCCESS if successful. Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_switchbox_connect(
    switchboxConnection *con,
    switchboxConnectionParams *prm)
{
    switchboxRoot *root;

    osalThreadOptParams opt;

    root = con->link.root;
    ioc_switchbox_lock(root);

    /* If we are already running connection, stop it. Wait until it has stopped.
     */
    while (ioc_terminate_switchbox_connection_thread(con))
    {
        ioc_switchbox_unlock(root);
        os_timeslice();
        ioc_switchbox_lock(root);
    }

//     os_strncpy(con->parameters, prm->parameters, IOC_CONNECTION_PRMSTR_SZ);


    /* If this is incoming TCP socket accepted by end point?
     */
    if (prm->newsocket)
    {
        con->stream = prm->newsocket;
//        con->flags |= IOC_CLOSE_CONNECTION_ON_ERROR|IOC_LISTENER;

        /* Reset connection state
         */
        ioc_reset_switchbox_connection(con);
    }

    /* If we want to run end point in separate thread.
     */
    con->worker.trig = osal_event_create(OSAL_EVENT_SET_AT_EXIT);
    con->worker.thread_running = OS_TRUE;
    con->worker.stop_thread = OS_FALSE;

    os_memclear(&opt, sizeof(opt));
    opt.thread_name = "connection";
    /* opt.stack_size = 16000; */
    opt.pin_to_core = OS_TRUE;
    opt.pin_to_core_nr = 0;

    osal_thread_create(ioc_switchbox_connection_thread, con,
        &opt, OSAL_THREAD_DETACHED);

    ioc_switchbox_unlock(root);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Request to terminate connection worker thread.
  @anchor ioc_terminate_switchbox_connection_thread

  The ioc_terminate_switchbox_connection_thread() function sets request to terminate worker thread, if
  one is running the end point.

  ioc_switchbox_lock() must be on when this function is called.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if no worker thread is running. OSAL_PENDING if worker
           thread is still running.

****************************************************************************************************
*/
osalStatus ioc_terminate_switchbox_connection_thread(
    switchboxConnection *con)
{
    osalStatus
        status = OSAL_SUCCESS;

    if (con->worker.thread_running)
    {
        con->worker.stop_thread = OS_TRUE;
        if (con->worker.trig) osal_event_set(con->worker.trig);
        status = OSAL_PENDING;
    }

    return status;
}


/**
****************************************************************************************************

  @brief Reset connection state to start from beginning
  @anchor ioc_reset_switchbox_connection

  The ioc_reset_switchbox_connection() function resets connection state and connected
  source and destination buffers.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_reset_switchbox_connection(
    switchboxConnection *con)
{
    os_timer tnow;

    /* Reset hand shake structure.
     */
    ioc_release_handshake_state(&con->handshake);
    con->handshake_ready = OS_FALSE;
    con->authentication_sent = OS_FALSE;
    con->authentication_received = OS_FALSE;

    /* Initialize timers.
     */
    os_get_timer(&tnow);
    con->last_receive = tnow;
    con->last_send = tnow;
}


/**
****************************************************************************************************

  @brief Close underlying socket or serial port.
  @anchor ioc_close_stream

  The ioc_close_stream() function closes the communication stream. For serial communication it
  clears the d
  source and destination buffers.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
static void ioc_switchbox_close_stream(
    switchboxConnection *con)
{
    if (con->stream)
    {
        osal_trace2("stream closed");
        osal_stream_close(con->stream, OSAL_STREAM_DEFAULT);
        con->stream = OS_NULL;
    }
}


/**
****************************************************************************************************

  @brief Connection worker thread function.
  @anchor ioc_switchbox_connection_thread

  The ioc_switchbox_connection_thread() function is worker thread to connect a socket (optional) and
  transfer data trough it.

  @param   prm Pointer to parameters for new thread, pointer to end point object.
  @param   done Event to set when parameters have been copied to entry point
           functions own memory.

****************************************************************************************************
*/
static void ioc_switchbox_connection_thread(
    void *prm,
    osalEvent done)
{
    switchboxRoot *root;
    switchboxConnection *con;
    osalStatus s;
    osalSelectData selectdata;
    os_timer tnow;
    os_int silence_ms;

    /* Parameters point to the connection object.
     */
    con = (switchboxConnection*)prm;
    root = con->link.root;

    /* Let thread which created this one proceed.
     */
    osal_event_set(done);

    osal_trace("connection: worker thread started");

    /* Select of time interval how often to check for timeouts, etc.
       Serial connections need to be monitored much more often than
       sockets to detect partically received frames.
     */
    silence_ms = IOC_SOCKET_SILENCE_MS;
    os_memclear(&selectdata, sizeof(selectdata));

    /* Run the connection.
     */
    while (!con->worker.stop_thread && osal_go())
    {
// static long ulledoo; if (++ulledoo > 109) {osal_debug_error("ulledoo connection\n"); ulledoo = 0;}

        /* If stream is not open, then connect it now. Do not try if two secons have not
           passed since last failed open try.
         */

        s = osal_stream_select(&con->stream, 1, con->worker.trig,
            &selectdata, IOC_SOCKET_CHECK_TIMEOUTS_MS, OSAL_STREAM_DEFAULT);

        if (s == OSAL_STATUS_NOT_SUPPORTED)
        {
            os_timeslice();
        }

        else if (s)
        {
            osal_debug_error("osal_stream_select failed");
            break;
        }
        os_get_timer(&tnow);

        /* First hand shake for socket connections.
         */
        s = ioc_switchbox_handshake_and_authentication(con);
        if (s == OSAL_PENDING) {
            continue;
        }
        if (s) {
            break;
        }

        /* Run the connection.
         */
        if (con->is_service_connection) {
            do {
                s = ioc_switchbox_service_con_run(con);
            }
            while (s == OSAL_WORK_DONE && !con->worker.stop_thread && osal_go());
        }
        else {
            s = ioc_switchbox_client_run(con);
        }
        if (OSAL_IS_ERROR(s)) {
            osal_debug_error_int("switchbox run error: ", s);
            break;
        }

        /* If too much time elapsed sice last receive?
         */
        if (os_has_elapsed_since(&con->last_receive, &tnow, silence_ms))
        {
            osal_trace("line is silent, closing connection");
            // break;
        }

        /* Flush data to the connection.
         */
        if (con->stream) {
            osal_stream_flush(con->stream, OSAL_STREAM_DEFAULT);
        }
    }

    /* Closing connection, close first the stream.
     */
    ioc_switchbox_close_stream(con);

    /* Unlink connection, delete trigger event and mark that this thread is no longer running.
     */
    ioc_switchbox_lock(root);
    ioc_switchbox_unlink_connection(con);
    osal_event_delete(con->worker.trig);
    con->worker.trig = OS_NULL;
    con->worker.thread_running = OS_FALSE;

    ioc_release_switchbox_connection(con);
    ioc_switchbox_unlock(root);

    osal_trace("switchbox: worker thread exited");
}

/* Load certificate (server only).
 */
static os_memsz ioc_switchbox_load_iocom_trust_certificate(
    const os_uchar *cert_buf,
    os_memsz cert_buf_sz,
    void *context)
{
    return 0;
}


/**
****************************************************************************************************

  @brief Do first handshake for the connection (only sockets).
  @anchor ioc_first_handshake

  Socket handshake for switchbox cloud network name and trusted certificate copy

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_switchbox_handshake_and_authentication(
    switchboxConnection *con)
{
    osalStatus s;
    iocHandshakeClientType htype;

    if (con->handshake_ready && (!con->is_service_connection ||
        (con->authentication_received && con->authentication_sent) ))
    {
        return OSAL_SUCCESS;
    }

    if (!con->handshake_ready)
    {
        s = ioc_server_handshake(&con->handshake, IOC_HANDSHAKE_SWITCHBOX_SERVER,
            con->stream, ioc_switchbox_load_iocom_trust_certificate, con);

        osal_stream_flush(con->stream, OSAL_STREAM_DEFAULT);

        if (s) {
            return s;
        }

        os_strncpy(con->network_name,
            ioc_get_handshake_cloud_netname(&con->handshake), IOC_NETWORK_NAME_SZ);

        if (con->network_name[0] == '\0') {
            osal_debug_error("switchbox: incoming connection without network name");
            return OSAL_STATUS_FAILED;
        }
        con->handshake_ready = OS_TRUE;
    }

    htype = ioc_get_handshake_client_type(&con->handshake);

    /* If this is service connection, handle authentication. For client connections,
       handling authentication belongs to the IO network service.
     */
    if (htype == IOC_HANDSHAKE_NETWORK_SERVICE)
    {
        /* Service connection: We need to receive authentication frame.
         */
        if (!con->authentication_received) {
            if (con->auth_recv_buf == OS_NULL) {
                con->auth_recv_buf = (iocSwitchboxAuthenticationFrameBuffer*)
                    os_malloc(sizeof(iocSwitchboxAuthenticationFrameBuffer), OS_NULL);
                os_memclear(con->auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
            }

            iocAuthenticationResults results;
            s = icom_switchbox_process_authentication_frame(con->stream, con->auth_recv_buf, &results);
            if (s == OSAL_COMPLETED) {
                os_free(con->auth_recv_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
                con->auth_recv_buf = OS_NULL;
                con->authentication_received = OS_TRUE;
            }
            else if (s != OSAL_PENDING) {
                osal_debug_error("eConnection: Valid authentication frame was not received");
                return OSAL_STATUS_FAILED;
            }
        }

        /* Service connection: We need to send responce(s).
         */
        if (!con->authentication_sent)
        {
            iocSwitchboxAuthenticationParameters prm;
            os_memclear(&prm, sizeof(prm));

            if (con->auth_send_buf == OS_NULL) {
                con->auth_send_buf = (iocSwitchboxAuthenticationFrameBuffer*)
                    os_malloc(sizeof(iocSwitchboxAuthenticationFrameBuffer), OS_NULL);
                os_memclear(con->auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));

                prm.network_name = "sb";
                prm.user_name = "srv";
                prm.password = "pw";
            }

            s = ioc_send_switchbox_authentication_frame(con->stream, con->auth_send_buf, &prm);
            if (s == OSAL_COMPLETED) {
                os_free(con->auth_send_buf, sizeof(iocSwitchboxAuthenticationFrameBuffer));
                con->auth_send_buf = OS_NULL;
                con->authentication_sent = OS_TRUE;
                osal_stream_flush(con->stream, OSAL_STREAM_DEFAULT);
            }
            else if (s != OSAL_PENDING) {
                osal_debug_error("eConnection: Failed to send authentication frame");
                return OSAL_STATUS_FAILED;
            }
        }

        if (!con->authentication_sent ||
            !con->authentication_received)
        {
            os_timeslice();
            osal_stream_flush(con->stream, OSAL_STREAM_DEFAULT);
            return OSAL_PENDING;
        }
    }

    switch (htype)
    {
        case IOC_HANDSHAKE_NETWORK_SERVICE:
            s = ioc_switchbox_setup_service_connection(con);
            break;

        case IOC_HANDSHAKE_CLIENT:
            s = ioc_switchbox_setup_client_connection(con);
            break;

        default:
            s = OSAL_STATUS_FAILED;
            osal_debug_error("switchbox: unknown incoming connection type");
            break;
    }

    return s;
}


/**
****************************************************************************************************

  @brief Setup a connection to IO service.

  Xy

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if successfull, other return values indicate a failue.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_setup_service_connection(
    switchboxConnection *con)
{
    switchboxRoot *root;
    switchboxConnection *scon;
    osalStatus s = OSAL_STATUS_FAILED;

    root = con->link.root;
    ioc_switchbox_lock(root);

    con->is_service_connection = OS_TRUE;

    /* If we have service connection with this name, kill it and fail for now.
     */
    scon = ioc_switchbox_find_service_connection(root, con->network_name, con);
    if (scon) {
        scon->worker.stop_thread = OS_TRUE;
        osal_event_set(scon->worker.trig);
        osal_debug_error_str("switchbox: service already connected, killing ", scon->network_name);
        goto getout;
    }

    s = OSAL_SUCCESS;

getout:
    ioc_switchbox_unlock(root);
    return s;
}


/**
****************************************************************************************************

  @brief Setup a connection to client.

  Xy

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if successfull, other return values indicate a failue.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_setup_client_connection(
    switchboxConnection *con)
{
    switchboxRoot *root;
    switchboxConnection *scon;
    osalStatus s = OSAL_STATUS_FAILED;;

    root = con->link.root;
    ioc_switchbox_lock(root);

    /* If we have no service connection with this name, we fail.
     */
    scon = ioc_switchbox_find_service_connection(root, con->network_name, OS_NULL);
    if (scon == OS_NULL) {
        osal_debug_error_str("switchbox: no service connection for ", con->network_name);
        goto getout;
    }

    /* Set client identifier.
     */
    con->client_id = ioc_new_switchbox_client_id(con->link.root);

    /* Join client connection to list of service connection.
     */
    ioc_switchbox_link_connection(con, scon);
    s = OSAL_SUCCESS;

getout:
    ioc_switchbox_unlock(root);
    return s;
}


/**
****************************************************************************************************

  @brief Write data to switchbox socket.
  @anchor ioc_switchbox_write_socket

  Write data from outgoing ring buffer to socket.

  @param   thiso Stream pointer representing the listening socket.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates that there is no error but
           no data was written, value OSAL_WORK_DONE that some data was written. All other
           nonzero values indicate a broken socket.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_write_socket(
    switchboxConnection *con)
{
    os_memsz n_written;
    os_int n, tail;
    osalStatus s;

    if (osal_ringbuf_is_empty(&con->outgoing)) {
        return OSAL_SUCCESS;
    }

    tail = con->outgoing.tail;
    n = osal_ringbuf_continuous_bytes(&con->outgoing);
    s = osal_socket_write(con->stream, con->outgoing.buf + tail,
        n, &n_written, OSAL_STREAM_DEFAULT);
    if (s) {
        return s;
    }
    if (n_written == 0) {
        return OSAL_SUCCESS;
    }
    tail += (os_int)n_written;
    if (tail >= con->outgoing.buf_sz) {
        tail = 0;

        n = con->outgoing.head;
        if (n) {
            s = osal_socket_write(con->stream, con->outgoing.buf + tail,
                n, &n_written, OSAL_STREAM_DEFAULT);
            if (s) {
                return s;
            }

            tail += (os_int)n_written;
        }
    }

    if (con->outgoing.tail == tail) {
        return OSAL_SUCCESS;
    }
    con->outgoing.tail = tail;
    return OSAL_WORK_DONE;
}


/**
****************************************************************************************************

  @brief Read data from switchbox socket.
  @anchor ioc_switchbox_read_socket

  Read data from socket to incoming ring buffer.

  @param   thiso Stream pointer representing the listening socket.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates that there is no error but
           no new data was read, value OSAL_WORK_DONE that data was read. All other nonzero
           values indicate a broken socket.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_read_socket(
    switchboxConnection *con)
{
    os_memsz n_read;
    os_int n, head;
    osalStatus s;

    if (osal_ringbuf_is_full(&con->incoming)) {
        return OSAL_SUCCESS;
    }

    head = con->incoming.head;
    n = osal_ringbuf_continuous_space(&con->incoming);
    s = osal_socket_read(con->stream, con->incoming.buf + head,
        n, &n_read, OSAL_STREAM_DEFAULT);
    if (s) {
        return s;
    }
    if (n_read == 0) {
        return OSAL_SUCCESS;
    }
    head += (os_int)n_read;
    if (head >= con->incoming.buf_sz) {
        head = 0;

        n = con->incoming.tail - 1;
        if (n > 0) {
            s = osal_socket_read(con->stream, con->incoming.buf + head,
                n, &n_read, OSAL_STREAM_DEFAULT);
            if (s) {
                return s;
            }

            head += (os_int)n_read;
        }
    }

    if (con->incoming.head == head) {
        return OSAL_SUCCESS;
    }
    con->incoming.head = head;
    return OSAL_WORK_DONE;
}



/**
****************************************************************************************************

  @brief Read data from switchbox socket.
  @anchor ioc_switchbox_read_socket

  Read data from socket to incoming ring buffer.

  @param   thiso Stream pointer representing the listening socket.
  @return  Function status code. Value OSAL_SUCCESS (0) indicates that there is no error but
           no new data was read, value OSAL_WORK_DONE that data was read. All other nonzero
           values indicate a broken socket.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_service_con_run(
    switchboxConnection *scon)
{
    switchboxRoot *root;
    switchboxConnection *c, *current_c, *next_c;
    os_int i, outbuf_space, bytes;
    osalStatus s;
    os_boolean work_done = OS_FALSE;

    /* Receive data from shared socket
     */
    s = ioc_switchbox_read_socket(scon);
    if (s == OSAL_WORK_DONE) {
        work_done = OS_TRUE;
    }
    else if (OSAL_IS_ERROR(s)) {
        return s;
    }

    /* Synchronize.
     */
    root = scon->link.root;
    ioc_switchbox_lock(root);

    /* loop trough client connections to generate new connection messages.
     * and find current client connection.
     */
    current_c = OS_NULL;
    scon->current_connection_ix++;
    for (c = scon->list.head.first, i = 0; c; c = c->list.clink.next, i++)
    {
        if (!c->new_connection_msg_sent) {
            s = ioc_switchbox_store_msg_header_to_ringbuf(&scon->outgoing, c->client_id, IOC_SWITCHBOX_NEW_CONNECTION);
            if (s != OSAL_SUCCESS) {
                continue;
            }
            c->new_connection_msg_sent = OS_TRUE;
            work_done = OS_TRUE;
        }

        if (i == scon->current_connection_ix) {
            current_c = c;
        }
    }
    if (current_c == OS_NULL) {
        for (c = scon->list.head.first; c; c = c->list.clink.next) {
            if (c->new_connection_msg_sent) {
                scon->current_connection_ix = 0;
                current_c = c;
                break;
            }
        }
    }

    /* loop trough client connections to read data from clients.
     */
    if (current_c) {
        c = current_c;
        do {
            /* If we do not have space in outgoing buffer for header + one byte,
               waste no time here.
             */
            outbuf_space = osal_ringbuf_space(&scon->outgoing);
            if (outbuf_space < SBOX_HDR_SIZE + 1) continue;

            next_c = c->list.clink.next;
            if (next_c == OS_NULL) {
                next_c = scon->list.head.first;
            }

            if (!c->new_connection_msg_sent) {
                goto nextcon;
            }

            if (osal_ringbuf_is_empty(&c->incoming)) {
                goto nextcon;
            }

            bytes = osal_ringbuf_bytes(&c->incoming);
            if (bytes > outbuf_space - SBOX_HDR_SIZE) {
                bytes = outbuf_space - SBOX_HDR_SIZE;
            }
            ioc_switchbox_store_msg_header_to_ringbuf(&scon->outgoing, c->client_id, bytes);
            ioc_switchbox_ringbuf_move(&scon->outgoing, &c->incoming, bytes);
            work_done = OS_TRUE;

nextcon:
            c = next_c;
        }
        while (c != current_c);
    }

    /* move data from shared socket to client connections.
     */
    // If we have no data bytes to move, see if we have message header


    // If we have data bytes to move, move data

    // loop trough client connections


    /* If something done, set event to come here again quickly
     */
    ioc_switchbox_unlock(root);

    /* Send data to shared socket
     */
    s = ioc_switchbox_write_socket(scon);
    if (s == OSAL_WORK_DONE) {
        work_done = OS_TRUE;
    }
    else if (OSAL_IS_ERROR(s)) {
        return s;
    }

    return work_done ? OSAL_WORK_DONE : OSAL_SUCCESS;
}


static osalStatus ioc_switchbox_client_run(
    switchboxConnection *ccon)
{
    switchboxRoot
        *root;

    /* Receive data from shared socket
     */

    /* Synchronize.
     */
    root = ccon->link.root;
    ioc_switchbox_lock(root);





    // loop trough client connections

        // if "new connection sent"
            // if client has unsent data, send it

    // ioc_switchbox_unlock(root);

    /* Send data to shared socket
     */

    /* If something done, set event to come here again quickly
     */


    ioc_switchbox_unlock(root);
    return OSAL_SUCCESS;
}

/**
****************************************************************************************************

  @brief Add a client connection to service connection's linked list.

  Note: ioc_switchbox_lock must be on when this function is called.

  @param   con Pointer to the client connection object.
  @param   scon Pointer to the service connection object.

****************************************************************************************************
*/
static void ioc_switchbox_link_connection(
    switchboxConnection *con,
    switchboxConnection *scon)
{
    osal_debug_assert(scon->is_service_connection);

    /* Join to list of client connections for the server connection.
     */
    con->list.clink.prev = scon->list.head.last;
    con->list.clink.next = OS_NULL;
    con->list.clink.scon = scon;
    if (scon->list.head.last) {
        scon->list.head.last->list.clink.next = con;
    }
    else {
        scon->list.head.first = con;
    }
    scon->list.head.last = con;
}


/**
****************************************************************************************************

  @brief Remove connection from service connection's linked list.

  If con is service connection, all client connections are unlinked and requested to terminate.

  Note: ioc_switchbox_lock must be on when this function is called.

  @param   con Pointer to the connection object to unlink. Can be service or client connection.

****************************************************************************************************
*/
static void ioc_switchbox_unlink_connection(
    switchboxConnection *con)
{
    switchboxConnection *c, *next_c, *scon;

    /* con is service connection.
     */
    if (con->is_service_connection) {
        for (c = con->list.head.first; c; c = next_c) {
            next_c = c->list.clink.next;
            c->worker.stop_thread = OS_TRUE;
            osal_event_set(c->worker.trig);
            c->list.clink.next = c->list.clink.prev = c->list.clink.scon = OS_NULL;
        }
        con->list.head.first = con->list.head.last = OS_NULL;
    }

    /* con is client connection.
     */
    else {
        scon = con->list.clink.scon;
        if (scon) {
            if (con->list.clink.prev) {
                con->list.clink.prev->list.clink.next = con->list.clink.next;
            }
            else {
                scon->list.head.first = con->list.clink.next;
            }
            if (con->list.clink.next) {
                con->list.clink.next->list.clink.prev = con->list.clink.prev;
            }
            else {
                scon->list.head.last = con->list.clink.prev;
            }
            con->list.clink.next = con->list.clink.prev = con->list.clink.scon = OS_NULL;
        }
    }
}
