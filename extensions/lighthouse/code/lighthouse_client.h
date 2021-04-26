/**

  @file    lighthouse_client.h
  @brief   Service discovery using UDP multicasts (client).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The lighthouse client can be used by an IO device to detect controller in local area
  network. It monitors for UDP multicasts sent by server and collects network information.

  Server to connect to (IP address and port) is resolved by IO network name and transport.
  The library supports also auto selecting IO network, assuming the local net has only
  one IO network using the specified transport.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef LIGHTHOUSE_CLIENT_H_
#define LIGHTHOUSE_CLIENT_H_
#include "lighthouse.h"

struct LighthouseClient;

/**
****************************************************************************************************
  Lighthouse client data structures
****************************************************************************************************
 */
/** Information about one IO network known by light house.
 */
typedef struct LightHouseNetwork
{
    /** Server IP address.
     */
    os_char ip_addr[OSAL_IPADDR_SZ];

    /** Server TCP port.
     */
    os_int port_nr;

    /** Transport, either IOC_TCP_SOCKET or IOC_TLS_SOCKET.
     */
    iocTransportEnum transport;

    /** IO network name, like "cafenet"
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Timer when network information was received. Used to replace the oldest of too
        many networks.
     */
    os_timer received_timer;
}
LightHouseNetwork;

typedef struct LightHouseClientCallbackData
{
    os_char *ip_addr;
    os_int tls_port_nr;
    os_int tcp_port_nr;
    os_char *network_name;
    os_char *protocol;
    os_char *nickname;

    os_ushort counter; /* Broadcast counter by this service */
}
LightHouseClientCallbackData;


typedef void ioc_lighthouse_client_callback(
    struct LighthouseClient *c,
    LightHouseClientCallbackData *data,
    void *context);


/** How many networks we can remember.
 */
#ifndef LIGHTHOUSE_NRO_NETS
  #if OSAL_MICROCONTROLLER
    #define LIGHTHOUSE_NRO_NETS 4
  #else
    #define LIGHTHOUSE_NRO_NETS 32
  #endif
#endif

/** Light house client state
 */
typedef struct LighthouseClient
{
    /** Multicast group IP address.
     */
    const os_char *multicast_ip;

    /** UDP socket handle. OS_NULL if UDP socket is not open.
     */
    osalStream udp_socket;

    /** Timer for retrying UDP socket open.
     */
    os_timer socket_error_timer;

    /** Time out for retrying, ms (socket_error_timer).
     */
    os_int socket_error_timeout;

    /** Counter to ocassionally check if there is expired network information to delete.
     */
    os_int check_expired_count;

    /** Information about knwon networks.
     */
    LightHouseNetwork net[LIGHTHOUSE_NRO_NETS];

    /** Network name we are looking for to get faster indication. Empty string if unknown.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Timer when last multicast was received. Used to close and reopen the socket
        if light house stops receiveing (problematic on some micro-controllers).
      */
    os_timer multicast_received;

    /** Lighthouse if used for real in this device (no static IP settings, etc)
     */
    os_boolean lighthouse_really_needed;

    /** Set to select connection with transport layer security. OS_FALSE to select
        plain unsecure socket.
     */
    os_boolean select_tls;

    /** Callback function and context.
     */
    ioc_lighthouse_client_callback *func;
    void *context;
}
LighthouseClient;


/**
****************************************************************************************************
  Lighthouse client functions
****************************************************************************************************
 */
/* Initialize the lighthouse client.
 */
void ioc_initialize_lighthouse_client(
    LighthouseClient *c,
    os_boolean is_ipv6,
    os_boolean is_tls,
    void *reserved);

/* Release resources allocated for lighthouse client.
 */
void ioc_release_lighthouse_client(
    LighthouseClient *c);

/* Set callback function when new connection is established.
 */
void ioc_set_lighthouse_client_callback(
    LighthouseClient *c,
    ioc_lighthouse_client_callback func,
    void *context);

/* Check if lighthouse is to be used with this host name.
 */
os_boolean ioc_is_lighthouse_used(
    const os_char *hostname,
    os_boolean *is_ipv6_wildcard);

/* Keep lighthouse client functionality alive.
 */
osalStatus ioc_run_lighthouse_client(
    LighthouseClient *c,
    osalEvent trigger);

/* Get server (controller) IP address and port by transport,
 * if received by UDP broadcast.
 */
osalStatus ioc_get_lighthouse_connectstr(
    LighthouseClient *c,
    LighthouseFuncNr func_nr,
    os_char *network_name,
    os_short flags,
    os_char *connectstr,
    os_memsz connectstr_sz);


#endif
