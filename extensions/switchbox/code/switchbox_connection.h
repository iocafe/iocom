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
    Service connection uses this structure to hold linked list of client connections.
****************************************************************************************************
*/
typedef struct switchboxClientList
{
    /** Pointer to the next first client connection with same network name.
     */
    struct switchboxConnection *first;

    /** Pointer to the previous client connection with same network name.
     */
    struct switchboxConnection *last;
}
switchboxClientList;


/**
****************************************************************************************************
    Linked list of client connection with same network name, for one service connection.
****************************************************************************************************
*/
typedef struct switchboxClientLink
{
    /** Pointer to the root object.
     */
    struct switchboxConnection *scon;

    /** Pointer to the next connection in linked list.
     */
    struct switchboxConnection *next;

    /** Pointer to the previous connection in linked list.
     */
    struct switchboxConnection *prev;
}
switchboxClientLink;


/**
****************************************************************************************************
    IOCOM connection structure.
****************************************************************************************************
*/
typedef struct switchboxConnection
{
    /** Service or client connection? OS_TRUE if this is connection to service,
        OS_FALSE if connection to client.
     */
    os_boolean is_service_connection;

    /** Network name. Empty string = any network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** OSAL stream handle (socket or serial port).
     */
    osalStream stream;

    /** Stream interface pointer, one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE
        or OSAL_TLS_IFACE.
     */
    const osalStreamInterface *iface;

    /* Chaining connection with same network name (same service) together.
     */
    union {
        switchboxClientList head;   /* Service connection holds head of the list */
        switchboxClientLink clink;  /* Client connections link together */
    }
    list;

    /** This connection in root's linked list of connections.
     */
    switchboxConnectionLink link;

    /** Timer of the last successful receive.
     */
    os_timer last_receive;

    /** Timer of the last successful send.
     */
    os_timer last_send;

    /** Worker thread specific member variables.
     */
    switchboxConnectionWorkerThread worker;

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
