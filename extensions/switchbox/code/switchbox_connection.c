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

static osalStatus ioc_switchbox_first_handshake(
    switchboxConnection *con);

static osalStatus ioc_switchbox_setup_service_connection(
    switchboxConnection *con);

static osalStatus ioc_switchbox_setup_client_connection(
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

    /* Release memory allocated for allowed_networks.
     */
/* #if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
    ioc_release_allowed_networks(&con->allowed_networks);
#endif */

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
//    const os_char *parameters;
    osalStatus status;
    osalSelectData selectdata;
    os_timer tnow;
    os_int silence_ms, count;

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
// static long ulledoo; if (++ulledoo > 10009) {osal_debug_error("ulledoo connection\n"); ulledoo = 0;}

        /* If stream is not open, then connect it now. Do not try if two secons have not
           passed since last failed open try.
         */

        status = osal_stream_select(&con->stream, 1, con->worker.trig,
            &selectdata, IOC_SOCKET_CHECK_TIMEOUTS_MS, OSAL_STREAM_DEFAULT);

        if (status == OSAL_STATUS_NOT_SUPPORTED)
        {
            os_timeslice();
        }

        else if (status)
        {
            osal_debug_error("osal_stream_select failed");
            break;
        }
        os_get_timer(&tnow);

        /* First hand shake for socket connections.
         */
        status = ioc_switchbox_first_handshake(con);
        if (status == OSAL_PENDING) {
            continue;
        }
        if (status) {
            break;
        }

        /* Receive and send in loop as long as we can without waiting.
           How ever fast we write, we cannot block here (count=32) !
         */
        count = 32;
        while (count--)
        {
            while (osal_go())
            {
                /* Try receiving data from the connection.
                 */
                /* status = ioc_connection_receive(con);
                if (status == OSAL_PENDING)
                {
                    break;
                }
                if (status)
                {
                    goto failed;
                } */

                /* Record timer of last successful receive.
                 */
                con->last_receive = tnow;
            }

            /* Try sending data though the connection.
             */
            /* status = ioc_connection_send(con);
            if (status == OSAL_PENDING)
            {
                break;
            }
            if (status)
            {
                goto failed;
            } */
        }

        /* If too much time elapsed sice last receive?
         */
        if (os_has_elapsed_since(&con->last_receive, &tnow, silence_ms))
        {
            osal_trace("line is silent, closing connection");
            break;
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

    /* Delete trigger event and mark that this thread is no longer running.
     */
    ioc_switchbox_lock(root);
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
static osalStatus ioc_switchbox_first_handshake(
    switchboxConnection *con)
{
    osalStatus s = OSAL_SUCCESS;

    if (!con->handshake_ready)
    {
        s = ioc_server_handshake(&con->handshake, IOC_HANDSHAKE_SWITCHBOX_SERVER,
            con->stream, ioc_switchbox_load_iocom_trust_certificate, con);

        osal_stream_flush(con->stream, OSAL_STREAM_DEFAULT);

        if (s) {
            return s;
        }

        switch (ioc_get_handshake_client_type(&con->handshake))
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

        ioc_release_handshake_state(&con->handshake);
        con->handshake_ready = OS_TRUE;
    }

    return s;
}


/**
****************************************************************************************************

  @brief X

  Xy

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_switchbox_setup_service_connection(
    switchboxConnection *con)
{
    return OSAL_STATUS_FAILED;
}

static osalStatus ioc_switchbox_setup_client_connection(
    switchboxConnection *con)
{
    return OSAL_STATUS_FAILED;
}
