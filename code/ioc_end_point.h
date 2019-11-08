/**

  @file    ioc_end_point.h
  @brief   End point object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.8.2018

  An end_point listens TCP socket for incoming connections and accepts these. Once a socket
  connection is received, a iocConnection object is created for it to transfer the data.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#ifndef IOC_END_POINT_INCLUDED
#define IOC_END_POINT_INCLUDED

/* Maximum parameter string length for end point.
 */
#define IOC_END_POINT_PRMSTR_SZ 32

/**
****************************************************************************************************
    Parameters for ioc_listen() function.
****************************************************************************************************
*/
typedef struct
{
    /** Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE or OSAL_TLS_IFACE defines.
     */
    const osalStreamInterface *iface;

    /** Parameters For example ":8817" or "127.0.0.1:8817" for TCP socket.
     */
    const os_char *parameters;
    
    /** Flags, bit fields:
        - IOC_SOCKET Connect with TCP socket (set always).
        - IOC_CREATE_THREAD Create thread to run end_point and create thread to run
          each accepted connection (multithread support needed).
     */
    os_short flags;
} 
iocEndPointParams;

/**
****************************************************************************************************
    This end point in root's linked list of end points.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the root object.
     */
    iocRoot *root;

    /** Pointer to the next end_point in linked list.
     */
    struct iocEndPoint *next;

    /** Pointer to the previous end_point in linked list.
     */
    struct iocEndPoint *prev;
}
iocEndPointLink;


/**
****************************************************************************************************

  @name End point object structure.

  X...

****************************************************************************************************
*/
typedef struct iocEndPoint
{
    /** Debug identifier must be first item in the object structure. It is used to verify
        that a function argument is pointer to correct initialized object.
     */
    IOC_DEBUG_ID

    /** Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE or OSAL_TLS_IFACE defines.
     */
    const osalStreamInterface *iface;

    /** Flags as given to ioc_listen()
     */
    os_short flags;

    /** Parameter string
     */
    os_char parameters[IOC_END_POINT_PRMSTR_SZ];

    /** OSAL socket handle.
     */
    osalStream socket;

    /** Timer to measure how long since last failed socket open try.
        Zero if socket has not been tried or it has succeeded the
        last time.
     */
    os_timer socket_open_fail_timer;

    /** Timer for accepting new incoming TCP socket connections. We do not do accept on
        every run, because we do not know how heavy the socket library accept function
        implementation is.
     */
    os_timer try_accept_timer;

#if OSAL_MULTITHREAD_SUPPORT
    /** Event to activate the worker thread.
     */
    osalEvent trig;

    /** TRUE If running worker thread for the end point.
     */
    os_boolean worker_thread_running;

    /** Flag to terminate worker thread.
     */
    os_boolean stop_worker_thread;
#endif

    /** This end point in root's linked list of end points.
     */
    iocEndPointLink link;

    /** Flag indicating that the end_point structure was dynamically allocated.
     */
    os_boolean allocated;
}
iocEndPoint;


/** 
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_end_point() function initializes or allocates new end_point object,
  and ioc_release_end_point() releases resources associated with it. Memory allocated for the
  end_point is freed, if the memory was allocated by ioc_initialize_end_point().

  The ioc_read() and ioc_write() functions are used to access data in end_point.

****************************************************************************************************
 */
/*@{*/

/* Initialize end_point object.
 */
iocEndPoint *ioc_initialize_end_point(
    iocEndPoint *epoint,
    iocRoot *root);

/* Release end_point object.
 */
void ioc_release_end_point(
    iocEndPoint *epoint);

/* Start or prepare the end point to listen for TCP socket connections.
 */
osalStatus ioc_listen(
    iocEndPoint *epoint,
    iocEndPointParams *prm);

/* Accept incoming TCP sockets.
 */
void ioc_run_endpoint(
    iocEndPoint *epoint);

#if OSAL_MULTITHREAD_SUPPORT
/* Request to terminate end point worker thread.
 */
osalStatus ioc_terminate_end_point_thread(
    iocEndPoint *epoint);
#endif

/*@}*/

#endif
