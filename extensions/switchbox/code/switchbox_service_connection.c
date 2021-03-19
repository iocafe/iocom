/**

  @file    switchbox_service_connection.c
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

static void ioc_connection_thread(
    void *prm,
    osalEvent done);


/**
****************************************************************************************************

  @brief Initialize connection.
  @anchor ioc_initialize_switchbox_service_connection

  The ioc_initialize_switchbox_service_connection() function initializes a connection. A connection can always
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
switchboxServiceConnection *ioc_initialize_switchbox_service_connection(
    switchboxServiceConnection *con,
    switchboxRoot *root)
{
    /* Synchronize.
     */
    ioc_switchbox_lock(root);

    if (con == OS_NULL)
    {
        con = (switchboxServiceConnection*)os_malloc(sizeof(switchboxServiceConnection), OS_NULL);
        if (con == OS_NULL)
        {
            ioc_switchbox_unlock(root);
            return OS_NULL;
        }

        os_memclear(con, sizeof(switchboxServiceConnection));
        con->allocated = OS_TRUE;
    }
    else
    {
        os_memclear(con, sizeof(switchboxServiceConnection));
    }

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
  @anchor ioc_release_switchbox_service_connection

  The ioc_release_switchbox_service_connection() function releases resources allocated for the connection
  object. Memory allocated for the connection object is freed, if it was allocated by
  ioc_initialize_switchbox_service_connection().

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_switchbox_service_connection(
    switchboxServiceConnection *con)
{
    switchboxRoot
        *root;

    os_boolean
        allocated;

    /* Synchronize.
     */
    root = con->link.root;
    ioc_switchbox_lock(root);

    /* If stream is open, close it.
     */
    ioc_close_switchbox_service_stream(con);

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


    /* Release memory allocated for allowed_networks.
     */
/* #if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
    ioc_release_allowed_networks(&con->allowed_networks);
#endif */

    /* Clear allocated memory indicate that is no longer initialized (for debugging and
       for primitive static allocation schema). Save allocated flag before memclear.
     */
    allocated = con->allocated;
    os_memclear(con, sizeof(switchboxServiceConnection));

    if (allocated)
    {
        os_free(con, sizeof(switchboxServiceConnection));
    }

    /* End syncronization.
     */
    ioc_switchbox_unlock(root);
    osal_trace("connection: released");
}



/**
****************************************************************************************************

  @brief Start or prepare the connection.
  @anchor ioc_switchbox_service_connect

  The ioc_switchbox_service_connect() function sets up for socket or serial connection. Actual socket
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
osalStatus ioc_switchbox_service_connect(
    switchboxServiceConnection *con,
    switchboxServiceConnectionParams *prm)
{
    switchboxRoot *root;

    osalThreadOptParams opt;

    root = con->link.root;
    ioc_switchbox_lock(root);

    /* If we are already running connection, stop it. Wait until it has stopped.
     */
    while (ioc_terminate_switchbox_service_connection_thread(con))
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
        ioc_reset_switchbox_service_connection(con);
    }

    /* If we want to run end point in separate thread.
     */
    con->worker.trig = osal_event_create();
    con->worker.thread_running = OS_TRUE;
    con->worker.stop_thread = OS_FALSE;

    os_memclear(&opt, sizeof(opt));
    opt.thread_name = "connection";
    /* opt.stack_size = 16000; */
    opt.pin_to_core = OS_TRUE;
    opt.pin_to_core_nr = 0;

    osal_thread_create(ioc_connection_thread, con,
        &opt, OSAL_THREAD_DETACHED);

    ioc_switchbox_unlock(root);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Connect and move data.
  @anchor ioc_run_switchbox_connection

  The ioc_run_switchbox_connection() function is connects and moves data trough TCP socket or serial
  communication link.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all is running fine. Other return values indicate that the
           connection has broken.

****************************************************************************************************
*/
osalStatus ioc_run_switchbox_connection(
    switchboxServiceConnection *con)
{
//    switchboxRoot *root;
    os_timer tnow;
    os_int silence_ms; // , count;

//    root = con->link.root;

    /* If stream is not open, then connect it now. Do not try if if two secons have not
       passed since last failed open try.
     */
#if 0
    if (con->stream == OS_NULL)
    {
        /* Do nothing if ioc_switchbox_service_connect() has not been called.
         */
        parameters = con->parameters;
        if (parameters[0] == '\0') {
            return OSAL_SUCCESS;
        }

        /* If we have no connect to address or it is "*": If we have the lighthouse
           functionality check if we have received the information by UDP broadcast.
           If we got it, try it. Otherwise we can do nothing.
         */
        if (parameters[0] == '\0' || !os_strcmp(parameters, osal_str_asterisk))
        {
            if (con->lighthouse_func == OS_NULL) return OSAL_SUCCESS;
            status = con->lighthouse_func(con->lighthouse, LIGHTHOUSE_GET_CONNECT_STR,
                con->link.root->network_name, con->flags, connectstr, sizeof(connectstr));
            if (OSAL_IS_ERROR(status)) {
                con->ip_from_lighthouse[0] = '\0';
                return OSAL_SUCCESS;
            }
            os_strncpy(con->ip_from_lighthouse, connectstr, OSAL_IPADDR_AND_PORT_SZ);
            parameters = connectstr;

            /* Here we may want to used some connection number instead of 0
             */
            osal_set_network_state_str(OSAL_NS_LIGHTHOUSE_CONNECT_TO, 0, connectstr);
        }

        /* Try connecting the transport.
         */
        if (status == OSAL_PENDING) return OSAL_SUCCESS;
        if (status) return status;

        /* Success, reset connection state.
         */
        ioc_reset_switchbox_service_connection(con);
        return OSAL_SUCCESS;
    }
#endif

    /* Select timing for socket or serial port.
     */
    silence_ms = IOC_SOCKET_SILENCE_MS;

    /* How ever fast we write, we cannot block here.
     */
    os_get_timer(&tnow);
//     count = 32;
#if 0
    while (count--)
    {
        /* Receive as much data as we can.
         */
        while (osal_go())
        {
            status = ioc_connection_receive(con);
            if (status == OSAL_PENDING)
            {
                break;
            }
            if (status)
            {
                goto failed;
            }
        }

        /* Send one frame to connection
         */
        status = ioc_connection_send(con);
        if (status == OSAL_PENDING)
        {
            break;
        }
        if (status)
        {
            goto failed;
        }
    }
#endif

    /* If too much time elapsed sice last receive.
     */
    if (os_has_elapsed_since(&con->last_receive, &tnow, silence_ms))
    {
        osal_trace2("line is silent, closing connection");
        goto failed;
    }


    /* Flush data to the connection.
     */
    if (con->stream)
    {
        osal_stream_flush(con->stream, 0);
    }

// osal_sysconsole_write("HEHE COM OK\n");

    return OSAL_SUCCESS;

failed:
// osal_sysconsole_write("HEHE COM FAILED\n");

    ioc_close_switchbox_service_stream(con);
    return OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Request to terminate connection worker thread.
  @anchor ioc_terminate_switchbox_service_connection_thread

  The ioc_terminate_switchbox_service_connection_thread() function sets request to terminate worker thread, if
  one is running the end point.

  ioc_switchbox_lock() must be on when this function is called.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if no worker thread is running. OSAL_PENDING if worker
           thread is still running.

****************************************************************************************************
*/
osalStatus ioc_terminate_switchbox_service_connection_thread(
    switchboxServiceConnection *con)
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
  @anchor ioc_reset_switchbox_service_connection

  The ioc_reset_switchbox_service_connection() function resets connection state and connected
  source and destination buffers.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_reset_switchbox_service_connection(
    switchboxServiceConnection *con)
{
    os_timer tnow;

    /* Initialize timers.
     */
    os_get_timer(&tnow);
    con->last_receive = tnow;
    con->last_send = tnow;
}


/**
****************************************************************************************************

  @brief Connection worker thread function.
  @anchor ioc_connection_thread

  The ioc_connection_thread() function is worker thread to connect a socket (optional) and
  transfer data trough it.

  @param   prm Pointer to parameters for new thread, pointer to end point object.
  @param   done Event to set when parameters have been copied to entry point
           functions own memory.

****************************************************************************************************
*/
static void ioc_connection_thread(
    void *prm,
    osalEvent done)
{
    switchboxRoot *root;
    switchboxServiceConnection *con;
//    const os_char *parameters;
//    osalStatus status;
    osalSelectData selectdata;
    os_timer tnow;
    os_int silence_ms, count;

    /* Parameters point to the connection object.
     */
    con = (switchboxServiceConnection*)prm;
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

    /* Run the end point.
     */
    while (!con->worker.stop_thread && osal_go())
    {
// static long ulledoo; if (++ulledoo > 10009) {osal_debug_error("ulledoo connection\n"); ulledoo = 0;}

        /* If stream is not open, then connect it now. Do not try if two secons have not
           passed since last failed open try.
         */
#if 0
        if (con->stream == OS_NULL)
        {
            parameters = con->parameters;

            /* If we have no connect to address or it is "*": If we have the lighthouse
               functionality check if we have received the information by UDP broadcast.
               If we got it, try it. Otherwise we can do nothing.
             */
            if (parameters[0] == '\0' || !os_strcmp(parameters, osal_str_asterisk))
            {
                if (con->lighthouse_func == OS_NULL) goto failed;
                os_char connectstr[OSAL_IPADDR_AND_PORT_SZ];
                status = con->lighthouse_func(con->lighthouse, LIGHTHOUSE_GET_CONNECT_STR,
                    con->link.root->network_name, con->flags,
                    connectstr, sizeof(connectstr));

                os_strncpy(con->ip_from_lighthouse, connectstr, OSAL_IPADDR_AND_PORT_SZ);
                if (OSAL_IS_ERROR(status)) goto failed;
                parameters = connectstr;

                /* Here we may want to used some connection number instead of 0
                 */
                osal_set_network_state_str(OSAL_NS_LIGHTHOUSE_CONNECT_TO, 0, connectstr);
            }

            /* Try connecting the transport.
             */
//            status = ioc_try_to_connect(con, parameters);
            if (status == OSAL_PENDING)
            {
                os_timeslice();
                goto failed;
            }
            if (status)
            {
                osal_trace("stream connect try failed");
                goto failed;
            }

            /* Reset connection state
             */
            ioc_reset_switchbox_service_connection(con);
        }

        if (con->flags & IOC_DISABLE_SELECT)
        {
            os_timeslice();
        }
        else
        {
            status = osal_stream_select(&con->stream, 1, con->worker.trig,
                &selectdata, check_timeouts_ms, OSAL_STREAM_DEFAULT);

            if (status == OSAL_STATUS_NOT_SUPPORTED)
            {
                os_timeslice();
            }

            else if (status)
            {
                osal_debug_error("osal_stream_select failed");
                goto failed;
            }
        }
        os_get_timer(&tnow);

#endif

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
            goto failed;
        }

        /* Flush data to the connection.
         */
        if (con->stream)
        {
            osal_stream_flush(con->stream, 0);
        }

        continue;

failed:
        ioc_close_switchbox_service_stream(con);

break;

        os_timeslice();
    }

    /* Delete trigger event and mark that this thread is no longer running.
     */
    ioc_switchbox_lock(root);
    osal_event_delete(con->worker.trig);
    con->worker.trig = OS_NULL;
    con->worker.thread_running = OS_FALSE;

    ioc_release_switchbox_service_connection(con);
    ioc_switchbox_unlock(root);

    osal_trace("connection: worker thread exited");
}


