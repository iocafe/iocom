
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

    c->msg.hdr.msg_id = LIGHTHOUSE_MSG_ID;
    c->msg.hdr.hdr_sz = (os_uchar)sizeof(LighthouseMessageHdr);
    c->msg.hdr.port_nr_low = (os_uchar)ep_port_nr;
    c->msg.hdr.port_nr_high = (os_uchar)(ep_port_nr >> 8);
    c->msg.hdr.transport = (os_uchar)ep_transport;
    os_strncpy(c->msg.publish, publish, LIGHTHOUSE_PUBLISH_SZ);
    c->msg.hdr.publish_sz = (os_uchar)os_strlen(c->msg.publish); /* Use this, may be cut */
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
    os_memsz bytes;
    os_ushort checksum, random_nr;
    os_int64 tstamp;
    os_ulong ul;
    os_int i;

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
        c->udp_socket = osal_stream_open(OSAL_SOCKET_IFACE, LIGHTHOUSE_PORT,
            OS_NULL, &s, OSAL_STREAM_UDP_MULTICAST);
        if (c->udp_socket == OS_NULL)
        {
            osal_error(OSAL_ERROR, eosal_iocom,
                OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
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

    random_nr = osal_rand(0, 65535);
    c->msg.hdr.random_nr_low = (os_uchar)random_nr;
    c->msg.hdr.random_nr_high = (os_uchar)(random_nr >> 8);

#if OSAL_TIME_SUPPORT
#if OSAL_LONG_IS_64_BITS
    os_time(&tstamp);
    ul = (os_ulong)tstamp;
    for (i = 0; i<8; i++) {
        c->msg.hdr.tstamp[i] = (os_uchar)ul;
        ul >>= 8;
    }
#endif
#endif
    c->msg.hdr.checksum_low = c->msg.hdr.checksum_high = 0;
    bytes = sizeof(LighthouseMessageHdr) + c->msg.hdr.publish_sz;
    checksum = os_checksum((const os_char*)&c->msg, bytes, OS_NULL);
    c->msg.hdr.checksum_low = (os_uchar)checksum;
    c->msg.hdr.checksum_high = (os_uchar)(checksum >> 8);

    /* Send packet to UDP stream
     */
    s = osal_stream_send_packet(c->udp_socket,
        LIGHTHOUSE_IP LIGHTHOUSE_PORT, (const os_char*)&c->msg, bytes, OSAL_STREAM_DEFAULT);
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
