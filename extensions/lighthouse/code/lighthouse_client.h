/**

  @file    lighthouse_client.h
  @brief   Service discovery using UDP multicasts (client).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.2.2020

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

    /** IO network name, like "iocafenet"
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Timer when network information was received. Used to replace the oldest of too
        many networks.
     */
    os_timer received_timer;
}
LightHouseNetwork;

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
    /** UDP socket handle. OS_NULL if UDP socket is not open.
     */
    osalStream udp_socket;

    /** Timer for retrying UDP socket open.
     */
    os_timer socket_error_timer;

    /** Time out for retrying, ms (socket_error_timer).
     */
    os_int socket_error_timeout;

    /** Information about knwon networks.
     */
    LightHouseNetwork net[LIGHTHOUSE_NRO_NETS];
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
    void *reserved);

/* Release resources allocated for lighthouse client.
 */
void ioc_release_lighthouse_client(
    LighthouseClient *c);

/* Keep lighthouse client functionality alive.
 */
osalStatus ioc_run_lighthouse_client(
    LighthouseClient *c);

/* Get server (controller) IP address and port by transport,
 * if received by UDP broadcast.
 */
osalStatus ioc_get_lighthouse_connectstr(
    LighthouseClient *c,
    LighthouseFuncNr func_nr,
    os_char *network_name,
    os_memsz network_name_sz,
    os_short flags,
    os_char *connectstr,
    os_memsz connectstr_sz);
