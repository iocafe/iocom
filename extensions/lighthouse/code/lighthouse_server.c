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
static void ioc_lighthouse_try_set_default_ports(
    LighthouseServer *c,
    os_int port,
    iocTransportEnum transport,
    LighthouseAddressFamily ipfamily);

static void ioc_release_lighthouse_server_one(
    LighthouseServerOne *c);

static osalStatus ioc_run_lighthouse_server_one(
    LighthouseServerOne *c,
    os_ushort counter,
    os_timer *ti);


/**
****************************************************************************************************

  @brief Initialize the lighthouse server structure.

  The ioc_initialize_lighthouse_server() function sets up light house server structure for use.

  @param   c Pointer to the light house server object structure.
  @param   interval_ms Time between sent between UDP multicasts, ms. Multicasts will not be sent
           faster than this, but can be sent at maximun at run call rate.

  @return  None.

****************************************************************************************************
*/
void ioc_initialize_lighthouse_server(
    LighthouseServer *c,
    os_int interval_ms)
{
    LighthouseServerOne *f;
    os_timer ti;
    os_int i;

    os_memclear(c, sizeof(LighthouseServer));

    os_get_timer(&ti);
    ti -= interval_ms;
    for (i = 0; i < LIGHTHOUSE_NRO_ADDR_FAMILIES; i++)
    {
        ti -= 20;
        f = &c->f[i];
        f->socket_error_timer = ti;
        f->socket_error_timeout = 100;
        f->multicast_timer = ti;
        f->multicast_interval = 200;
        if (f->multicast_interval > interval_ms) {
            f->multicast_interval = interval_ms;
        }
        f->multicast_interval_max = interval_ms;
        f->msg.hdr.msg_id = LIGHTHOUSE_MSG_ID;
        f->msg.hdr.hdr_sz = (os_uchar)sizeof(LighthouseMessageHdr);
    }


    c->f[LIGHTHOUSE_IPV4].multicast_ip = LIGHTHOUSE_IP_IPV4;
    c->f[LIGHTHOUSE_IPV6].multicast_ip = LIGHTHOUSE_IP_IPV6;
}



/**
****************************************************************************************************

  @brief Start end point information setup.

  The ioc_lighthouse_start_endpoints() function starts (or restarts) the end point data setup.
  The function must be called after ioc_initialize_lighthouse_server() and before adding
  iocom end points ioc_lighthouse_add_iocom_endpoints.

  The function uses global nickname of the device or process.

  @param   c Pointer to the light house server object structure.
  @return  None.

****************************************************************************************************
*/
void ioc_lighthouse_start_endpoints(
    LighthouseServer *c)
{
    LighthouseServerOne *f;
    os_int i;

    for (i = 0; i < LIGHTHOUSE_NRO_ADDR_FAMILIES; i++)
    {
        f = &c->f[i];
        os_strncpy(f->msg.publish, osal_nickname(), LIGHTHOUSE_PUBLISH_SZ);
        f->msg.hdr.tls_port_nr_low = f->msg.hdr.tls_port_nr_high = 0;
        f->msg.hdr.tcp_port_nr_low = f->msg.hdr.tcp_port_nr_high = 0;
        f->is_configured = OS_FALSE;
    }

    c->f[LIGHTHOUSE_IPV4].multicast_ip = LIGHTHOUSE_IP_IPV4;
    c->f[LIGHTHOUSE_IPV6].multicast_ip = LIGHTHOUSE_IP_IPV6;
}


/**
****************************************************************************************************

  @brief Add information about IOCOM protocol end points.

  The ioc_lighthouse_add_iocom_endpoints() function stores static information about the IOCOM
  end points for UDP multicasts.

  @param   c Pointer to the light house server object structure.
  @param   publish List of network names to publish, separated by comma.
           For example "cafenet,asteroidnet".
  @param   end_point_info Information about end points.
  @return  None.

****************************************************************************************************
*/
void ioc_lighthouse_add_iocom_endpoints(
    LighthouseServer *c,
    const os_char *publish,
    struct osalLighthouseInfo *end_point_info)
{
    osalLighthouseEndPointInfo *ep;
    LighthouseServerOne *f;
    LighthouseAddressFamily ipfamily;
    os_int port_nrs[LIGHTHOUSE_NRO_ADDR_FAMILIES][2], port_nr;
    os_int i;

    os_memclear(port_nrs, sizeof(port_nrs));
    for (i = 0; i<end_point_info->n_epoints; i++)
    {
        ep = &end_point_info->epoint[i];
        ipfamily = ep->is_ipv6 ? LIGHTHOUSE_IPV6 : LIGHTHOUSE_IPV4;
        ioc_lighthouse_try_set_default_ports(c, ep->port_nr, ep->transport, ipfamily);

        f = &c->f[ipfamily];
        port_nr = f->msg.hdr.tcp_port_nr_high;
        port_nr = (port_nr << 8) | f->msg.hdr.tcp_port_nr_low;
        if (port_nr && port_nrs[ipfamily][0] == 0) {
            port_nrs[ipfamily][0] = port_nr;
        }

        port_nr = f->msg.hdr.tls_port_nr_high;
        port_nr = (port_nr << 8) | f->msg.hdr.tls_port_nr_low;
        if (port_nr && port_nrs[ipfamily][1] == 0) {
            port_nrs[ipfamily][1] = port_nr;
        }
    }

    for (i = 0; i < LIGHTHOUSE_NRO_ADDR_FAMILIES; i++)
    {
        ioc_lighthouse_add_endpoint(c, publish, "i", port_nrs[i][1],
            port_nrs[i][0], i == LIGHTHOUSE_IPV6);
    }
}


/**
****************************************************************************************************

  @brief Add information about an end point or end points.

  The ioc_lighthouse_add_endpoint() function stores information about the end points
  for UDP multicasts.

  @param   c Pointer to the light house server object structure.
  @param   publish List of network names to publish, or eobjects device name.
  @param   protocol "i" for IOCOM, "o" for eobjects.
  @param   tls_port TLS port number, 0 if unused.
  @param   tcp_port TCPS port number, 0 if unused.
  @param   is_ipv6 OS_TRUE to use IPv6 addressess or OS_FALSE for IPv4 addressess.
  @return  None.

****************************************************************************************************
*/
void ioc_lighthouse_add_endpoint(
    LighthouseServer *c,
    const os_char *publish,
    const os_char *protocol,
    os_int tls_port,
    os_int tcp_port,
    os_boolean is_ipv6)
{
    LighthouseServerOne *f;
    LighthouseAddressFamily ipfamily;
    os_char buf[LIGHTHOUSE_ITEM_SZ], nbuf[OSAL_NBUF_SZ];
    const os_char *e;
    os_memsz sz;
    os_ushort tls_port_nr, tcp_port_nr;

    if (tcp_port == 0 && tls_port == 0) {
        return;
    }

    ipfamily = is_ipv6 ? LIGHTHOUSE_IPV6 : LIGHTHOUSE_IPV4;
    ioc_lighthouse_try_set_default_ports(c, tls_port, IOC_TLS_SOCKET, ipfamily);
    ioc_lighthouse_try_set_default_ports(c, tcp_port, IOC_TCP_SOCKET, ipfamily);
    f = &c->f[ipfamily];

    while (*publish != '\0')
    {
        e = os_strchr(publish, ',');
        if (e == OS_NULL) {
            e = os_strchr(publish, '\0');
        }

        os_strncpy(buf, ",", sizeof(buf));
        if (tls_port) {
            os_strncat(buf, is_ipv6 ? "T" : "t", sizeof(buf));
            tls_port_nr = f->msg.hdr.tls_port_nr_high;
            tls_port_nr = (tls_port_nr << 8) | f->msg.hdr.tls_port_nr_low;
            if (tls_port != tls_port_nr) {
                osal_int_to_str(nbuf, sizeof(nbuf), tls_port);
                os_strncat(buf, nbuf, sizeof(buf));
            }
        }
        if (tcp_port) {
            os_strncat(buf, is_ipv6 ? "S" : "s", sizeof(buf));
            tcp_port_nr = f->msg.hdr.tcp_port_nr_high;
            tcp_port_nr = (tcp_port_nr << 8) | f->msg.hdr.tcp_port_nr_low;
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

        if (os_strncat(f->msg.publish, buf, LIGHTHOUSE_PUBLISH_SZ)) {
            osal_debug_error("lighthouse: \"publish\" buffer overflow");
        }

        if (*e == '\0') break;
        publish = e + 1;
    }
    f->msg.hdr.publish_sz = (os_uchar)os_strlen(f->msg.publish);
}


static void ioc_lighthouse_try_set_default_ports(
    LighthouseServer *c,
    os_int port,
    iocTransportEnum transport,
    LighthouseAddressFamily ipfamily)
{
    LighthouseServerOne *f;

    if (port == 0) return;
    f = &c->f[ipfamily];

    /* Default port must be not set
     */
    switch (transport) {
        case IOC_TLS_SOCKET:
            if (f->msg.hdr.tls_port_nr_high || f->msg.hdr.tls_port_nr_low) return;
            f->msg.hdr.tls_port_nr_low = (os_uchar)port;
            f->msg.hdr.tls_port_nr_high = (os_uchar)(port >> 8);
            break;

        case IOC_TCP_SOCKET:
            if (f->msg.hdr.tcp_port_nr_high || f->msg.hdr.tcp_port_nr_low) return;
            f->msg.hdr.tcp_port_nr_low = (os_uchar)port;
            f->msg.hdr.tcp_port_nr_high = (os_uchar)(port >> 8);
            break;

        default:
            return;
    }

    f->is_configured = OS_TRUE;
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
    LighthouseServerOne *f;
    os_timer tval;
    os_int i;
    osalStatus s4 = OSAL_SUCCESS, s6 = OSAL_SUCCESS;
    os_boolean inc_counter = OS_FALSE;

    if (ti == OS_NULL)
    {
        os_get_timer(&tval);
        ti = &tval;
    }

    for (i = 0; i < LIGHTHOUSE_NRO_ADDR_FAMILIES; i++)
    {
        f = &c->f[i];
        if (f->is_configured) {
            s4 = ioc_run_lighthouse_server_one(f, c->counter, ti);
            if (s4 == OSAL_SUCCESS) inc_counter = OS_TRUE;
        }
    }

    /* If multicast was sent using either protocol, increment multicast counter
     */
    if (inc_counter) {
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
    c->multicast_interval = c->multicast_interval_max;

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

