
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
    os_memclear(c, sizeof(LighthouseServer));
    os_get_timer(&c->socket_error_timer);
    c->socket_error_timeout = 100;
    os_get_timer(&c->multicast_timer);
    c->multicast_interval = 200;
}

/* Release resources allocated for lighthouse server.
 */
void ioc_release_lighthouse_server(
    LighthouseServer *c)
{
    if (c->udp_socket)
    {
        osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
        c->udp_socket = OS_NULL;
    }
}

/* Keep lighthouse server functionality alive.
 */
osalStatus ioc_run_lighthouse_server(
    LighthouseServer *c)
{
    osalStatus s;

    /* If UDP socket is not open
     */
    if (c->udp_socket == OS_NULL)
    {
        /* If not enough time has passed since last try.
         */
        if (!os_elapsed(&c->socket_error_timer, c->socket_error_timeout))
        {
            return OSAL_PENDING;
        }
        os_get_timer(&c->socket_error_timer);
        c->socket_error_timeout = 5000;

        /* Try to open UDP socket. Set error state.
         */
        c->udp_socket = osal_stream_open(OSAL_SOCKET_IFACE, "5766",
            OS_NULL, &s, OSAL_STREAM_UDP_MULTICAST);
        if (c->udp_socket == OS_NULL)
        {
            osal_error(OSAL_ERROR, eosal_iocom,
                OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, "5766");
            return s;
        }
        osal_error(OSAL_CLEAR_ERROR, eosal_iocom,
            OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
    }

    /* If not enough time has passed to send next UDP multicast.
     */
    if (!os_elapsed(&c->multicast_timer, c->multicast_interval))
    {
        return OSAL_SUCCESS;
    }
    os_get_timer(&c->multicast_timer);
    c->multicast_interval = 4000;

    /* Send packet to UDP stream
     */
    s = osal_stream_send_packet(c->udp_socket, "?", "MYdata", 5, OSAL_STREAM_DEFAULT);
    if (OSAL_IS_ERROR(s))
    {
        osal_error(OSAL_ERROR, eosal_iocom,
            OSAL_STATUS_SENDING_UDP_PACKET_FAILED, OS_NULL);

        osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
        c->udp_socket = OS_NULL;
        return s;
    }

    return OSAL_SUCCESS;
}
