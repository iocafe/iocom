/**

  @file    lighthouse_client.c
  @brief   Service discovery using UDP multicasts (client).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The lighthouse client can be used by an IO device to detect controller in local area
  network. It monitors for UDP multicasts sent by server and collects network information.

  Server to connect to (IP address and port) is resolved by IO network name and transport.
  The library supports also auto selecting IO network, assuming the local net has only
  one IO network using the specified transport.

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

static void ioc_delete_expired_lighthouse_nets(
    LighthouseClient *c);

/**
****************************************************************************************************

  @brief Initialize the lighthouse client.

  The ioc_initialize_lighthouse_client() function initializes light house client structure.

  @param   c Pointer to the light house client object structure.
  @param   is_ipv6 Set OS_TRUE to look for IPv6 address. OS_FALSE for IPv4 address.
  @param   is_tls Set OS_TRUE for TLS connection. OS_FALSE for plain unsecure socket.
  @param   reserved Reserved for future, set OS_NULL.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_lighthouse_client(
    LighthouseClient *c,
    os_boolean is_ipv6,
    os_boolean is_tls,
    void *reserved)
{
    OSAL_UNUSED(reserved);
    os_memclear(c, sizeof(LighthouseClient));
    os_get_timer(&c->socket_error_timer);
    c->socket_error_timeout = 100;
    c->multicast_ip = is_ipv6 ? LIGHTHOUSE_IP_IPV6 : LIGHTHOUSE_IP_IPV4;
    c->select_tls = is_tls;
}


/**
****************************************************************************************************

  @brief Release resources allocated for lighthouse client.

  The ioc_release_lighthouse_client() function releases the resources allocated for lighthouse
  client. In practice the function closes the socket, which listens for UDP multicasts.
  The function doesn't release memory allocated for the client structure.

  @param   c Pointer to the light house client object structure.
  @return  None.

****************************************************************************************************
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


/* Set callback function when new connection is established.
 */
void ioc_set_lighthouse_client_callback(
    LighthouseClient *c,
    ioc_lighthouse_client_callback func,
    void *context)
{
    c->func = func;
    c->context = context;
}


/**
****************************************************************************************************

  @brief Check if lighthouse is to be used with this host name.

  The ioc_is_lighthouse_used() function checks if host name is asterisk or otherwise
  unspecified. If so lighthouse can be used and the function returns OS_TRUE. It also
  checks if wildcard specifies IPv6 address, like "[*]".

  @param   hostname Pointer to host name to check.
  @param   is_ipv6_wildcard Pointer to boolean to set to indicate if this is IPv6 wildcard.
           Can be OS_NULL if not needed.
  @return  OS_TRUE if light house should be used, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean ioc_is_lighthouse_used(
    const os_char *hostname,
    os_boolean *is_ipv6_wildcard)
{
    os_boolean lighthouse_on, ipv6;

    lighthouse_on = OS_FALSE;
    ipv6 = OS_FALSE;

    if (!os_strcmp(hostname, osal_str_asterisk) ||
        !os_strcmp(hostname, osal_str_empty))
    {
        lighthouse_on = OS_TRUE;
    }
    else if (!os_strcmp(hostname, "[*]"))
    {
        lighthouse_on = OS_TRUE;
        ipv6 = OS_TRUE;
    }

    if (is_ipv6_wildcard) *is_ipv6_wildcard = ipv6;
    return lighthouse_on;
}


/**
****************************************************************************************************

  @brief Keep lighthouse client functionality alive, poll for UDP multicasts.

  The ioc_run_lighthouse_client() function is called repeatedly to poll for received
  lighthouse UDP messages. The IO network information received is stores within
  the lighthouse client structure.

  @param   c Pointer to the light house client object structure.
  @param   trigger To return always immediately, give 0 here. If event is given here,
           the funcion waits for event once all messages have been successfully processed.
           When the wait breaks, the runction returns OSAL_SUCCESS. If this function returns
           OSAL_PENDING or error code, caller may want to call os_sleep() to save processor time.
  @return  OSAL_SUCCESS or OSAL_PENDING if all is fine. Latter indicates that we are waiting
           for next time to try to open a socket. Other values indicate a network error.

****************************************************************************************************
*/
osalStatus ioc_run_lighthouse_client(
    LighthouseClient *c,
    osalEvent trigger)
{
    osalStatus s;
    LighthouseMessage msg;
    os_char remote_addr[OSAL_IPADDR_SZ];
    os_memsz n_read, bytes, n, count;
    os_ushort checksum, port_nr, tls_port_nr, tcp_port_nr, counter;
    os_ushort my_tls_port_nr, my_tcp_port_nr;
    os_char network_item[IOC_NETWORK_NAME_SZ + IOC_NAME_SZ + 10], *p, *e;
    os_char *network_name, *protocol, nickname[IOC_NAME_SZ], *justincase, *q;
    os_timer received_timer;
    os_boolean is_tls;
#if OSAL_SOCKET_SELECT_SUPPORT
    osalStream streams[1];
#endif
    LightHouseClientCallbackData callbackdata;

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
        c->udp_socket = osal_stream_open(OSAL_SOCKET_IFACE,
            LIGHTHOUSE_PORT, (void*)c->multicast_ip, &s,
            OSAL_STREAM_MULTICAST|OSAL_STREAM_LISTEN|OSAL_STREAM_USE_GLOBAL_SETTINGS);
        if (c->udp_socket == OS_NULL)
        {
            osal_error(OSAL_ERROR, iocom_mod,
                OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);
            return s;
        }
        osal_error(OSAL_CLEAR_ERROR, iocom_mod,
            OSAL_STATUS_OPENING_UDP_SOCKET_FAILED, OS_NULL);

        os_get_timer(&c->multicast_received);
    }

    while (OS_TRUE)
    {
        /* Try to read multicast received from UDP stream
         */
        s = osal_stream_receive_packet(c->udp_socket, (os_char*)&msg, sizeof(msg), &n_read,
            remote_addr, sizeof(remote_addr), OSAL_STREAM_DEFAULT);
        if (OSAL_IS_ERROR(s))
        {
            osal_error(OSAL_ERROR, iocom_mod,
                OSAL_STATUS_RECEIVE_MULTICAST_FAILED, OS_NULL);

            osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
            c->udp_socket = OS_NULL;

            return s;
        }

        /*  Recoed that we recieived a multicast.
         */
        os_get_timer(&c->multicast_received);

        /* If success, but nothing received
         */
        if (n_read == 0)
        {
            /* Delete information about received networks which is exipired (internal).
             */
            if (c->check_expired_count++ > 17)
            {
                ioc_delete_expired_lighthouse_nets(c);
                c->check_expired_count = 0;
            }

            break;
        }

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
            osal_error(OSAL_WARNING, iocom_mod,
                OSAL_STATUS_UNKNOWN_LIGHTHOUSE_MULTICAST, "content");
            continue;
        }

        /* Verify checksum
         */
        checksum = msg.hdr.checksum_high;
        checksum = (checksum << 8) | msg.hdr.checksum_low;

        msg.hdr.checksum_high = msg.hdr.checksum_low = 0;
        if (checksum != os_checksum((const os_char*)&msg, bytes, OS_NULL))
        {
            osal_error(OSAL_WARNING, iocom_mod,
                OSAL_STATUS_UNKNOWN_LIGHTHOUSE_MULTICAST, "checksum");
            continue;
        }

        /* Get network ports and counter.
         */
        tls_port_nr = msg.hdr.tls_port_nr_high;
        tls_port_nr = (tls_port_nr << 8) | msg.hdr.tls_port_nr_low;
        tcp_port_nr = msg.hdr.tcp_port_nr_high;
        tcp_port_nr = (tcp_port_nr << 8) | msg.hdr.tcp_port_nr_low;
        counter = msg.hdr.counter_high;
        counter = (counter << 8) | msg.hdr.counter_low;

        /* Add/update device network or process name.
         */
        if (tls_port_nr || tcp_port_nr) {
            os_get_timer(&received_timer);

            p = os_strchr(msg.publish, ',');
            if (p == OS_NULL) {
                p = os_strchr(msg.publish, '\0');
            }
            n = p - msg.publish + 1;
            if (n > (os_memsz)sizeof(nickname)) {
                n = sizeof(nickname);
            }
            os_strncpy(nickname, msg.publish, n);
            p++;

            while (*p != '\0')
            {
                e = os_strchr(p, ',');
                if (e == OS_NULL) e = os_strchr(p, '\0');
                n = e - p + 1;
                if (n > (os_memsz)sizeof(network_item)) {
                    n = sizeof(network_item);
                }
                os_strncpy(network_item, p, n);

                protocol = os_strchr(network_item, ':');
                if (protocol == OS_NULL) goto goon;
                protocol++;

                network_name = os_strchr(protocol, ':');
                if (network_name == OS_NULL) goto goon;
                *(network_name++) = '\0';

                justincase = os_strchr(network_name, ':');
                if (justincase) {
                    *justincase = '\0';
                }

                my_tls_port_nr = 0;
                my_tcp_port_nr = 0;
                q = network_item;
                while (q + 1 < protocol) {
                    switch (*q) {
                        case 'T':
                        case 't':
                            is_tls = OS_TRUE;
                            break;

                        case 'S':
                        case 's':
                            is_tls = OS_FALSE;
                            break;

                        default:
                            osal_debug_error("Unknown lighthouse TLS/IPv6 mark");
                            goto goon;
                    }
                    q++;

                    port_nr = 0;
                    if (osal_char_isdigit(*q)) {
                        port_nr = (os_ushort)osal_str_to_int(q, &count);
                        q += count;
                    }

                    if (is_tls) {
                        my_tls_port_nr = port_nr ? port_nr : tls_port_nr;
                    }
                    else {
                        my_tcp_port_nr = port_nr ? port_nr : tcp_port_nr;
                    }
                }
                port_nr = c->select_tls ? my_tls_port_nr : my_tcp_port_nr;

                os_memclear(&callbackdata, sizeof(LightHouseClientCallbackData));
                callbackdata.ip_addr = remote_addr;
                callbackdata.protocol = protocol;
                callbackdata.tls_port_nr = my_tls_port_nr;
                callbackdata.tcp_port_nr = my_tcp_port_nr;
                callbackdata.network_name = network_name;
                callbackdata.nickname = nickname;
                callbackdata.counter = counter;

                if (c->func) {
                    c->func(c, &callbackdata, c->context);
                }

                if (port_nr && !os_strcmp(protocol, "i")) {
                    /* SWITCH THIS TO USE CALLBACKDATA and select_tls flag.
                     * And to use counter to select fastest of multiple IPs.
                     */
                    ioc_add_lighthouse_net(c, remote_addr, port_nr,
                        c->select_tls ? IOC_TLS_SOCKET : IOC_TCP_SOCKET,
                        network_name, &received_timer);
                }
goon:
                if (*e == '\0') break;
                p = e + 1;
            }
        }
    }

    /* If we have not received anything for 30 seconds, close socket to reopen it
     */
/*     if (c->lighthouse_really_needed)
    {
        if (os_has_elapsed(&c->multicast_received, 30000))
        {
            osal_debug_error("No multicasts for 30s");
            osal_stream_close(c->udp_socket, OSAL_STREAM_DEFAULT);
            c->udp_socket = OS_NULL;
        }
    }
*/

    if (trigger) {
#if OSAL_SOCKET_SELECT_SUPPORT
        streams[0] = c->udp_socket;
        if (osal_stream_select(streams, 1, trigger,
            OSAL_INFINITE, OSAL_STREAM_DEFAULT) == OSAL_STATUS_NOT_SUPPORTED)
        {
            return OSAL_STATUS_NOT_SUPPORTED;
        }
#else
        return OSAL_STATUS_NOT_SUPPORTED;
#endif
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Store information about an IO network to lighthouse client structure (internal).

  The ioc_add_lighthouse_net() function is called for each network when lighthouse UDP is
  received.

  @param   c Pointer to the light house client object structure.
  @param   ip_address Something like "192.168.1.220".
  @param   port_nr TCP port number, for example 6369.
  @param   transport Either IOC_TCP_SOCKET or IOC_TLS_SOCKET.
  @param   network_name Network name, like "cafenet".
  @param   received_timer os_get_timer() value to drop oldest information first when
           lighthouse client structure can hold no more information.
  @return  None.

****************************************************************************************************
*/
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
    os_boolean update_iface, is_right_net;

    os_lock();

    /* If we already have network with this name, update it.
     */
    selected_i = -1;
    update_iface = OS_FALSE;
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
        update_iface = OS_TRUE;
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
            if (!os_has_elapsed_since(&c->net[selected_i].received_timer,
                &c->net[i].received_timer, 1))
            {
                selected_i = i;
            }
        }
    }

    /* If we already got loopback interface and new received interface is
       something else, we prefer to keep the loopback unless it is very old (20 seconds).
     */
    n = c->net + selected_i;
    if (update_iface &&
        (!os_strcmp(n->ip_addr, "127.0.0.1") || !os_strcmp(n->ip_addr, "::1")) &&
         !os_strcmp(n->network_name, network_name) &&
         os_strcmp(ip_addr, "127.0.0.1") &&
         os_strcmp(ip_addr, "::1"))
    {
        if (!os_has_elapsed_since(&n->received_timer, received_timer, 10000)) {
            os_unlock();
            return;
        }
    }

    /* Save or update the network.
     */
    os_strncpy(n->ip_addr, ip_addr, OSAL_IPADDR_SZ);
    n->port_nr = port_nr;
    n->transport = transport;
    os_strncpy(n->network_name, network_name, OSAL_IPADDR_SZ);
    n->received_timer = *received_timer;

    /* Show lighthouse connect/no in network state
     */
    is_right_net = (os_boolean) !os_strcmp(network_name, c->network_name);
    os_unlock();
    if (is_right_net) {
        osal_set_network_state_int(OSAL_NS_LIGHTHOUSE_STATE, 0, OSAL_LIGHTHOUSE_OK);
    }
}


/**
****************************************************************************************************

  @brief Delete information about received networks which is exipired (internal).

  The ioc_delete_expired_lighthouse_nets() function...

  @param   c Pointer to the light house client object structure.
  @return  rNone.

****************************************************************************************************
*/
static void ioc_delete_expired_lighthouse_nets(
    LighthouseClient *c)
{
    OSAL_UNUSED(c);
#if 0
    os_timer ti;
    os_int i;

    os_get_timer(&ti);

    for (i = 0; i < LIGHTHOUSE_NRO_NETS; i++)
    {
        if (!c->net[i].transport) continue;

        /* 60s, shoud be linger than loopback expiration */
        if (os_has_elapsed_since(&c->net[i].received_timer, &ti, 60000))
        {
            c->net[i].transport = 0;
        }
    }
#endif
}


/**
****************************************************************************************************

  @brief Get server (controller) IP address and port IO network name and transport.

  The ioc_get_lighthouse_connectstr() function is typically called by ioc_connect.c trough
  function pointer (if lighthouse library is used). The function resolved

  @param   c Pointer to the light house client object structure.
  @param   func_nr Reserved for future, set LIGHTHOUSE_GET_CONNECT_STR for now.
  @param   network_name IO network name as input. If IO network name is empty string
           or equals to "*", any IO network name will match.
  @param   flags Flags as given to ioc_connect(), these define the transport: Bit flag
           IOC_SOCKET for both TLS and plain TCP socket,  IOC_SECURE_CONNECTION
           for TLS socket.
  @param   connectstr If successful, "connect to" string with IP address and port number is
           stored here, for example "192.168.1.220:6368".
  @param   Size of connectstr buffer in bytes. Should be at least OSAL_HOST_BUF_SZ characters.

  @return  If successful, the function returns either OSAL_SUCCESS or OSAL_IO_NETWORK_NAME_SET.
           The latter indicates that network_name was set by this function. The function
           return OSAL_STATUS_FAILED or other error code if unable to resolve IP address/port
           to connect to.

****************************************************************************************************
*/
osalStatus ioc_get_lighthouse_connectstr(
    LighthouseClient *c,
    LighthouseFuncNr func_nr,
    os_char *network_name,
    os_short flags,
    os_char *connectstr,
    os_memsz connectstr_sz)
{
    os_int i, selected_i;
    os_char nbuf[OSAL_NBUF_SZ];
    const os_char *compare_name;
    iocTransportEnum transport;
    os_boolean lighthouse_visible;
    OSAL_UNUSED(func_nr);

    /* If this is not socket (TCP or TLS, we can do nothing)
     * Set transport number, either IOC_TCP_SOCKET or IOC_TLS_SOCKET.
     */
    if ((flags & IOC_SOCKET) == 0) return OSAL_STATUS_FAILED;
    transport = (flags & IOC_SECURE_CONNECTION) ? IOC_TLS_SOCKET : IOC_TCP_SOCKET;

    os_lock();

    /* Mark that we are really using light house in this configuration.
     */
    c->lighthouse_really_needed = OS_TRUE;

    compare_name = network_name;
    if (!os_strcmp(compare_name, osal_str_asterisk)) {
        compare_name = osal_str_empty;
    }

    selected_i = -1;
    lighthouse_visible = OS_FALSE;
    for (i = 0; i < LIGHTHOUSE_NRO_NETS; i++)
    {
        /* Skip if transport doesn't match (skips also unused ones).
         */
        if (c->net[i].transport != transport) continue;

        lighthouse_visible = OS_TRUE;

        /* If network name doesn't match and we have network name, skip
         */
        if (*compare_name != '\0' &&  os_strcmp(compare_name, c->net[i].network_name)) continue;

        /* If this is older than some previous match, skip
         */
        if (selected_i >= 0) {
            if (!os_has_elapsed_since(&c->net[selected_i].received_timer, &c->net[i].received_timer, 1)) {
                continue;
            }
        }

        /* Select this line.
         */
        selected_i = i;
    }

    /* If we found no match?
     */
    if (selected_i < 0)
    {
        os_unlock();
        osal_set_network_state_int(OSAL_NS_LIGHTHOUSE_STATE, 0, lighthouse_visible
            ?  OSAL_NO_LIGHTHOUSE_FOR_THIS_IO_NETWORK : OSAL_LIGHTHOUSE_NOT_VISIBLE);
        return OSAL_STATUS_FAILED;
    }

    /* Set connect string
     */
    os_strncpy(connectstr, c->net[selected_i].ip_addr, connectstr_sz);
    osal_int_to_str(nbuf, sizeof(nbuf), c->net[selected_i].port_nr);
    os_strncat(connectstr, ":", connectstr_sz);
    os_strncat(connectstr, nbuf, connectstr_sz);

    /* Save the network name we are looking for to provide faster indication
     */
    os_strncpy(c->network_name, c->net[selected_i].network_name, IOC_NETWORK_NAME_SZ);

    os_unlock();
    osal_set_network_state_int(OSAL_NS_LIGHTHOUSE_STATE, 0, OSAL_LIGHTHOUSE_OK);
    return OSAL_SUCCESS;
}

