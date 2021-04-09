/**

  @file    switchbox_connection.h
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
#pragma once
#ifndef SWITCHBOX_CONNECTION_H_
#define SWITCHBOX_CONNECTION_H_
#include "iocom.h"


/**
****************************************************************************************************
    Connection related defines.
****************************************************************************************************
*/



/**
****************************************************************************************************
    Parameters for ioc_switchbox_connect() function.
****************************************************************************************************
*/
typedef struct switchboxConnectionParams
{
    /** Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_BLUETOOTH_IFACE, OSAL_SOCKET_IFACE
        or OSAL_TLS_IFACE defines.
     */
    const osalStreamInterface *iface;

    /** Depending on connection type, can be "127.0.0.1:8817" for TCP socket.
     */
    const os_char *parameters;

    /** If socket connection is accepted by listening end point, this is
        the socket handle. Otherwise this argument needs to be OS_NULL.
     */
    osalStream newsocket;
}
switchboxConnectionParams;



/**
****************************************************************************************************
    Worker thread specific member variables.
****************************************************************************************************
*/
typedef struct switchboxConnectionWorkerThread
{
    /** Event to activate the worker thread.
     */
    osalEvent trig;

    /** True If running worker thread for the end point.
     */
    os_boolean thread_running;

    /** Flag to terminate worker thread.
     */
    os_boolean stop_thread;
}
switchboxConnectionWorkerThread;



/**
****************************************************************************************************
    This connection in root's linked list of connections.
****************************************************************************************************
*/
typedef struct switchboxConnectionLink
{
    /** Pointer to the root object.
     */
    switchboxRoot *root;

    /** Pointer to the next connection in linked list.
     */
    struct switchboxConnection *next;

    /** Pointer to the previous connection in linked list.
     */
    struct switchboxConnection *prev;
}
switchboxConnectionLink;


/**
****************************************************************************************************
    IOCOM connection structure.
****************************************************************************************************
*/
typedef struct switchboxConnection
{
    /** Flags as given to ioc_switchbox_connect(): define IOC_SOCKET, IOC_CLOSE_CONNECTION_ON_ERROR
        IOC_CONNECT_UP...
     */
    // os_short flags;

    /** Parameter string
     */
    // os_char parameters[IOC_CONNECTION_PRMSTR_SZ];

    /** OSAL stream handle (socket or serial port).
     */
    osalStream stream;

    /** Stream interface pointer, one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE
        or OSAL_TLS_IFACE.
     */
    const osalStreamInterface *iface;

    /** Timer of the last successful receive.
     */
    os_timer last_receive;

    /** Timer of the last successful send.
     */
    os_timer last_send;

    /** Worker thread specific member variables.
     */
    switchboxConnectionWorkerThread worker;

    /** This connection in root's linked list of connections.
     */
    switchboxConnectionLink link;

    /** Handshake state structure (switbox cloud net name and copying trust certificate).
     */
    iocHandshakeState handshake;

    /** First handshake successfully completed after connect.
     */
    os_boolean handshake_ready;

    /** Authentication data sent to connection flag. We must send and receive authentication
        data before sending anything else.
     */
    // os_boolean authentication_sent;

    /** Authentication data received from connection flag.
     */
    // os_boolean authentication_received;

    /** Flag indicating that stream is connected. Connected
        means that one message has been successfully received.
     */
    // os_boolean connected;

    /** The allowed_networks is structure set up by user authentication to hold list of networks
        which can be accessed trough the connection and privileges for each network. Must be released
        by ioc_release_allowed_networks().
     */
//     iocAllowedNetworkConf allowed_networks;
}
switchboxConnection;


/**
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_switchbox_connection() function initializes or allocates new connection object,
  and ioc_release_switchbox_connection() releases resources associated with it. Memory allocated for the
  connection is freed, if the memory was allocated by ioc_initialize_switchbox_connection().

****************************************************************************************************
 */
/*@{*/

/* Initialize connection object.
 */
switchboxConnection *ioc_initialize_switchbox_connection(
    switchboxRoot *root);

/* Release connection object.
 */
void ioc_release_switchbox_connection(
    switchboxConnection *con);

/* Close underlying socket or serial port.
 */
void ioc_close_switchbox_service_stream(
    switchboxConnection *con);

/* Start or prepare the connection.
 */
osalStatus ioc_switchbox_connect(
    switchboxConnection *con,
    switchboxConnectionParams *prm);

/* Request to terminate connection worker thread.
 */
osalStatus ioc_terminate_switchbox_connection_thread(
    switchboxConnection *con);

/* Reset connection state to start from beginning
 */
void ioc_reset_switchbox_connection(
    switchboxConnection *con);

/*@}*/

#endif
