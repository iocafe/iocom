/**

  @file    lighthouse_server.h
  @brief   Service discovery using UDP multicasts (server).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.2.2020

  The server, or controller, sends periodic UDP multicasts. This allows clients in same network
  to locate the controller without pre configured address.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef LIGHTHOUSE_SERVER_H_
#define LIGHTHOUSE_SERVER_H_
#include "lighthouse.h"

struct osalLighthouseInfo;

typedef enum
{
    LIGHTHOUSE_IPV4 = 0,
    LIGHTHOUSE_IPV6 = 1,
    LIGHTHOUSE_NRO_ADDR_FAMILIES = 2
}
LighthouseAddressFamily;

/* Maximum published item string size.
 */
#define LIGHTHOUSE_ITEM_SZ (OSAL_IPADDR_AND_PORT_SZ + 4)

/**
****************************************************************************************************
  Lighthouse server structure for one IP address family
****************************************************************************************************
 */
typedef struct LighthouseServerOne
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

    /** Timer for sending UDP multicasts.
     */
    os_timer multicast_timer;

    /** How often to send UDP multicasts, ms.
     */
    os_int multicast_interval;

    /** Outgoing message buffer.
     */
    LighthouseMessage msg;
}
LighthouseServerOne;


/**
****************************************************************************************************
  Lighthouse server structure
****************************************************************************************************
 */
typedef struct LighthouseServer
{
    LighthouseServerOne f[LIGHTHOUSE_NRO_ADDR_FAMILIES];

    /** Number of multicast messages sent. Wraps around at 65535, needs to be 16 bit unsigned.
     */
    os_ushort counter;
}
LighthouseServer;


/**
****************************************************************************************************
  Lighthouse server functions
****************************************************************************************************
 */
/* Initialize the lighthouse server.
 */
void ioc_initialize_lighthouse_server(
    LighthouseServer *c,
    const os_char *publish,
    struct osalLighthouseInfo *lighthouse_info,
    const os_char *nickname);

/* Release resources allocated for lighthouse server.
 */
void ioc_release_lighthouse_server(
    LighthouseServer *c);

/* Keep lighthouse server functionality alive.
 */
osalStatus ioc_run_lighthouse_server(
    LighthouseServer *c,
    os_timer *ti);

#endif
