/**

  @file    lighthouse_client.c
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
#include "lighthouse.h"


/* Initialize the lighthouse client.
 */
void ioc_initialize_lighthouse_client(
    LighthouseClient *c,
    void *reserved)
{
    os_memclear(c, sizeof(LighthouseClient));
    os_get_timer(&c->socket_error_timer);
    c->socket_error_timeout = 100;
}

/* Release resources allocated for lighthouse client.
 */
void ioc_release_lighthouse_client(
    LighthouseClient *c)
{
    if (c->udp_socket)
    {
        osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
        c->udp_socket = OS_NULL;
    }
}

/* Keep lighthouse client functionality alive.
 */
osalStatus ioc_run_lighthouse_client(
    LighthouseClient *c)
{
    osalStatus s;
    os_char buf[512];
    os_char remote_addr[OSAL_IPADDR_SZ];
    os_memsz n_read;

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
        c->udp_socket = osal_stream_open(OSAL_SOCKET_IFACE,
            LIGHTHOUSE_PORT, LIGHTHOUSE_IP, &s,
            OSAL_STREAM_UDP_MULTICAST|OSAL_STREAM_LISTEN);
        if (c->udp_socket == OS_NULL)
        {
            osal_error(OSAL_ERROR, eosal_iocom,
                OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, "5766");
            return s;
        }
        osal_error(OSAL_CLEAR_ERROR, eosal_iocom,
            OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
    }

    /* Send packet to UDP stream
     */
    s = osal_stream_receive_packet(c->udp_socket, buf, sizeof(buf), &n_read,
        remote_addr, sizeof(remote_addr), OSAL_STREAM_DEFAULT);
    if (OSAL_IS_ERROR(s))
    {
        osal_error(OSAL_ERROR, eosal_iocom,
            OSAL_STATUS_RECEIVING_UDP_PACKET_FAILED, OS_NULL);

        osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
        c->udp_socket = OS_NULL;
        return s;
    }

    return OSAL_SUCCESS;
}

/* Get server (controller) IP address and port by transport,
 * if received by UDP broadcast.
 */
osalStatus ioc_get_lighthouse_server(
    LighthouseClient *c)
{
    return OSAL_SUCCESS;
}
