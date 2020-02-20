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

/**
****************************************************************************************************
  Lighthouse server structure
****************************************************************************************************
 */
typedef struct LighthouseServer
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
    os_int ep_port_nr,
    iocTransportEnum ep_transport);

/* Release resources allocated for lighthouse server.
 */
void ioc_release_lighthouse_server(
    LighthouseServer *c);

/* Keep lighthouse server functionality alive.
 */
osalStatus ioc_run_lighthouse_server(
    LighthouseServer *c);
