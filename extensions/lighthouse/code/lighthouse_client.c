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

/* Forward referred static functions.
 */
static void ioc_add_lighthouse_net(
    LighthouseClient *c,
    os_char *ip_addr,
    os_int port_nr,
    iocTransportEnum transport,
    os_char *network_name,
    os_timer *received_timer);


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
    LighthouseMessage msg;
    os_char remote_addr[OSAL_IPADDR_SZ];
    os_memsz n_read, bytes, n;
    os_ushort checksum, port_nr;
    os_char network_name[IOC_NETWORK_NAME_SZ], *p, *e;
    os_timer received_timer;

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
                OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
            return s;
        }
        osal_error(OSAL_CLEAR_ERROR, eosal_iocom,
            OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
    }

    /* Send packet to UDP stream
     */
    s = osal_stream_receive_packet(c->udp_socket, (os_char*)&msg, sizeof(msg), &n_read,
        remote_addr, sizeof(remote_addr), OSAL_STREAM_DEFAULT);
    if (OSAL_IS_ERROR(s))
    {
        osal_error(OSAL_ERROR, eosal_iocom,
            OSAL_STATUS_RECEIVING_UDP_PACKET_FAILED, OS_NULL);

        osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
        c->udp_socket = OS_NULL;
        return s;
    }

    /* If success, but nothing received
     */
    if (n_read == 0) return OSAL_SUCCESS;

    /* Make sure that string is terminated (just in case) and
       Validate the message id and size.
     */
    msg.publish[LIGHTHOUSE_PUBLISH_SZ-1] = '\0';
    bytes = sizeof(LighthouseMessageHdr) + msg.hdr.publish_sz;
    if (msg.hdr.msg_id != LIGHTHOUSE_MSG_ID ||
        msg.hdr.publish_sz < 1 ||
        msg.hdr.publish_sz > LIGHTHOUSE_PUBLISH_SZ ||
        msg.hdr.hdr_sz !=  sizeof(LighthouseMessageHdr) ||
        n_read < bytes)
    {
        osal_error(OSAL_WARNING, eosal_iocom, OSAL_UNKNOWN_LIGHTHOUSE_UDP_MULTICAST, "content");
        return OSAL_SUCCESS;
    }

    /* Verify checksum
     */
    checksum = msg.hdr.checksum_high;
    checksum = (checksum << 8) | msg.hdr.checksum_low;

    msg.hdr.checksum_high = msg.hdr.checksum_low = 0;
    if (checksum != os_checksum((const os_char*)&msg, bytes, OS_NULL))
    {
        osal_error(OSAL_WARNING, eosal_iocom, OSAL_UNKNOWN_LIGHTHOUSE_UDP_MULTICAST, "checksum");
        return OSAL_SUCCESS;
    }

    /* Add networks.
     */
    port_nr = msg.hdr.port_nr_high;
    port_nr = (port_nr << 8) | msg.hdr.port_nr_low;
    os_get_timer(&received_timer);
    p = msg.publish;
    while (*p != '\0')
    {
        e = os_strchr(p, ',');
        if (e == OS_NULL) e = os_strchr(p, '\0');
        n = e - p + 1;
        if (n > sizeof(network_name)) n = sizeof(network_name);
        os_strncpy(network_name, p, n);
        ioc_add_lighthouse_net(c, remote_addr, port_nr, msg.hdr.transport, network_name, &received_timer);
        if (*e == '\0') break;
        p = e + 1;
    }

    return OSAL_SUCCESS;
}

static void ioc_add_lighthouse_net(
    LighthouseClient *c,
    os_char *ip_addr,
    os_int port_nr,
    iocTransportEnum transport,
    os_char *network_name,
    os_timer *received_timer)
{
    LightHouseNetwork *n;
    os_int i, selected_i;

    /* If we already have network with this name, update it.
     */
    selected_i = -1;
    for (i = 0; i < LIGHTHOUSE_NRO_NETS; i++)
    {
        /* Skip if transport doesn't match (skips also unused ones).
         */
        if (c->net[i].transport != transport) continue;

        /* If network name doesn't match, skip.
         */
        if (os_strcmp(network_name, c->net[i].network_name)) continue;

        /* Select this line.
         */
        selected_i = i;
        break;
    }

    /* No matching network name, if we have empty one, use it.
     */
    if (selected_i < 0)
    {
        for (i = 0; i < LIGHTHOUSE_NRO_NETS; i++)
        {
            if (c->net[i].transport) continue;
            selected_i = i;
            break;
        }
    }

    /* No empty ones, pick up the oldest.
     */
    if (selected_i < 0)
    {
        selected_i = 0;
        for (i = 1; i < LIGHTHOUSE_NRO_NETS; i++)
        {
            if (!os_elapsed2(&c->net[selected_i].received_timer, &c->net[i].received_timer, 1))
            {
                selected_i = i;
            }
        }
    }

    /* Save the network
     */
    n = c->net + selected_i;
    os_strncpy(n->ip_addr, ip_addr, OSAL_IPADDR_SZ);
    n->port_nr = port_nr;
    n->transport = transport;
    os_strncpy(n->network_name, network_name, OSAL_IPADDR_SZ);
    n->received_timer = *received_timer;
}


/* Get server (controller) IP address and port by transport,
 * if received by UDP broadcast.
 *
   @param flags Flags as given to ioc_connect(): define IOC_SOCKET, IOC_SECURE_CONNECTION
   @return OSAL_IO_NETWORK_NAME_SET IO network name has been changed.
 */
osalStatus ioc_get_lighthouse_connectstr(
    LighthouseClient *c,
    LighthouseFuncNr func_nr,
    os_char *network_name,
    os_memsz network_name_sz,
    os_short flags,
    os_char *connectstr,
    os_memsz connectstr_sz)
{
    os_int i, selected_i;
    os_char nbuf[OSAL_NBUF_SZ], *compare_name;
    iocTransportEnum transport;

    /* If this is not socket (TCP or TLS, we can do nothing)
     * Set transport number, either IOC_TCP_SOCKET or IOC_TLS_SOCKET.
     */
    if ((flags & IOC_SOCKET) == 0) return OSAL_STATUS_FAILED;
    transport = (flags & IOC_SECURE_CONNECTION) ? IOC_TLS_SOCKET : IOC_TCP_SOCKET;

    compare_name = network_name;
    if (!os_strcmp(compare_name, "*")) compare_name = "";

    selected_i = -1;
    for (i = 0; i < LIGHTHOUSE_NRO_NETS; i++)
    {
        /* Skip if transport doesn't match (skips also unused ones).
         */
        if (c->net[i].transport != transport) continue;

        /* If network name doesn't match and we have network name, skip
         */
        if (*compare_name != '\0' &&  os_strcmp(compare_name, c->net[i].network_name)) continue;

        /* If this is older than some previous match, skip
         */
        if (selected_i >= 0) {
            if (!os_elapsed2(&c->net[selected_i].received_timer, &c->net[i].received_timer, 1)) {
                continue;
            }
        }

        /* Select this line.
         */
        selected_i = i;
    }

    /* If we found no match?
     */
    if (selected_i < 0) return OSAL_STATUS_FAILED;

    /* Set connect string
     */
    os_strncpy(connectstr, c->net[selected_i].ip_addr, connectstr_sz);
    osal_int_to_str(nbuf, sizeof(nbuf), c->net[selected_i].port_nr);
    os_strncat(connectstr, ":", connectstr_sz);
    os_strncat(connectstr, nbuf, connectstr_sz);

    /* If we do not have network name, set it
     */
    if (*compare_name == '\0') {
        os_strncpy(network_name, c->net[selected_i].network_name, network_name_sz);
        return OSAL_IO_NETWORK_NAME_SET;
    }

    return OSAL_SUCCESS;
}

