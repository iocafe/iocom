/**

  @file    ioc_connection.c
  @brief   Connection object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.8.2018

  A connection object represents logical connection between two devices. Both ends of
  communication have a connection object dedicated for that link, serialized data is
  transferred from a connection object to another.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/* Forward referred static functions.
 */
static void ioc_free_connection_bufs(
    iocConnection *con);

static void ioc_free_source_and_target_bufs(
    iocRoot *root,
    iocConnection *con);

static osalStatus ioc_try_to_connect(
    iocConnection *con);

#if OSAL_MULTITHREAD_SUPPORT
static void ioc_connection_thread(
    void *prm,
    osalEvent done);
#endif


/**
****************************************************************************************************

  @brief Initialize connection.
  @anchor ioc_initialize_connection

  The ioc_initialize_connection() function initializes a connection. A connection can always
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
iocConnection *ioc_initialize_connection(
    iocConnection *con,
    iocRoot *root)
{
    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    /* Synchronize.
     */
    ioc_lock(root);

    if (con == OS_NULL)
    {
        con = (iocConnection*)ioc_malloc(root, sizeof(iocConnection), OS_NULL);
        if (con == OS_NULL)
        {
            ioc_unlock(root);
            return OS_NULL;
        }

        os_memclear(con, sizeof(iocConnection));
        con->allocated = OS_TRUE;
    }
    else
    {
        os_memclear(con, sizeof(iocConnection));
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

    /* Mark connection structure as initialized connection object (for debugging).
     */
    IOC_SET_DEBUG_ID(con, 'C')

    /* End syncronization.
     */
    ioc_unlock(root);

    /* Return pointer to initialized connection.
     */
    osal_trace("connection: initialized");
    return con;
}


/**
****************************************************************************************************

  @brief Release connection.
  @anchor ioc_release_connection

  The ioc_release_connection() function releases resources allocated for the connection
  object. Memory allocated for the connection object is freed, if it was allocated by
  ioc_initialize_connection().

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_connection(
    iocConnection *con)
{
    iocRoot
        *root;

    os_boolean
        allocated;

    /* Check that con is valid pointer.
     */
    osal_debug_assert(con->debug_id == 'C');

    /* Synchronize.
     */
    root = con->link.root;
    ioc_lock(root);

    /* If stream is open, close it.
     */
    ioc_close_stream(con);

    /* Connection is being deleted, remove it from all send info.
     */
    ioc_mbinfo_con_is_closed(con);

    /* Release all source and target buffers.
     */
    ioc_free_source_and_target_bufs(root, con);

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

    /* Release memory allocated for connection buffers, if any
     */
    ioc_free_connection_bufs(con);

    /* Clear allocated memory indicate that is no longer initialized (for debugging and
       for primitive static allocation schema). Save allocated flag before memclear.
     */
    allocated = con->allocated;
    os_memclear(con, sizeof(iocConnection));

    if (allocated)
    {
        ioc_free(root, con, sizeof(iocConnection));
    }

    /* End syncronization.
     */
    ioc_unlock(root);
    osal_trace("connection: released");
}


/**
****************************************************************************************************

  @brief Release memory allocated for connection buffers.
  @anchor ioc_free_connection_bufs

  The ioc_free_connection_bufs() function...

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
static void ioc_free_connection_bufs(
    iocConnection *con)
{
    iocRoot *root;
    root = con->link.root;

    if (con->frame_out.allocated)
    {
        ioc_free(root, con->frame_out.buf, con->frame_sz);
        con->frame_out.allocated = OS_FALSE;
        con->frame_out.buf = OS_NULL;
    }

    if (con->frame_in.allocated)
    {
        ioc_free(root, con->frame_in.buf, con->frame_sz);
        con->frame_in.allocated = OS_FALSE;
        con->frame_in.buf = OS_NULL;
    }
}


/**
****************************************************************************************************

  @brief Release all source and target buffers related to connection.
  @anchor ioc_free_source_and_target_bufs

  The ioc_free_source_and_target_bufs() function...

  Mutex lock must be on.

  @param   root Pointer to root object.
  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
static void ioc_free_source_and_target_bufs(
    iocRoot *root,
    iocConnection *con)
{
#if IOC_DYNAMIC_MBLK_CODE
    iocHandle handle;

    /* Release all source buffers.
     */
    while (con->sbuf.first)
    {
        ioc_setup_handle(&handle, root, con->sbuf.first->mlink.mblk);
        ioc_release_source_buffer(con->sbuf.first);
        ioc_release_dynamic_mblk_if_not_attached(&handle);
        ioc_release_handle(&handle);
    }

    /* Release all target buffers.
     */
    while (con->tbuf.first)
    {
        ioc_setup_handle(&handle, root, con->tbuf.first->mlink.mblk);
        ioc_release_target_buffer(con->tbuf.first);
        ioc_release_dynamic_mblk_if_not_attached(&handle);
        ioc_release_handle(&handle);
    }
#else
    /* Release all source buffers.
     */
    while (con->sbuf.first)
    {
        ioc_release_source_buffer(con->sbuf.first);
    }

    /* Release all target buffers.
     */
    while (con->tbuf.first)
    {
        ioc_release_target_buffer(con->tbuf.first);
    }
#endif
}


/**
****************************************************************************************************

  @brief Start or prepare the connection.
  @anchor ioc_connect

  The ioc_connect() function sets up for socket or serial connection. Actual socket
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

  @return  OSAL_SUCCESS if successfull. Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_connect(
    iocConnection *con,
    iocConnectionParams *prm)
{
    iocRoot *root;
    os_char *frame_in_buf, *frame_out_buf;
    os_int flags;
    osalThreadOptParams opt;

    osal_debug_assert(con->debug_id == 'C');

    root = con->link.root;
    ioc_lock(root);

#if OSAL_MULTITHREAD_SUPPORT
    /* If we are already running connection, stop it. Wait until it has stopped.
     */
    while (ioc_terminate_connection_thread(con))
    {
        ioc_unlock(root);
        os_timeslice();
        ioc_lock(root);
    }
#endif

    flags = prm->flags;
#if OSAL_MULTITHREAD_SUPPORT==0
    /* If we have no multithread support, make sure that
       IOC_CREATE_THREAD flag is not given.
     */
    osal_debug_assert((flags & IOC_CREATE_THREAD) == 0);
#endif

    /* Save pointer to stream interface to use.
     */
    con->iface = prm->iface;

    if (flags & IOC_SOCKET)
    {
        con->frame_sz = IOC_SOCKET_FRAME_SZ;
        con->max_in_air = IOC_SOCKET_MAX_IN_AIR;
        con->unacknogledged_limit = IOC_SOCKET_UNACKNOGLEDGED_LIMIT;
        con->max_ack_in_air = IOC_SOCKET_MAX_ACK_IN_AIR;
#if OSAL_SOCKET_SELECT_SUPPORT == 0
        flags |= IOC_DISABLE_SELECT;
#endif
    }
    else
    {
        con->frame_sz = IOC_SERIAL_FRAME_SZ;
        con->max_in_air = IOC_SERIAL_MAX_IN_AIR;
        con->unacknogledged_limit = IOC_SERIAL_UNACKNOGLEDGED_LIMIT;
        con->max_ack_in_air = IOC_SERIAL_MAX_ACK_IN_AIR;
#if OSAL_SERIAL_SELECT_SUPPORT == 0
        flags |= IOC_DISABLE_SELECT;
#endif
    }
    con->flags = flags;

#if OSAL_SERIAL_SUPPORT
    con->sercon_state = OSAL_SERCON_STATE_INIT_1;
#endif

#if OSAL_DEBUG
    osal_debug_assert(os_strlen(prm->parameters) <= IOC_CONNECTION_PRMSTR_SZ);

    if (prm->frame_out_buf)
    {
        osal_debug_assert(prm->frame_out_buf_sz == con->frame_sz);
    }
#endif
    os_strncpy(con->parameters, prm->parameters, IOC_CONNECTION_PRMSTR_SZ);

    /* Release any previously allocated buffers.
     */
    ioc_free_connection_bufs(con);

    /* Setup or allocate outgoing frame buffer.
     */
    frame_out_buf = prm->frame_out_buf;
    if (frame_out_buf == OS_NULL)
    {
        frame_out_buf = ioc_malloc(root, con->frame_sz, OS_NULL);
        if (frame_out_buf == OS_NULL)
        {
            ioc_unlock(root);
            return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        }
        con->frame_out.allocated = OS_TRUE;
    }
    os_memclear(frame_out_buf, con->frame_sz);
    con->frame_out.buf = frame_out_buf;

    /* Setup or allocate outgoing frame buffer.
     */
    frame_in_buf = prm->frame_in_buf;
    if (frame_in_buf == OS_NULL)
    {
        frame_in_buf = ioc_malloc(root, con->frame_sz, OS_NULL);
        if (frame_in_buf == OS_NULL)
        {
            ioc_free_connection_bufs(con);
            ioc_unlock(root);
            return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        }
        con->frame_in.allocated = OS_TRUE;
    }
    os_memclear(frame_in_buf, con->frame_sz);
    con->frame_in.buf = frame_in_buf;

    /* If this is incoming TCP socket accepted by end point?
     */
    if (prm->newsocket)
    {
        con->stream = prm->newsocket;
        con->flags |= IOC_CLOSE_CONNECTION_ON_ERROR|IOC_SERIAL_LISTENER;

        /* Reset connection state
         */
        ioc_reset_connection_state(con);
    }

#if OSAL_MULTITHREAD_SUPPORT
    /* If we want to run end point in separate thread.
     */
    if (flags & IOC_CREATE_THREAD)
    {
        con->worker.trig = osal_event_create();
        con->worker.thread_running = OS_TRUE;
        con->worker.stop_thread = OS_FALSE;

        os_memclear(&opt, sizeof(opt));
        opt.thread_name = "connection";
        opt.stack_size = 4000;
        opt.pin_to_core = OS_TRUE;
        opt.pin_to_core_nr = 0;

        osal_thread_create(ioc_connection_thread, con,
            &opt, OSAL_THREAD_DETACHED);
    }
#endif

    ioc_unlock(root);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Connect and move data.
  @anchor ioc_run_connection

  The ioc_run_connection() function is connects and moves data trough TCP socket or serial
  communication link. It is called repeatedly by ioc_run() and should not be called from
  application.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all is running fine. Other return values indicate that the
           connection has broken.

****************************************************************************************************
*/
osalStatus ioc_run_connection(
    iocConnection *con)
{
    IOC_MT_ROOT_PTR;

    osalStatus
        status;

    os_timer
        tnow;

    os_int
        silence_ms,
        count;

    os_boolean
        is_serial;

    osal_debug_assert(con->debug_id == 'C');

    /* If stream is not open, then connect it now. Do not try if if two secons have not
       passed since last failed open try.
     */
    if (con->stream == OS_NULL)
    {
        /* Do nothing if ioc_connect() has not been called. Should we report a program error?
         */
        if (con->parameters[0] == '\0')
            return OSAL_SUCCESS;

        status = ioc_try_to_connect(con);
        if (status == OSAL_STATUS_PENDING) return OSAL_SUCCESS;
        if (status) return status;

        /* Reset connection state
         */
        ioc_reset_connection_state(con);
        return OSAL_SUCCESS;
    }

    /* Select timing for socket or serial port.
     */
#if OSAL_SERIAL_SUPPORT
    is_serial = (os_boolean)((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL);
    if (is_serial)
    {
        /* Check if we need to initiate the serial connection.
         */
        status = ioc_establish_serial_connection(con);
        if (status == OSAL_STATUS_PENDING) return OSAL_SUCCESS;
        if (status) goto failed;

        silence_ms = IOC_SERIAL_SILENCE_MS;
    }
    else
    {
        silence_ms = IOC_SOCKET_SILENCE_MS;
    }
#else
    is_serial = OS_FALSE;
    silence_ms = IOC_SOCKET_SILENCE_MS;
#endif

    os_get_timer(&tnow);
    count = 32; /* How ever fasw we write, we cannot block here */
    while (count--)
    {
        /* Receive as much data as we can
         */
        while (osal_go())
        {
            status = ioc_connection_receive(con);
            if (status == OSAL_STATUS_PENDING)
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
        if (status == OSAL_STATUS_PENDING)
        {
            break;
        }
        if (status)
        {
            goto failed;
        }
    }

    /* If too much time elapsed sice last receive
     */
    if (os_elapsed2(&con->last_receive, &tnow, silence_ms))
    {
        osal_trace2("line is silent, closing connection");
        goto failed;
    }

    /* If time to send keep alive.
     */
    if (ioc_send_timed_keepalive(con, &tnow))
    {
        goto failed;
    }

    /* Flush data to the connection.
     */
    if (con->stream)
    {
        osal_stream_flush(con->stream, 0);
    }

    return OSAL_SUCCESS;

failed:
    /* If this is flagged connected, turn the flag off.
     */
    if (con->connected)
    {
        ioc_set_mt_root(root, con->link.root);
        ioc_reset_connection_state(con);
        ioc_lock(root);
        con->connected = OS_FALSE;
        ioc_free_source_and_target_bufs(root, con);
        /* ioc_count_connected_streams(con->link.root, OS_TRUE); */
        ioc_mbinfo_con_is_closed(con);
        ioc_unlock(root);
    }

    ioc_close_stream(con);
    return OSAL_STATUS_FAILED;
}


#if OSAL_MULTITHREAD_SUPPORT
/**
****************************************************************************************************

  @brief Request to terminate connection worker thread.
  @anchor ioc_terminate_connection_thread

  The ioc_terminate_connection_thread() function sets request to terminate worker thread, if
  one is running the end point.

  ioc_lock() must be on when this function is called.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if no worker thread is running. OSAL_STATUS_PENDING if worker
           thread is still running.

****************************************************************************************************
*/
osalStatus ioc_terminate_connection_thread(
    iocConnection *con)
{
    osalStatus
        status = OSAL_SUCCESS;

    if (con->worker.thread_running)
    {
        con->worker.stop_thread = OS_TRUE;
        if (con->worker.trig) osal_event_set(con->worker.trig);
        status = OSAL_STATUS_PENDING;
    }

    return status;
}
#endif


/**
****************************************************************************************************

  @brief Try to connect the stream.
  @anchor ioc_try_to_connect

  The ioc_try_to_connect() function tries to open listening TCP socket. Do not try if
  f two secons have not passed since last failed open try.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if we have succesfully opened the stream. Othervalues indicate
           failure  or delay.

****************************************************************************************************
*/
static osalStatus ioc_try_to_connect(
    iocConnection *con)
{
    osalStatus
        status;

    const osalStreamInterface
        *iface;

    os_int
        flags;

    /* If two seconds have not passed since last failed try.
     */
    if (!osal_int64_is_zero(&con->socket_open_fail_timer))
    {
        if (!os_elapsed(&con->socket_open_fail_timer, 2000)) return OSAL_STATUS_PENDING;
    }

    /* Select serial or socket interface by flags and operating system abstraction layer support.
     */
    iface = con->iface;
/* #if OSAL_SERIAL_SUPPORT
    iface = OSAL_SERIAL_IFACE;
#if OSAL_SOCKET_SUPPORT
    iface = (con->flags & IOC_SOCKET) ? OSAL_SOCKET_IFACE : OSAL_SERIAL_IFACE;
#endif
#else
    iface = OSAL_SOCKET_IFACE;
#endif
*/

    /* Try to open listening socket port.
     */
    osal_trace3("connection: opening stream...");
    flags = OSAL_STREAM_CONNECT|OSAL_STREAM_TCP_NODELAY;
    flags |= ((con->flags & IOC_DISABLE_SELECT) ? OSAL_STREAM_NO_SELECT : OSAL_STREAM_SELECT);

    con->stream = osal_stream_open(iface, con->parameters, OS_NULL, &status, flags);
    if (con->stream == OS_NULL)
    {
        osal_debug_error("Opening stream failed");
        os_get_timer(&con->socket_open_fail_timer);
        return status;
    }

    /* Success.
     */
    osal_int64_set_zero(&con->socket_open_fail_timer);
    osal_trace2("connection: stream opened");
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Reset connection state to start from beginning
  @anchor ioc_reset_connection_state

  The ioc_reset_connection_state() function resets connection state and connected
  source and destination buffers.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_reset_connection_state(
    iocConnection *con)
{
    os_timer tnow;
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf;

    con->frame_in.frame_nr = 0;
    con->frame_in.pos = 0;
    con->frame_out.frame_nr = 0;
    con->frame_out.pos = 0;
    con->frame_out.used = OS_FALSE;

    /* Clear flow control variables.
     */
    con->bytes_received = 0;
    con->bytes_acknowledged = 0xA000;
    con->bytes_sent = con->processed_bytes = 0;

    /* Initialize timers.
     */
    os_get_timer(&tnow);
    con->last_receive = tnow;
    con->last_send = tnow;

    for (sbuf = con->sbuf.first;
         sbuf;
         sbuf = sbuf->clink.next)
    {
        // sbuf->changed.range_set = OS_FALSE;
        // sbuf->syncbuf.used = OS_FALSE;
        // sbuf->syncbuf.start_addr = sbuf->syncbuf.end_addr = 0;
        sbuf->syncbuf.make_keyframe = OS_TRUE;
        sbuf->syncbuf.is_keyframe = OS_TRUE;
    }

    for (tbuf = con->tbuf.first;
         tbuf;
         tbuf = tbuf->clink.next)
    {
        // tbuf->syncbuf.buf_start_addr = tbuf->syncbuf.buf_end_addr = 0;
        // tbuf->syncbuf.buf_used = OS_FALSE;
        tbuf->syncbuf.has_new_data = OS_FALSE;
        tbuf->syncbuf.newdata_start_addr = tbuf->syncbuf.newdata_end_addr = 0;
    }
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
void ioc_close_stream(
    iocConnection *con)
{
    if (con->stream)
    {
        osal_trace2("stream closed");
        osal_stream_close(con->stream);
        con->stream = OS_NULL;
#if OSAL_SERIAL_SUPPORT
        con->sercon_state = OSAL_SERCON_STATE_INIT_1;
#endif
    }
}


#if OSAL_MULTITHREAD_SUPPORT
/**
****************************************************************************************************

  @brief Connection worker thread function.
  @anchor ioc_connection_thread

  The ioc_connection_thread() function is worker thread to connect a socket (optional) and
  transfer data trough it.

  @param   prm Pointer to parameters for new thread, pointer to end point object.
  @param   done Event to set when parameters have been copied to entry point
           functions own memory.

  @return  None.

****************************************************************************************************
*/
static void ioc_connection_thread(
    void *prm,
    osalEvent done)
{
    iocRoot
        *root;

    iocConnection
        *con;

    osalStatus
        status;

    osalSelectData
        selectdata;

    os_timer
        tnow;

    os_int
        check_timeouts_ms,
        silence_ms,
        count;

    os_boolean
        is_serial;

    osal_trace("connection: worker thread started");

    /* Parameters point to the connection object.
     */
    con = (iocConnection*)prm;

    /* Let thread which created this one proceed.
     */
    osal_event_set(done);

    /* Select of time interval how often to check for timeouts, etc.
       Serial connections need to be monitored much more often than
       sockets to detect partically received frames.
     */
    is_serial = (os_boolean)((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL);
    if (is_serial)
    {
        silence_ms = IOC_SERIAL_SILENCE_MS;
        check_timeouts_ms = IOC_SERIAL_CHECK_TIMEOUTS_MS;

    }
    else
    {
        silence_ms = IOC_SOCKET_SILENCE_MS;
        check_timeouts_ms = IOC_SOCKET_CHECK_TIMEOUTS_MS;
    }
    os_memclear(&selectdata, sizeof(selectdata));

    /* Run the end point.
     */
    while (!con->worker.stop_thread && osal_go())
    {
        /* If stream is not open, then connect it now. Do not try if two secons have not
           passed since last failed open try.
         */
        if (con->stream == OS_NULL)
        {
            status = ioc_try_to_connect(con);
            if (status == OSAL_STATUS_PENDING)
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
            ioc_reset_connection_state(con);
        }

        if (con->flags & IOC_DISABLE_SELECT)
        {
            os_timeslice();
        }
        else
        {
            status = osal_stream_select(&con->stream, 1, con->worker.trig,
                &selectdata, check_timeouts_ms, OSAL_STREAM_DEFAULT);
        }
        os_get_timer(&tnow);

#if OSAL_SERIAL_SUPPORT
        /* Check if we need to initiate the serial connection.
         */
        if (is_serial)
        {
            status = ioc_establish_serial_connection(con);
            if (status == OSAL_STATUS_PENDING) continue;
            if (status) goto failed;
        }
#endif

#if OSAL_SOCKET_SUPPORT
        /* If socket has been closed by the other end.
         */
        if (selectdata.eventflags & OSAL_STREAM_CLOSE_EVENT)
        {
            osal_trace("stream close event");
            goto failed;
        }

        /* Anything error after checking for close event is interprented as broken socket.
         */
        if (selectdata.errorcode)
        {
            osal_trace("socket broken, stream error");
            goto failed;
        }
#endif

        /* Receive and send in loop as long as we can without waiting.
         */
        count = 32; /* How ever fast we write, we cannot block here */
        while (count--)
        {
            while (osal_go())
            {
                /* Try receiving data from the connection.
                 */
                status = ioc_connection_receive(con);
                if (status == OSAL_STATUS_PENDING)
                {
                    break;
                }
                if (status)
                {
                    goto failed;
                }

                /* Record timer of last successfull receive.
                 */
                con->last_receive = tnow;
            }

            /* Try sending data though the connection.
             */
            status = ioc_connection_send(con);
            if (status == OSAL_STATUS_PENDING)
            {
                break;
            }
            if (status)
            {
                goto failed;
            }
        }

        /* If too much time elapsed sice last receive?
         */
        if (os_elapsed2(&con->last_receive, &tnow, silence_ms))
        {
            osal_trace("line is silent, closing connection");
            goto failed;
        }

        /* If time to send keep alive?
         */
        if (ioc_send_timed_keepalive(con, &tnow))
        {
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
        ioc_close_stream(con);

        /* If this is flagged connected, turn the flag off.
         */
        if (con->connected)
        {
            con->connected = OS_FALSE;

            root = con->link.root;
            ioc_lock(root);
            ioc_free_source_and_target_bufs(root, con);
            ioc_unlock(root);
            /* ioc_count_connected_streams(con->link.root, OS_TRUE); */
            ioc_mbinfo_con_is_closed(con);
        }

        if (con->flags & IOC_CLOSE_CONNECTION_ON_ERROR)
        {
            break;
        }
    }

    /* Delete trigger event and mark that this thread is no longer running.
     */
    root = con->link.root;
    ioc_lock(root);
    osal_event_delete(con->worker.trig);
    con->worker.trig = OS_NULL;
    con->worker.thread_running = OS_FALSE;

    if (con->flags & IOC_CLOSE_CONNECTION_ON_ERROR)
    {
        ioc_release_connection(con);
    }
    ioc_unlock(root);

    osal_trace("connection: worker thread exited");
}
#endif

