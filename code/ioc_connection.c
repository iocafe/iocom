/**

  @file    ioc_connection.c
  @brief   Connection object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

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
    iocConnection *con,
    const os_char *parameters);

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

    /* Initialize the "remove memory block request" list root structure to as empty list.
     */
#if IOC_DYNAMIC_MBLK_CODE
    ioc_initialize_remove_mblk_req_list(&con->del_mlk_req_list);
#endif

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

    /* Release memory allocated for allowed_networks.
     */
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
    ioc_release_allowed_networks(&con->allowed_networks);
#endif

    /* Release memory allocated for the "remove memory block request" list.
     */
#if IOC_DYNAMIC_MBLK_CODE
    ioc_release_remove_mblk_req_list(&con->del_mlk_req_list);
#endif

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

  When dynamic mblk code is used, this becomes a bit of chicken and egg problem with
  interdependencies. When connection is beging closed:
    - All source and target buffers related to connection to be closed are to be deleted.
    - Then we need to delete memory blocks which have no connections left "down"
    - And for those memory blocks which are deleted and have connections "up" we need
      to generate a remove memory block request.
    - It is nice to do this without allocating memory, as far as we can.

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
    iocMemoryBlock *mblk;
    iocSourceBuffer *sbuf;
    iocTargetBuffer *tbuf;

    /* Set "to_be_deleted" flags in memory blocks which will be deleted when this
     * connection closes.
     */
    for (sbuf = con->sbuf.first; sbuf; sbuf = sbuf->clink.next)
    {
        ioc_release_dynamic_mblk_if_not_attached(sbuf->mlink.mblk, con, OS_FALSE);
    }
    for (tbuf = con->tbuf.first; tbuf; tbuf = tbuf->clink.next)
    {
        ioc_release_dynamic_mblk_if_not_attached(tbuf->mlink.mblk, con, OS_FALSE);
    }

    /* Generate "remove memory block" requests for other connections which will
       be left but access a memory block to be deleted as consequence of deleting
       this connection.
     */
    for (sbuf = con->sbuf.first; sbuf; sbuf = sbuf->clink.next)
    {
        ioc_generate_del_mblk_request(sbuf->mlink.mblk, con);
    }
    for (tbuf = con->tbuf.first; tbuf; tbuf = tbuf->clink.next)
    {
        ioc_generate_del_mblk_request(tbuf->mlink.mblk, con);
    }

    /* Delete source and target buffers and memory blocks to be consequentally deleted
     */
    while ((sbuf = con->sbuf.first))
    {
        mblk = sbuf->mlink.mblk;
        ioc_release_source_buffer(sbuf);
        ioc_release_dynamic_mblk_if_not_attached(mblk, con, OS_TRUE);
    }
    while ((tbuf = con->tbuf.first))
    {
        mblk = tbuf->mlink.mblk;

        /* if (!mblk->to_be_deleted) {
            ioc_tbuf_disconnect_signals(tbuf);
        } */

        ioc_release_target_buffer(tbuf);
        ioc_release_dynamic_mblk_if_not_attached(mblk, con, OS_TRUE);
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

  @return  OSAL_SUCCESS if successful. Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_connect(
    iocConnection *con,
    iocConnectionParams *prm)
{
    iocRoot *root;
    os_char *frame_in_buf, *frame_out_buf;
    os_int flags;

#if OSAL_MULTITHREAD_SUPPORT
    osalThreadOptParams opt;
#endif

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

#if OSAL_PC_DEBUG
    /* Report IOC_DYNAMIC_MBLKS is given but cannot be followed.
     */
    if (prm->flags & IOC_DYNAMIC_MBLKS) {
#if IOC_DYNAMIC_MBLK_CODE
        if (root->droot == OS_NULL) {
            osal_debug_error("ioc_connect(): IOC_DYNAMIC_MBLKS flag set but "
                "ioc_initialize_dynamic_root() has not been called");
        }
#else
        osal_debug_error("ioc_connect(): IOC_DYNAMIC_MBLKS flag set but "
            "disabled by define IOC_DYNAMIC_MBLK_CODE=0");
#endif
    }
#endif

    flags = prm->flags;
    if (prm->iface) if (prm->iface->iflags & OSAL_STREAM_IFLAG_SECURE)
    {
        flags |= IOC_SECURE_CONNECTION;
    }
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
#if IOC_AUTHENTICATION_CODE
    os_strncpy(con->user_override, prm->user_override, IOC_NAME_SZ);
    os_strncpy(con->password_override, prm->password_override, IOC_PASSWORD_SZ);
#endif

#if OSAL_SOCKET_SUPPORT
    con->lighthouse_func = prm->lighthouse_func;
    con->lighthouse = prm->lighthouse;
#endif

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
        con->flags |= IOC_CLOSE_CONNECTION_ON_ERROR|IOC_LISTENER;

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
        /* opt.stack_size = 16000; */
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

    const os_char *parameters;
    osalStatus status;
    os_timer tnow;
    os_int silence_ms, count;

    osal_debug_assert(con->debug_id == 'C');

    /* If stream is not open, then connect it now. Do not try if if two secons have not
       passed since last failed open try.
     */
    if (con->stream == OS_NULL)
    {
        /* Do nothing if ioc_connect() has not been called.
         */
        parameters = con->parameters;
        if (parameters[0] == '\0') {
            return OSAL_SUCCESS;
        }

#if OSAL_SOCKET_SUPPORT
        /* If we have no connect to address or it is "*": If we have the lighthouse
           functionality check if we have received the information by UDP broadcast.
           If we got it, try it. Otherwise we can do nothing.
         */
        if (parameters[0] == '\0' || !os_strcmp(parameters, osal_str_asterisk))
        {
            os_char connectstr[OSAL_HOST_BUF_SZ];
            if (con->lighthouse_func == OS_NULL) return OSAL_SUCCESS;
            status = con->lighthouse_func(con->lighthouse, LIGHTHOUSE_GET_CONNECT_STR,
                con->link.root->network_name, IOC_NETWORK_NAME_SZ, con->flags,
                connectstr, sizeof(connectstr));
            if (OSAL_IS_ERROR(status)) {
                con->ip_from_lighthouse[0] = '\0';
                return OSAL_SUCCESS;
            }
            os_strncpy(con->ip_from_lighthouse, connectstr, OSAL_IPADDR_AND_PORT_SZ);
            parameters = connectstr;
        }
#endif

        /* Try connecting the transport.
         */
        status = ioc_try_to_connect(con, parameters);
        if (status == OSAL_PENDING) return OSAL_SUCCESS;
        if (status) return status;

        /* Success, reset connection state.
         */
        ioc_reset_connection_state(con);
        return OSAL_SUCCESS;
    }

    /* Select timing for socket or serial port.
     */
#if OSAL_SERIAL_SUPPORT
    if ((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL)
    {
        /* Check if we need to initiate the serial connection.
         */
        status = ioc_establish_serial_connection(con);
        if (status == OSAL_PENDING) return OSAL_SUCCESS;
        if (status) goto failed;

        silence_ms = IOC_SERIAL_SILENCE_MS;
    }
    else
    {
        silence_ms = IOC_SOCKET_SILENCE_MS;
    }
#else
    silence_ms = IOC_SOCKET_SILENCE_MS;
#endif

    /* How ever fast we write, we cannot block here.
     */
    os_get_timer(&tnow);
    count = 32;
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

    /* If too much time elapsed sice last receive.
     */
    if (os_has_elapsed_since(&con->last_receive, &tnow, silence_ms))
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
        ioc_free_source_and_target_bufs(con->link.root, con);
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
  @return  OSAL_SUCCESS if no worker thread is running. OSAL_PENDING if worker
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
        status = OSAL_PENDING;
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
    iocConnection *con,
    const os_char *parameters)
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
        if (!os_has_elapsed(&con->socket_open_fail_timer, 2000)) return OSAL_PENDING;
    }
    if (!osal_int64_is_zero(&con->socket_open_try_timer))
    {
        if (!os_has_elapsed(&con->socket_open_try_timer, 500)) return OSAL_PENDING;
    }

    /* Save stream interface pointer.
     */
    iface = con->iface;

    /* Try to open listening socket port.
     */
    osal_trace3("connection: opening stream...");
    flags = OSAL_STREAM_CONNECT|OSAL_STREAM_TCP_NODELAY;
    flags |= ((con->flags & IOC_DISABLE_SELECT) ? OSAL_STREAM_NO_SELECT : OSAL_STREAM_SELECT);
    os_get_timer(&con->socket_open_try_timer);

    con->stream = osal_stream_open(iface, parameters, OS_NULL, &status, flags);
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

    /* We have not authenticated other end yet
     */
    con->authentication_sent = OS_FALSE;
    con->authentication_received = OS_FALSE;

    /* Clear flow control variables.
     */
    con->bytes_received = 0;
    con->bytes_acknowledged = 0xA0A000;
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
        sbuf->syncbuf.used = OS_FALSE;
        sbuf->syncbuf.start_addr = sbuf->syncbuf.end_addr = 0;
        sbuf->syncbuf.make_keyframe = OS_TRUE;
        sbuf->syncbuf.is_keyframe = OS_TRUE;
    }

    for (tbuf = con->tbuf.first;
         tbuf;
         tbuf = tbuf->clink.next)
    {
        tbuf->syncbuf.buf_start_addr = tbuf->syncbuf.buf_end_addr = 0;
        tbuf->syncbuf.buf_used = OS_FALSE;
        tbuf->syncbuf.has_new_data = OS_FALSE;
        tbuf->syncbuf.newdata_start_addr = tbuf->syncbuf.newdata_end_addr = 0;
    }

#if IOC_DYNAMIC_MBLK_CODE
    ioc_release_remove_mblk_req_list(&con->del_mlk_req_list);
    ioc_initialize_remove_mblk_req_list(&con->del_mlk_req_list);
#endif
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
        osal_stream_close(con->stream, OSAL_STREAM_DEFAULT);
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
    iocRoot *root;
    iocConnection *con;
    const os_char *parameters;
    osalStatus status;
    osalSelectData selectdata;
    os_timer tnow;
    os_int check_timeouts_ms, silence_ms, count;
    os_boolean is_serial;

    osal_trace("connection: worker thread started");

    /* Parameters point to the connection object.
     */
    con = (iocConnection*)prm;
    root = con->link.root;

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
            parameters = con->parameters;

#if OSAL_SOCKET_SUPPORT
            /* If we have no connect to address or it is "*": If we have the lighthouse
               functionality check if we have received the information by UDP broadcast.
               If we got it, try it. Otherwise we can do nothing.
             */
            if (parameters[0] == '\0' || !os_strcmp(parameters, osal_str_asterisk))
            {
                if (con->lighthouse_func == OS_NULL) goto failed;
                os_char connectstr[OSAL_IPADDR_AND_PORT_SZ];
                status = con->lighthouse_func(con->lighthouse, LIGHTHOUSE_GET_CONNECT_STR,
                    con->link.root->network_name, IOC_NETWORK_NAME_SZ, con->flags,
                    connectstr, sizeof(connectstr));

                os_strncpy(con->ip_from_lighthouse, connectstr, OSAL_IPADDR_AND_PORT_SZ);
                if (OSAL_IS_ERROR(status)) goto failed;
                parameters = connectstr;
            }
#endif

            /* Try connecting the transport.
             */
            status = ioc_try_to_connect(con, parameters);
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
            ioc_reset_connection_state(con);
        }

// ioc_send_all(root);
// ioc_receive_all(root); TO BE TESTED

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

#if OSAL_SERIAL_SUPPORT
        /* Check if we need to initiate the serial connection.
         */
        if (is_serial)
        {
            status = ioc_establish_serial_connection(con);
            if (status == OSAL_PENDING) continue;
            if (status) goto failed;
        }
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
                status = ioc_connection_receive(con);
                if (status == OSAL_PENDING)
                {
                    break;
                }
                if (status)
                {
                    goto failed;
                }

                /* Record timer of last successful receive.
                 */
                con->last_receive = tnow;
            }

            /* Try sending data though the connection.
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

        /* If too much time elapsed sice last receive?
         */
        if (os_has_elapsed_since(&con->last_receive, &tnow, silence_ms))
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

  //          root = con->link.root;
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
//    root = con->link.root;
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

