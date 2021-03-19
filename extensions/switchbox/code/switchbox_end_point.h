/**

  @file    switchbox_end_point.h
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
#pragma once
#ifndef SWITCHBOX_END_POINT_H_
#define SWITCHBOX_END_POINT_H_
#include "switchbox.h"

struct switchboxEndPoint;

/* Maximum parameter string length for end point.
 */
#define IOC_END_POINT_PRMSTR_SZ 32

/**
****************************************************************************************************
    Parameters for ioc_switchbox_listen() function.
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
switchboxEndPointParams;

/**
****************************************************************************************************
    This end point in root's linked list of end points.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the root object.
     */
    switchboxRoot *root;

    /** Pointer to the next end_point in linked list.
     */
    struct switchboxEndPoint *next;

    /** Pointer to the previous end_point in linked list.
     */
    struct switchboxEndPoint *prev;
}
switchboxEndPointLink;


/**
****************************************************************************************************
    End point object structure.
****************************************************************************************************
*/
typedef struct switchboxEndPoint
{
    /** Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE or OSAL_TLS_IFACE defines.
     */
    const osalStreamInterface *iface;

    /** Flags as given to ioc_switchbox_listen()
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
    os_timer open_fail_timer;

    /** Timer for accepting new incoming TCP socket connections. We do not do accept on
        every run, because we do not know how heavy the socket library accept function
        implementation is.
     */
    os_timer try_accept_timer;

    /** Event to activate the worker thread.
     */
    osalEvent trig;

    /** TRUE If running worker thread for the end point.
     */
    os_boolean worker_thread_running;

    /** Flag to terminate worker thread.
     */
    os_boolean stop_worker_thread;

    os_boolean try_accept_timer_set;
    os_boolean open_fail_timer_set;

    /** This end point in root's linked list of end points.
     */
    switchboxEndPointLink link;

    /** Flag indicating that the end_point structure was dynamically allocated.
     */
    os_boolean allocated;
}
switchboxEndPoint;


/**
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_switchbox_end_point() function initializes or allocates new end_point object,
  and ioc_release_switchbox_end_point() releases resources associated with it. Memory allocated for the
  end_point is freed, if the memory was allocated by ioc_initialize_switchbox_end_point().

****************************************************************************************************
 */
/*@{*/

/* Initialize end_point object.
 */
switchboxEndPoint *ioc_initialize_switchbox_end_point(
    switchboxEndPoint *epoint,
    switchboxRoot *root);

/* Release end_point object.
 */
void ioc_release_switchbox_end_point(
    switchboxEndPoint *epoint);

/* Start or prepare the end point to listen for TCP socket connections.
 */
osalStatus ioc_switchbox_listen(
    switchboxEndPoint *epoint,
    switchboxEndPointParams *prm);

/* Accept incoming TCP sockets.
 */
void ioc_switchbox_run_endpoint(
    switchboxEndPoint *epoint);

/* Request to terminate end point worker thread.
 */
osalStatus ioc_terminate_switchbox_end_point_thread(
    switchboxEndPoint *epoint);

/*@}*/

#endif
