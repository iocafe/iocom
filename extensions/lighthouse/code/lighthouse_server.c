
/**

  @file    lighthouse_server.c
  @brief   Service discovery using UDP multicasts (server).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.2.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "lighthouse.h"

/* Initialize the lighthouse server.
   @param   ep_port_nr Listening TCP port number.
   @param   ep_transport Transport, either IOC_TLS_SOCKET or IOC_TCP_SOCKET.
 */
void ioc_initialize_lighthouse_server(
    LighthouseServer *c,
    const os_char *publish,
    os_int ep_port_nr,
    iocTransportEnum ep_transport)
{

}

/* Release resources allocated for lighthouse server.
 */
void ioc_release_lighthouse_server(
    LighthouseServer *c)
{
}

/* Keep lighthouse server functionality alive.
 */
osalStatus ioc_run_lighthouse_server(
    LighthouseServer *c)
{
    return OSAL_SUCCESS;
}
