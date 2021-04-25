/**

  @file    switchbox_end_point.c
  @brief   End point object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  An end_point listens TCP socket for incoming connections and accepts these.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "switchbox.h"

/* Forward referred static functions.
 */
static osalStatus ioc_try_to_open_switchbox_endpoint(
    switchboxEndPoint *epoint);

static osalStatus ioc_try_accept_new_switchbox_sockets(
    switchboxEndPoint *epoint);

static osalStatus ioc_establish_switchbox_connection(
    switchboxEndPoint *epoint,
    osalStream newsocket,
    os_char *remote_ip_addr);

static void ioc_switchbox_endpoint_thread(
    void *prm,
    osalEvent done);

static void ioc_do_switchbox_end_point_callback(
    switchboxEndPoint *epoint,
    switchboxEndPointEvent event);

/**
****************************************************************************************************

  @brief Initialize end_point.
  @anchor ioc_initialize_switchbox_end_point

  The ioc_initialize_switchbox_end_point() function initializes a end_point. A end_point can always
  be allocated global variable. In this case pointer to memory to be initialized is given as
  argument and return value is the same pointer. If dynamic memory allocation is supported,
  and the epoint argument is OS_NULL, the end point object is allocated by the function.

  @param   epoint Pointer to static end_point structure, or OS_NULL to allocate end_point
           object dynamically.
  @param   root Pointer to the root object.
  @return  Pointer to initialized end_point object. OS_NULL is memory allocation failed.

****************************************************************************************************
*/
switchboxEndPoint *ioc_initialize_switchbox_end_point(
    switchboxEndPoint *epoint,
    switchboxRoot *root)
{
    /* Synchronize.
     */
    ioc_switchbox_lock(root);

    if (epoint == OS_NULL)
    {
        epoint = (switchboxEndPoint*)os_malloc(sizeof(switchboxEndPoint), OS_NULL);
        if (epoint == OS_NULL)
        {
            ioc_switchbox_unlock(root);
            return OS_NULL;
        }

        os_memclear(epoint, sizeof(switchboxEndPoint));
        epoint->allocated = OS_TRUE;
    }
    else
    {
        os_memclear(epoint, sizeof(switchboxEndPoint));
    }

    /* Save pointer to root object and join to linked list of end_points.
     */
    epoint->link.root = root;
    epoint->link.prev = root->epoint.last;
    if (root->epoint.last)
    {
        root->epoint.last->link.next = epoint;
    }
    else
    {
        root->epoint.first = epoint;
    }
    root->epoint.last = epoint;

    epoint->trig = osal_event_create(OSAL_EVENT_SET_AT_EXIT);

    /* End syncronization.
     */
    ioc_switchbox_unlock(root);

    /* Return pointer to initialized end_point.
     */
    osal_trace("end point: initialized");
    return epoint;
}


/**
****************************************************************************************************

  @brief Release end_point.
  @anchor ioc_release_switchbox_end_point

  The ioc_release_switchbox_end_point() function releases resources allocated for the end_point
  object. Memory allocated for the end point object is freed, if it was allocated by
  ioc_initialize_switchbox_end_point().

  @param   epoint Pointer to the end point object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_switchbox_end_point(
    switchboxEndPoint *epoint)
{
    switchboxRoot *root;
    os_boolean allocated;

    /* If we are running end point thread, stop it.
     */
    while (ioc_terminate_switchbox_end_point_thread(epoint)) {
        os_timeslice();
    }

    /* Synchronize.
     */
    root = epoint->link.root;
    ioc_switchbox_lock(root);

    /* Remove end_point from linked list.
     */
    if (epoint->link.prev)
    {
        epoint->link.prev->link.next = epoint->link.next;
    }
    else
    {
        epoint->link.root->epoint.first = epoint->link.next;
    }
    if (epoint->link.next)
    {
        epoint->link.next->link.prev = epoint->link.prev;
    }
    else
    {
        epoint->link.root->epoint.last = epoint->link.prev;
    }

    osal_event_delete(epoint->trig);
    epoint->trig = OS_NULL;

    /* Clear allocated memory indicate that is no longer initialized (for debugging and
       for primitive static allocation schema).
     */
    allocated = epoint->allocated;
    os_memclear(epoint, sizeof(switchboxEndPoint));

    /* If memory for end point was allocated, release it.
     */
    if (allocated)
    {
        os_free(epoint, sizeof(switchboxEndPoint));
    }

    /* End syncronization.
     */
    ioc_switchbox_unlock(root);

    osal_trace("end point: released");
}


/**
****************************************************************************************************

  @brief Start or prepare the end point to listen for TCP socket connections.
  @anchor ioc_switchbox_listen

  The ioc_switchbox_listen() function sets up listening socket end point. If IOC_CREATE_THREAD flag is
  given, the function created a new thread to run the end point.

  @param   epoint Pointer to the end point object.
  @param   prm Parameter structure. Clear parameter structure using os_memclear() and
           set the members needed. Members:
           - parameters For example ":8817" or "127.0.0.1:8817" for TCP socket.
           - flags Bit fields: IOC_SOCKET Connect with TCP socket (set always).
             IOC_CREATE_THREAD Create thread to run end_point and create thread to run
             each accepted connection (multithread support needed).

  @return  OSAL_SUCCESS if successful. Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_switchbox_listen(
    switchboxEndPoint *epoint,
    switchboxEndPointParams *prm)
{
    switchboxRoot *root;
    os_short flags;

    root = epoint->link.root;
    ioc_switchbox_lock(root);

    flags = prm->flags;
    if (prm->iface) if (prm->iface->iflags & OSAL_STREAM_IFLAG_SECURE)
    {
        flags |= IOC_SECURE_CONNECTION;
    }
    epoint->flags = flags;
    epoint->iface = prm->iface;

#if OSAL_DEBUG
    if (os_strlen(prm->parameters) > SWITCHBOX_END_POINT_PRMSTR_SZ)
    {
        osal_debug_error("switchbox: Too long parameter string");
    }
#endif
    osal_socket_embed_default_port(prm->parameters,
        epoint->parameters, SWITCHBOX_END_POINT_PRMSTR_SZ,
        prm->default_port);

    /* If we are already running end point thread, stop it. Wait until it has stopped.
     */
    while (ioc_terminate_switchbox_end_point_thread(epoint))
    {
        ioc_switchbox_unlock(root);
        os_timeslice();
        ioc_switchbox_lock(root);
    }

    /* If we want to run end point in separate thread.
     */
    if (flags & IOC_CREATE_THREAD)
    {
        /* Create an event. Select will react to this event.
           Mark that worker thread is running and thread stop has
           not been requested.
         */
        epoint->worker_thread_running = OS_TRUE;
        epoint->stop_worker_thread = OS_FALSE;

        osalThreadOptParams opt;
        os_memclear(&opt, sizeof(osalThreadOptParams));
        opt.thread_name = "endpoint";
        /* opt.stack_size = 4000; */
        opt.pin_to_core = OS_TRUE;
        opt.pin_to_core_nr = 0;

        osal_thread_create(ioc_switchbox_endpoint_thread, epoint,
            &opt, OSAL_THREAD_DETACHED);
    }

    ioc_switchbox_unlock(root);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Accept incoming TCP sockets.
  @anchor ioc_run_epoint

  The ioc_switchbox_run_endpoint() function is accepts received TCP sockets.

  This function is either called from own thread (multithreading) or from commonioc_run()
  function (no multithreading).

  @param   epoint Pointer to the end point object.
  @return  None.

****************************************************************************************************
*/
void ioc_switchbox_run_endpoint(
    switchboxEndPoint *epoint)
{
    /* Do nothing if ioc_switchbox_listen() has not been called.
     */
    if (epoint->parameters[0] == '\0')
        return;

    /* If listening socket is not open, then open it now and start listening. Do not try if
       if two secons have not passed since last failed open try. Continue in this function
       only with open listening socket.
     */
    if (epoint->socket == OS_NULL)
    {
        if (ioc_try_to_open_switchbox_endpoint(epoint)) return;
    }

    /* Try to accept a socket.
     */
    ioc_try_accept_new_switchbox_sockets(epoint);
}


/**
****************************************************************************************************

  @brief Request to terminate end point worker thread.

  The ioc_terminate_switchbox_end_point_thread() function sets request to terminate worker thread, if
  one is running the end point.

  ioc_switchbox_lock() must be on when this function is called.

  @param   epoint Pointer to the end point object.
  @return  OSAL_SUCCESS if no worker thread is running. OSAL_PENDING if there is .

****************************************************************************************************
*/
osalStatus ioc_terminate_switchbox_end_point_thread(
    switchboxEndPoint *epoint)
{
    osalStatus
        status = OSAL_SUCCESS;

    if (epoint->worker_thread_running)
    {
        epoint->stop_worker_thread = OS_TRUE;
        if (epoint->trig) osal_event_set(epoint->trig);
        status = OSAL_PENDING;
    }

    return status;
}



/**
****************************************************************************************************

  @brief Try to open listening socket port.
  @anchor ioc_try_to_open_switchbox_endpoint

  The ioc_try_to_open_switchbox_endpoint() function tries to open listening TCP socket. Do not try if
  f two secons have not passed since last failed open try.

  @param   epoint Pointer to the end point object.
  @return  OSAL_SUCCESS if we have succesfully opened the listening TCP socket port. Other
           values indicate failure or delay.

****************************************************************************************************
*/
static osalStatus ioc_try_to_open_switchbox_endpoint(
    switchboxEndPoint *epoint)
{
    osalStatus
        status;

    /* If two seconds have not passed since last failed try.
     */
    if (epoint->open_fail_timer_set) {
        if (!os_has_elapsed(&epoint->open_fail_timer, 2000)) {
            return OSAL_PENDING;
        }
    }

    /* Try to open listening socket port.
     */
    epoint->socket = osal_stream_open(epoint->iface, epoint->parameters, OS_NULL, &status,
        OSAL_STREAM_LISTEN /* |OSAL_STREAM_NO_REUSEADDR */);
    if (epoint->socket == OS_NULL)
    {
        osal_debug_error("Opening listening socket failed");
        os_get_timer(&epoint->open_fail_timer);
        epoint->open_fail_timer_set = OS_TRUE;
        return status;
    }

    /* Success.
     */
    epoint->open_fail_timer_set = OS_FALSE;
    epoint->try_accept_timer_set = OS_FALSE;
    ioc_do_switchbox_end_point_callback(epoint, SWITCHBOX_END_POINT_LISTENING);

    osal_trace("end point: listening");
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Try to accept new incoming socket connection.
  @anchor ioc_try_accept_new_switchbox_sockets

  The ioc_try_accept_new_switchbox_sockets() function is accepts received TCP sockets. It is called
  by ioc_run() and should not be called from application.

  @param   epoint Pointer to the end point object.
  @return  Returns OSAL_SUCCESS if successful, regardless if new socket is accepted or not.
           Other return values indicate an error with listening socket (closed now).
           Even if when running out of connection pool, this function must return OSAL_SUCCESS.

****************************************************************************************************
*/
static osalStatus ioc_try_accept_new_switchbox_sockets(
    switchboxEndPoint *epoint)
{
    osalStatus
        status;

    osalStream
        newsocket;

    os_char remote_ip_addr[IOC_CONNECTION_PRMSTR_SZ];

    /* If two seconds have not passed since last failed try. We cannot delay here
     * if we are running with select, we would miss selected events.
     */
    if (!epoint->worker_thread_running)
    {
        if (epoint->try_accept_timer_set &&
            !os_has_elapsed(&epoint->try_accept_timer, 50)) return OSAL_SUCCESS;
        os_get_timer(&epoint->try_accept_timer);
        epoint->try_accept_timer_set = OS_TRUE;
    }

    /* Try to accept an incoming socket connection.
     */
    newsocket = osal_stream_accept(epoint->socket, remote_ip_addr, sizeof(remote_ip_addr),
        &status, OSAL_STREAM_TCP_NODELAY);
    switch (status)
    {
        case OSAL_SUCCESS:
            /* We get success as status, assert that we have the socket struct pointer.
             */
            osal_debug_assert(newsocket != OS_NULL);

            osal_trace("end point: connection accepted");
            if (!ioc_establish_switchbox_connection(epoint, newsocket, remote_ip_addr)) break;
            osal_debug_error("Out of connection pool");
            osal_stream_close(epoint->socket, OSAL_STREAM_DEFAULT);
            epoint->socket = OS_NULL;
            break;

        case OSAL_NO_NEW_CONNECTION:
            break;

        default: /* failed, close the listening socket */
            osal_debug_error("Listening socket broken");
            osal_stream_close(epoint->socket, OSAL_STREAM_DEFAULT);
            epoint->socket = OS_NULL;
            ioc_do_switchbox_end_point_callback(epoint, SWITCHBOX_END_POINT_DROPPED);
            return status;
    }

    /* Success.
     */
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief A socket has been accepted, establish connection.
  @anchor ioc_establish_switchbox_connection

  The ioc_establish_switchbox_connection() function is called once incoming TCP socket is accepted.
  It creates connection object for the accepted socket.

  @param   epoint Pointer to the end point object.
  @param   newsocket Accepted socket handle.
  @return  OSAL_SUCCESS if successful. OSAL_STATUS_FAILED if connection failed and needs to
           be closed. For example pool is given, but there is no space for the connection.

****************************************************************************************************
*/
static osalStatus ioc_establish_switchbox_connection(
    switchboxEndPoint *epoint,
    osalStream newsocket,
    os_char *remote_ip_addr)
{
    switchboxConnection
        *con;

    switchboxConnectionParams
        conprm;

    /* If allocate connection structure either dynamically or from static pool and initialize it.
     */
    con = ioc_initialize_switchbox_connection(epoint->link.root);
    if (con == OS_NULL) return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;

    os_memclear(&conprm, sizeof(conprm));
    conprm.iface = newsocket->iface;
    conprm.parameters = remote_ip_addr;
    conprm.newsocket = newsocket;
    //conprm.flags = epoint->flags;
    return ioc_switchbox_connect(con, &conprm);
}


/**
****************************************************************************************************

  @brief End point thread function.
  @anchor ioc_switchbox_endpoint_thread

  The ioc_switchbox_endpoint_thread() function is worker thread to accept new incoming TCP sockets.

  @param   prm Pointer to parameters for new thread, pointer to end point object.
  @param   done Event to set when parameters have been copied to entry point
           functions own memory.

  @return  None.

****************************************************************************************************
*/
static void ioc_switchbox_endpoint_thread(
    void *prm,
    osalEvent done)
{
    switchboxEndPoint
        *epoint;

    osalStatus
        status;

    osal_trace("end point: worker thread created");

    /* Parameters point to end point object.
     */
    epoint = (switchboxEndPoint*)prm;

    /* Let thread which created this one proceed.
     */
    osal_event_set(done);

    /* Run the end point.
     */
    while (!epoint->stop_worker_thread && osal_go())
    {
static long ulledoo; if (++ulledoo > 0) {osal_debug_error("ulledoo end point\n"); ulledoo = 0;}

        ioc_switchbox_run_endpoint(epoint);

        if (epoint->socket && (epoint->flags & IOC_DISABLE_SELECT) == 0)
        {
            status = osal_stream_select(&epoint->socket, 1, epoint->trig,
                OSAL_INFINITE, OSAL_STREAM_DEFAULT);

            if (status == OSAL_STATUS_NOT_SUPPORTED)
            {
                os_sleep(100);
            }

            else if (status)
            {
                osal_debug_error("osal_stream_select failed");
                osal_stream_close(epoint->socket, OSAL_STREAM_DEFAULT);
                epoint->socket = OS_NULL;
            }
        }
        else
        {
            os_sleep(100);
        }
    }

    osal_stream_close(epoint->socket, OSAL_STREAM_DEFAULT);
    epoint->socket = OS_NULL;

    /* This thread is no longer running.
     */
    epoint->worker_thread_running = OS_FALSE;

    osal_trace("end point: worker thread exited");
}


/**
****************************************************************************************************

  @brief Do callback to indicate that end point is now listening or dropped.
  @anchor ioc_do_switchbox_end_point_callback

  The ioc_do_switchbox_end_point_callback() function calls application's callback function for
  the connection to indicate that end point is really listening or has been dropped (not typical).

  @param   epoint Pointer to the end point object.
  @param   event Either IOC_END_POINT_LISTENING or IOC_END_POINT_DROPPED.


****************************************************************************************************
*/
static void ioc_do_switchbox_end_point_callback(
    switchboxEndPoint *epoint,
    switchboxEndPointEvent event)
{
    if (epoint->callback_func)
    {
        epoint->callback_func(epoint, event, epoint->callback_context);
    }
}


/**
****************************************************************************************************

  @brief Set callback function for switchboxEndPoint object.
  @anchor ioc_set_switchbox_end_point_callback

  The ioc_set_switchbox_end_point_callback function sets callback function and context. The callback
  can be used to inform the application that end point is really listening, and about dropped
  end points.

  @param   epoint Pointer to the end point object.
  @param   func Pointer to a callback function. Set OS_NULL to remove a callback.
  @param   context Application specific context pointer to be passed to the callback function.
  @return  None.

****************************************************************************************************
*/
void ioc_set_switchbox_end_point_callback(
    switchboxEndPoint *epoint,
    switchbox_end_point_callback func,
    void *context)
{
    epoint->callback_func = func;
    epoint->callback_context = context;
}
