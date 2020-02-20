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

    /* Last received complete message
     */
    LighthouseMessage msg;
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
osalStatus ioc_get_lighthouse_connectstr(
    LighthouseClient *c,
    LighthouseFuncNr func_nr,
    os_char *network_name,
    os_memsz network_name_sz,
    os_short flags,
    os_char *connectstr,
    os_memsz connectstr_sz);
