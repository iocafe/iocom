/**

  @file    lighthouse_server.c
  @brief   Service discovery using UDP multicasts (server).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.2.2020

  The server, or controller, sends periodic UDP multicasts. This allows clients in same network
  to locate the controller without pre configured address.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "lighthouse.h"

/* Forward referred static functions.
 */
static void ioc_initialize_lighthouse_server_one(
    LighthouseServerOne *c,
    const os_char *publish,
    os_int tls_port,
    os_int tcp_port,
    os_boolean is_ipv6);

static void ioc_lighthouse_publish_one(
    LighthouseServerOne *c,
    const os_char *publish,
    const os_char *protocol,
    os_int tls_port,
    os_int tcp_port,
    os_boolean is_ipv6);

static void ioc_release_lighthouse_server_one(
    LighthouseServerOne *c);

static osalStatus ioc_run_lighthouse_server_one(
    LighthouseServerOne *c,
    os_ushort counter,
    os_timer *ti);


/**
****************************************************************************************************

  @brief Initialize the lighthouse server.

  The ioc_initialize_lighthouse_server() function initializes light house server structure
  and stores static information about the service to multicast.

  @param   c Pointer to the light house server object structure.
  @param   publish List of network names to publish, separated by comma.
           For example "iocafenet,asteroidnet".
  @param   ep_port_nr Listening TCP port number.
  @param   ep_transport Transport, either IOC_TLS_SOCKET or IOC_TCP_SOCKET.
  @param   reserved Reserved for future, set OS_NULL for now.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_lighthouse_server(
    LighthouseServer *c,
    const os_char *publish,
    struct osalLighthouseInfo *lighthouse_info,
    void *reserved)
{
    osalLighthouseEndPointInfo *ep;
    os_int i;
    iocTransportEnum transport;

    // iocTransportEnum ipv4_transport = IOC_DEFAULT_TRANSPORT,
    //    ipv6_transport = IOC_DEFAULT_TRANSPORT;

    os_int port, ipv4_tls_port = 0, ipv6_tls_port = 0;
    os_int ipv4_tcp_port = 0, ipv6_tcp_port = 0;

    os_memclear(c, sizeof(LighthouseServer));

    /* Select port number and transport separatelt for IPv4 and IPv6. Choose TLS over plain TCP.
     */
    for (i = 0; i<lighthouse_info->n_epoints; i++)
    {
        ep = &lighthouse_info->epoint[i];
        transport = ep->transport;
        port = ep->port_nr;

        if (ep->is_ipv6) {
            if (transport == IOC_TLS_SOCKET && ipv6_tls_port == 0) {
                ipv6_tls_port = port;
            }
            if (transport == IOC_TCP_SOCKET && ipv6_tcp_port == 0) {
                ipv6_tcp_port = port;
            }
        }
        else {
            if (transport == IOC_TLS_SOCKET && ipv4_tls_port == 0) {
                ipv4_tls_port = port;
            }
            if (transport == IOC_TCP_SOCKET && ipv4_tcp_port == 0) {
                ipv4_tcp_port = port;
            }
        }
    }

    if (ipv4_tls_port || ipv4_tcp_port) {
        ioc_initialize_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV4],
            publish, ipv4_tls_port, ipv4_tcp_port, OS_FALSE);
    }
    if (ipv6_tls_port || ipv6_tcp_port) {
        ioc_initialize_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV6],
            publish, ipv6_tls_port, ipv6_tcp_port, OS_TRUE);
    }
}


void ioc_lighthouse_publish(
    LighthouseServer *c,
    const os_char *publish,
    os_int tls_port,
    os_int tcp_port,
    os_boolean is_ipv6)
{
    /* if (ipv4_tls_port || ipv4_tcp_port) {
        ioc_initialize_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV4],
            publish, ipv4_tls_port, ipv4_tcp_port, OS_FALSE);
    }
    if (ipv6_tls_port || ipv6_tcp_port) {
        ioc_initialize_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV6],
            publish, ipv6_tls_port, ipv6_tcp_port, OS_TRUE);
    } */
}

static void ioc_initialize_lighthouse_server_one(
    LighthouseServerOne *c,
    const os_char *publish,
    os_int tls_port,
    os_int tcp_port,
    os_boolean is_ipv6)
{
    c->multicast_ip = is_ipv6 ? LIGHTHOUSE_IP_IPV6 : LIGHTHOUSE_IP_IPV4;

    os_get_timer(&c->socket_error_timer);
    c->socket_error_timeout = 100;
    os_get_timer(&c->multicast_timer);
    c->multicast_interval = 400;

    c->msg.hdr.msg_id = LIGHTHOUSE_MSG_ID;
    c->msg.hdr.hdr_sz = (os_uchar)sizeof(LighthouseMessageHdr);
    c->msg.hdr.tls_port_nr_low = (os_uchar)tls_port;
    c->msg.hdr.tls_port_nr_high = (os_uchar)(tls_port >> 8);
    c->msg.hdr.tcp_port_nr_low = (os_uchar)tcp_port;
    c->msg.hdr.tcp_port_nr_high = (os_uchar)(tcp_port >> 8);
    // c->msg.hdr.transport = (os_uchar)1;
    // os_strncpy(c->msg.publish, publish, LIGHTHOUSE_PUBLISH_SZ);
    // c->msg.hdr.publish_sz = (os_uchar)os_strlen(c->msg.publish); /* Use this, may be cut */

    ioc_lighthouse_publish_one(c, publish, "i", tls_port, tcp_port, is_ipv6);

}


static void ioc_lighthouse_publish_one(
    LighthouseServerOne *c,
    const os_char *publish,
    const os_char *protocol,
    os_int tls_port,
    os_int tcp_port,
    os_boolean is_ipv6)
{
    os_char buf[LIGHTHOUSE_ITEM_SZ], nbuf[OSAL_NBUF_SZ];
    const os_char *e;
    os_memsz sz;
    os_ushort tls_port_nr, tcp_port_nr;

    while (*publish != '\0')
    {
        e = os_strchr(publish, ',');
        if (e == OS_NULL) {
            e = os_strchr(publish, '\0');
        }

        buf[0] = '\0';
        if (tls_port) {
            os_strncat(buf, is_ipv6 ? "T" : "t", sizeof(buf));
            tls_port_nr = c->msg.hdr.tls_port_nr_high;
            tls_port_nr = (tls_port_nr << 8) | c->msg.hdr.tls_port_nr_low;
            if (tls_port != tls_port_nr) {
                osal_int_to_str(nbuf, sizeof(nbuf), tls_port);
                os_strncat(buf, nbuf, sizeof(buf));
            }
        }
        if (tcp_port) {
            os_strncat(buf, is_ipv6 ? "S" : "s", sizeof(buf));
            tcp_port_nr = c->msg.hdr.tcp_port_nr_high;
            tcp_port_nr = (tcp_port_nr << 8) | c->msg.hdr.tcp_port_nr_low;
            if (tcp_port != tcp_port_nr) {
                osal_int_to_str(nbuf, sizeof(nbuf), tcp_port);
                os_strncat(buf, nbuf, sizeof(buf));
            }
        }
        os_strncat(buf, ":", sizeof(buf));
        os_strncat(buf, protocol, sizeof(buf));
        os_strncat(buf, ":", sizeof(buf));

        sz = e - publish + os_strlen(buf);
        if (sz > sizeof(buf)) {
            sz = sizeof(buf);
        }
        os_strncat(buf, publish, sz);

        if (c->msg.publish[0] != '\0') {
            os_strncat(c->msg.publish, ",", LIGHTHOUSE_PUBLISH_SZ);
        }

        if (os_strncat(c->msg.publish, buf, LIGHTHOUSE_PUBLISH_SZ)) {
            osal_debug_error("lighthouse: \"publish\" buffer overflow");
        }

        if (*e == '\0') break;
        publish = e + 1;
    }
    c->msg.hdr.publish_sz = (os_uchar)os_strlen(c->msg.publish);
}


/**
****************************************************************************************************

  @brief Release resources allocated for lighthouse server.

  The ioc_release_lighthouse_server() function releases the resources allocated for lighthouse
  server. In practice the function closes the socket used to send multicasts.

  @param   c Pointer to the light house server object structure.
  @return  None.

****************************************************************************************************
*/
void ioc_release_lighthouse_server(
    LighthouseServer *c)
{
    if (c->f[LIGHTHOUSE_IPV4].msg.hdr.msg_id)
    {
        ioc_release_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV4]);
    }

    if (c->f[LIGHTHOUSE_IPV6].msg.hdr.msg_id)
    {
        ioc_release_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV6]);
    }
}


static void ioc_release_lighthouse_server_one(
    LighthouseServerOne *c)
{
    if (c->udp_socket)
    {
        osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
        c->udp_socket = OS_NULL;
    }
}


/**
****************************************************************************************************

  @brief Keep lighthouse server functionality alive, send UDP multicasts.

  The ioc_run_lighthouse_server() function is called repeatedly keep sending UDP multicast about
  once per four seconds. This informs IO devices (clients) that the service is here.

  @param   c Pointer to the light house client object structure.
  @param   ti Pointer to current timer value. If OS_NULL, timer is read with os_get_timer().
  @return  OSAL_SUCCESS or OSAL_PENDING if all is fine. Latter indicates that we are waiting
           for next time to try to open a socket. Other values indicate a network error.

****************************************************************************************************
*/
osalStatus ioc_run_lighthouse_server(
    LighthouseServer *c,
    os_timer *ti)
{
    os_timer tval;
    osalStatus s4 = OSAL_SUCCESS, s6 = OSAL_SUCCESS;

    if (ti == OS_NULL)
    {
        os_get_timer(&tval);
        ti = &tval;
    }

    if (c->f[LIGHTHOUSE_IPV4].msg.hdr.msg_id)
    {
        s4 = ioc_run_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV4], c->counter, ti);
    }

    if (c->f[LIGHTHOUSE_IPV6].msg.hdr.msg_id)
    {
        s6 = ioc_run_lighthouse_server_one(&c->f[LIGHTHOUSE_IPV6], c->counter, ti);
    }

    /* If multicast was sent using either protocol, increment multicast counter
     */
    if (s4 == OSAL_SUCCESS || s6 == OSAL_SUCCESS) {
        c->counter++;
    }

    return s4 ? s4 : s6;
}

/* OSAL_SUCCESS is returned only if multicast is sent. OSAL_PENDING indicates
 * that all is fine now, but no multicast yet sent
 */
static osalStatus ioc_run_lighthouse_server_one(
    LighthouseServerOne *c,
    os_ushort counter,
    os_timer *ti)
{
    osalStatus s;
    os_memsz bytes;
    os_ushort checksum, random_nr;

#if OSAL_TIME_SUPPORT
#if OSAL_LONG_IS_64_BITS
    os_int64 tstamp;
    os_ulong ul;
    os_int i;
#endif
#endif

    /* If UDP socket is not open
     */
    if (c->udp_socket == OS_NULL)
    {
        /* If not enough time has passed since last try.
         */
        if (!os_has_elapsed(&c->socket_error_timer, c->socket_error_timeout))
        {
            return OSAL_PENDING;
        }
        os_get_timer(&c->socket_error_timer);
        c->socket_error_timeout = 5000;

        /* Try to open UDP socket. Set error state.
         */
        c->udp_socket = osal_stream_open(OSAL_SOCKET_IFACE, LIGHTHOUSE_PORT,
            (void*)c->multicast_ip, &s, OSAL_STREAM_MULTICAST|OSAL_STREAM_USE_GLOBAL_SETTINGS);
        if (c->udp_socket == OS_NULL)
        {
            osal_error(OSAL_ERROR, iocom_mod,
                OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
            return s;
        }
        osal_error(OSAL_CLEAR_ERROR, iocom_mod,
            OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
    }

    /* If not enough time has passed to send next UDP multicast.
     */
    if (!os_has_elapsed_since(&c->multicast_timer, ti, c->multicast_interval))
    {
        return OSAL_PENDING;
    }
    c->multicast_timer = *ti;
    c->multicast_interval = 4000;

    random_nr = (os_ushort)osal_rand(0, 65535);
    c->msg.hdr.random_nr_low = (os_uchar)random_nr;
    c->msg.hdr.random_nr_high = (os_uchar)(random_nr >> 8);

    c->msg.hdr.counter_low = (os_uchar)counter;
    c->msg.hdr.counter_high = (os_uchar)(counter >> 8);

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
    bytes = sizeof(LighthouseMessageHdr) + (os_memsz)c->msg.hdr.publish_sz;
    checksum = os_checksum((const os_char*)&c->msg, bytes, OS_NULL);
    c->msg.hdr.checksum_low = (os_uchar)checksum;
    c->msg.hdr.checksum_high = (os_uchar)(checksum >> 8);

    /* Send packet to UDP stream
     */
    s = osal_stream_send_packet(c->udp_socket, (const os_char*)&c->msg, bytes, OSAL_STREAM_DEFAULT);
    if (OSAL_IS_ERROR(s))
    {
        osal_error(OSAL_ERROR, iocom_mod,
            OSAL_STATUS_SEND_MULTICAST_FAILED, OS_NULL);

        osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
        c->udp_socket = OS_NULL;
        return s;
    }

    return OSAL_SUCCESS;
}
