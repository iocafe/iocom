/**

  @file    lighthouse_client.h
  @brief   Service discovery using UDP multicasts (client).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.2.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


/**
****************************************************************************************************
  X...
****************************************************************************************************
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
}
LighthouseClient;


/**
****************************************************************************************************
  Functions
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
osalStatus ioc_get_lighthouse_server(
    LighthouseClient *c);
