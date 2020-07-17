/**

  @file    deviceinfo_node_conf.c
  @brief   Publish device's network configuration and state.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Display network information aboue the device, show network status changes or automatically
  determined IP address or IO network name. Allow user to modify network parameters trough
  IO signals.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "deviceinfo.h"
#include <stddef.h>


typedef struct dinfoSetSignalMapping
{
    os_char set_signal_nr;              /* Imported set value for signal. */
    os_char signal_nr;                  /* Exported signal value to display. */
    os_char net_state_item;             /* Net state item. */
    os_char net_state_index;            /* Nwt state index, like wifi nr, nic nr, connection number. */
    os_short offset;                    /* Position within persistent block */
    os_short sz;                        /* Negative values: size in persistent block, positive values: data type */
}
dinfoSetSignalMapping;

// #define IOC_DINFO_NOT 0x40

static OS_FLASH_MEM dinfoSetSignalMapping dinfo_sigmap[] = {
    {IOC_DINFO_SET_NC_NR, IOC_DINFO_NC_NR, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, device_nr_override), -OSAL_DEVICE_NR_STR_SZ},
    {IOC_DINFO_SET_NC_NET, IOC_DINFO_NC_NET, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, network_name_override), -OSAL_NETWORK_NAME_SZ},
    {IOC_DINFO_SET_NC_CONNECT, IOC_DINFO_NC_CONNECT, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, connect_to_override[0].parameters), -OSAL_HOST_BUF_SZ},
#if OSAL_NSTATE_MAX_CONNECTIONS > 1
    {IOC_DINFO_SET_NC_CONNECT_2, IOC_DINFO_NC_CONNECT_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, connect_to_override[1].parameters), -OSAL_HOST_BUF_SZ},
#endif

#if OSAL_SUPPORT_WIFI_NETWORK_CONF
    {IOC_DINFO_SET_NC_WIFI, IOC_DINFO_NC_WIFI, OSAL_NS_WIFI_NETWORK_NAME, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, wifi[0].wifi_net_name), -OSAL_WIFI_PRM_SZ},
    {IOC_DINFO_SET_NC_PASS, IOC_DINFO_NC_PASS, OSAL_NS_WIFI_PASSWORD, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, wifi[0].wifi_net_password), -OSAL_WIFI_PRM_SZ},
#if OSAL_MAX_NRO_WIFI_NETWORKS > 1
    {IOC_DINFO_SET_NC_WIFI_2, IOC_DINFO_NC_WIFI_2, OSAL_NS_WIFI_NETWORK_NAME, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, wifi[1].wifi_net_name), -OSAL_WIFI_PRM_SZ},
    {IOC_DINFO_SET_NC_PASS_2, IOC_DINFO_NC_PASS_2, OSAL_NS_WIFI_PASSWORD, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, wifi[1].wifi_net_password), -OSAL_WIFI_PRM_SZ},
#endif
#endif

#if OSAL_SUPPORT_STATIC_NETWORK_CONF
    {IOC_DINFO_SET_NC_DHCP, IOC_DINFO_NC_DHCP, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[0].dhcp), -OSAL_BOOL_STR_SZ},
    {IOC_DINFO_SET_NC_IP, IOC_DINFO_NC_IP, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[0].ip_address), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_SUBNET, IOC_DINFO_NC_SUBNET, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[0].subnet_mask), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_GATEWAY, IOC_DINFO_NC_GATEWAY, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[0].gateway_address), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_DNS, IOC_DINFO_NC_DNS, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[0].dns_address), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_DNS2, IOC_DINFO_NC_DNS2, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[0].dns_address_2), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_SEND_UDP_MULTICASTS, IOC_DINFO_NC_SEND_UDP_MULTICASTS, -1, 0, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[0].send_udp_multicasts), -OSAL_BOOL_STR_SZ},
#if OSAL_MAX_NRO_NICS > 1
    {IOC_DINFO_SET_NC_DHCP_2, IOC_DINFO_NC_DHCP_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[1].dhcp), -OSAL_BOOL_STR_SZ},
    {IOC_DINFO_SET_NC_IP_2, IOC_DINFO_NC_IP_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[1].ip_address), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_SUBNET_2, IOC_DINFO_NC_SUBNET_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[1].subnet_mask), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_GATEWAY_2, IOC_DINFO_NC_GATEWAY_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[1].gateway_address), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_DNS_2, IOC_DINFO_NC_DNS_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[1].dns_address), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_DNS2_2, IOC_DINFO_NC_DNS2_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[1].dns_address_2), -OSAL_IPADDR_SZ},
    {IOC_DINFO_SET_NC_SEND_UDP_MULTICASTS_2, IOC_DINFO_NC_SEND_UDP_MULTICASTS_2, -1, 1, (os_ushort)offsetof(struct osalNodeConfOverrides, nics[1].send_udp_multicasts), -OSAL_BOOL_STR_SZ},
#endif
#endif

    {-1, 0, 0, 0, 0, 0}
};


/* Forward referred static functions.
 */
static void dinfo_nc_net_state_notification_handler(
    struct osalNetworkState *net_state,
    void *context);


/**
****************************************************************************************************

  @brief Initialize and store IO signal pointers.

  Called at startup.

  @param   dinfo_nc Pointer to device info structure. This pointer is used as "handle".
  @return  sigs Structure containing signal pointers to set. Macros like
           DINFO_SET_COMMON_NET_CONF_SIGNALS_FOR_WIFI can be used to initialize typical
           signal pointers.

****************************************************************************************************
*/
void dinfo_initialize_node_conf(
    dinfoNodeConfState *dinfo_nc,
    dinfoNodeConfSignals *sigs)
{
    os_memclear(dinfo_nc, sizeof(dinfoNodeConfState));
    os_memcpy(&dinfo_nc->sigs, sigs, sizeof(dinfoNodeConfSignals));
}


/**
****************************************************************************************************

  @brief Set device information about network configuration.

  Called at startup after memory block "exp" has been created. Adds network state change
  notification handler to detect changes run time. The nodeconf library provides pointers
  to initialized decice_id, connconf, nics... structure.

  @param   dinfo_nc Pointer to device info structure. This pointer is used as "handle".
  @param   device_id Device identification structure.
  @param   connconf Connection configuration structure.
  @param   nics Information about network interface "cards".
  @param   wifi WiFi configuration information.
  @param   security Security information.
  @return  None.

****************************************************************************************************
*/
void dinfo_set_node_conf(
    dinfoNodeConfState *dinfo_nc,
    iocDeviceId *device_id,
    iocConnectionConfig *connconf,
    iocNetworkInterfaces *nics,
    iocWifiNetworks *wifis,
    osalSecurityConfig *security)
{
    iocMemoryBlock *mblk;
    const os_char *p;
    os_int addr, mina, maxa, i;
    os_boolean dhcp;
    os_char nbuf[OSAL_NBUF_SZ];
    static OS_FLASH_MEM os_char one[] = "1", zero[] = "0";
    OSAL_UNUSED(security);

    /* Device identification.
     */
    if (device_id)
    {
        if (device_id->device_nr == IOC_AUTO_DEVICE_NR) {
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_NR], osal_str_asterisk);
        }
        else {
            osal_int_to_str(nbuf, sizeof(nbuf), device_id->device_nr);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_NR], nbuf);
        }
        p = device_id->network_name;
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_NET], p);
        dinfo_nc->io_network_name_set = (os_boolean)(p[0] != '\0' && os_strcmp(p, osal_str_asterisk));
    }

    /* Connections.
     */
    if (connconf)
    {
        p = connconf->connection[0].parameters;
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_CONNECT], p);

        dinfo_nc->connect_to_set = (os_boolean)(p[0] != '\0' && os_strcmp(p, osal_str_asterisk));
    }

    /* Wifi
     * WE COULD IFDEF OSAL_WIFI_SUPPORT, ETC
     */
    if (wifis) if (wifis->n_wifi >= 1)
    {
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_WIFI], wifis->wifi[0].wifi_net_name);
        p = wifis->wifi[0].wifi_net_password;
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_PASS], (p[0] == '\0' || !os_strcmp(p, osal_str_asterisk))
                ? "not set" : "hidden");

#if OSAL_MAX_NRO_WIFI_NETWORKS > 1
        if (wifis->n_wifi >= 2) {
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_WIFI_2], wifis->wifi[1].wifi_net_name);
            p = wifis->wifi[1].wifi_net_password;
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_PASS_2], (p[0] == '\0' || !os_strcmp(p, osal_str_asterisk))
                    ? "not set" : "hidden");
        }
#endif
    }

    /* NICs
     */
    if (nics) if (nics->n_nics >= 1)
    {
        dhcp = !nics->nic[0].no_dhcp;
        dinfo_nc->dhcp = dhcp;
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_DHCP], dhcp ? one : zero);
        if (!dhcp) {
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_IP], nics->nic[0].ip_address);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_SUBNET], nics->nic[0].subnet_mask);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_GATEWAY], nics->nic[0].gateway_address);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_DNS], nics->nic[0].dns_address);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_DNS2], nics->nic[0].dns_address_2);
        }
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_SEND_UDP_MULTICASTS], nics->nic[0].send_udp_multicasts  ? one : zero);
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_MAC], nics->nic[0].mac);

#if OSAL_MAX_NRO_NICS>1
        if (nics->n_nics >= 2)
        {
            dhcp = !nics->nic[1].no_dhcp;
            dinfo_nc->dhcp_2 = dhcp;
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_DHCP_2], dhcp ? one : zero);
            if (!dhcp) {
                ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_IP_2], nics->nic[1].ip_address);
                ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_SUBNET_2], nics->nic[1].subnet_mask);
                ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_GATEWAY_2], nics->nic[1].gateway_address);
                ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_DNS_2], nics->nic[1].dns_address);
                ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_DNS2_2], nics->nic[1].dns_address_2);
            }
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_SEND_UDP_MULTICASTS_2], nics->nic[1].send_udp_multicasts  ? one : zero);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_MAC_2], nics->nic[1].mac);
        }
#endif
    }

    mina = 0x7FFFFFFF;
    maxa = -1;
    for (i = 0; i<IOC_DINFO_NRO_SET_NC_SIGNALS; i++) {
        if (dinfo_nc->sigs.set_sig[i] == OS_NULL) {
            continue;
        }
        addr = dinfo_nc->sigs.set_sig[i]->addr;
        if (addr < mina) mina = addr;
        if (addr > maxa) maxa = addr;
        mblk = dinfo_nc->sigs.set_sig[i]->handle->mblk;
        if (mblk) {
            dinfo_nc->mblk = mblk;
        }
    }
    dinfo_nc->min_set_addr = mina;
    dinfo_nc->max_set_addr = maxa;


    osal_add_network_state_notification_handler(dinfo_nc_net_state_notification_handler, dinfo_nc, 0);
}


/**
****************************************************************************************************

  @brief Handle network state change notifications.
  @anchor dinfo_nc_net_state_notification_handler

  The dinfo_nc_net_state_notification_handler() function is callback function when network state
  changes. Shows the items automatically detected in device infomration.

  @param   net_state Network state structure.
  @param   context Pointer to network device info structure.
  @return  None.

****************************************************************************************************
*/
static void dinfo_nc_net_state_notification_handler(
    struct osalNetworkState *net_state,
    void *context)
{
    const iocSignal *sig;
    os_char buf[OSAL_IPADDR_AND_PORT_SZ+1], *p;
    osalMorseCodeEnum code;
    dinfoNodeConfState *dinfo_nc;
    dinfo_nc = (dinfoNodeConfState*)context;

    code = osal_network_state_to_morse_code(net_state);
    ioc_set(dinfo_nc->sigs.sig[IOC_DINFO_NC_STATUS], code);

    /* IP address of the first NIC.
     */
    if (dinfo_nc->dhcp) {
        p = net_state->nic_ip[0];
        sig = dinfo_nc->sigs.sig[IOC_DINFO_NC_IP];
        if (sig && *p) if (os_strcmp(p, osal_str_asterisk))
        {
            os_strncpy(buf, p, sizeof(buf));
            os_strncat(buf, osal_str_asterisk, sizeof(buf));
            ioc_set_str(sig, buf);
        }
    }

#if OSAL_MAX_NRO_NICS>1
    /* IP address of the seconf NIC.
     */
    if (dinfo_nc->dhcp_2) {
        p = net_state->nic_ip[1];
        sig = dinfo_nc->sigs.sig[IOC_DINFO_NC_IP_2];
        if (sig && *p) if (os_strcmp(p, osal_str_asterisk))
        {
            os_strncpy(buf, p, sizeof(buf));
            os_strncat(buf, osal_str_asterisk, sizeof(buf));
            ioc_set_str(sig, buf);
        }
    }
#endif

    /* IO network name.
     */
    if (!dinfo_nc->io_network_name_set) {
        sig = dinfo_nc->sigs.sig[IOC_DINFO_NC_NET];
        if (sig) {
            osal_get_network_state_str(OSAL_NS_IO_NETWORK_NAME, 0, buf, sizeof(buf));
            os_strncat(buf, osal_str_asterisk, sizeof(buf));
            ioc_set_str(sig, buf);
        }
    }

    /* Connect to by lighhouse.
     */
    if (!dinfo_nc->connect_to_set) {
        sig = dinfo_nc->sigs.sig[IOC_DINFO_NC_CONNECT];
        if (sig) {
            osal_get_network_state_str(OSAL_NS_LIGHTHOUSE_CONNECT_TO, 0, buf, sizeof(buf));
            os_strncat(buf, osal_str_asterisk, sizeof(buf));
            ioc_set_str(sig, buf);
        }
    }

    /* Status code.
     */
    code = osal_network_state_to_morse_code(net_state);
    ioc_set(dinfo_nc->sigs.sig[IOC_DINFO_NC_STATUS], code);
}


/**
****************************************************************************************************

  @brief Handle "set_*" signal changes.
  @anchor dinfo_node_conf_callback

  The dinfo_node_conf_callback() function is called by communication callback to process
  changes to "set_" signals to configure the IO device.

  @param   dinfo_nc Pointer to device info structure. This pointer is used as "handle".
  @param   check_signals Array of signals which may have changed.
  @param   n_signals Number of items in check_signals array.
  @param   flags Flags of communication callback forwarded.
  @return  None.

****************************************************************************************************
*/
void dinfo_node_conf_callback(
    dinfoNodeConfState *dinfo_nc,
    const iocSignal *check_signals,
    os_int n_signals,
    os_ushort flags)
{
    const iocSignal **sigs, **set_sigs, *ss, *ds;
    const dinfoSetSignalMapping *m;
    os_char buf[OSAL_HOST_BUF_SZ], buf2[OSAL_HOST_BUF_SZ], state_bits;
    os_int x;

    if ((flags & IOC_MBLK_CALLBACK_RECEIVE) == 0) {
        return;
    }

    if (check_signals[0].addr > dinfo_nc->max_set_addr ||
        check_signals[n_signals - 1].addr < dinfo_nc->min_set_addr ||
        check_signals->handle->mblk != dinfo_nc->mblk)
    {
        return;
    }

    sigs = dinfo_nc->sigs.sig;
    set_sigs = dinfo_nc->sigs.set_sig;

    for (m = dinfo_sigmap; m->set_signal_nr >= 0; m++)
    {
        ss = set_sigs[(int)m->set_signal_nr];
        if (ss == OS_NULL) continue;

        ds = sigs[(int)m->signal_nr];
        if (ds == OS_NULL) continue;
        if (m->sz < 0) {
            state_bits = ioc_get_str(ss, buf, sizeof(buf));
            ioc_get_str(ds, buf2, sizeof(buf2));
            if (!os_strcmp(buf, buf2)) { state_bits = 0; }
            os_strncat(buf, "^", sizeof(buf));
            if (!os_strcmp(buf, buf2)) {state_bits = 0; }
            else { ioc_set_str(ds, buf); }
        }
        else {
            x = (os_int)ioc_get_ext(ss, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
            if (state_bits & OSAL_STATE_CONNECTED) {
                ioc_set(ds, x);
            }
        }

        if (state_bits & OSAL_STATE_CONNECTED) {
            os_get_timer(&dinfo_nc->modified_timer);
            dinfo_nc->modified_common = OS_TRUE;
        }
    }

    ss = dinfo_nc->sigs.set_sig[IOC_DINFO_SET_NC_REBOOT];
    if (ss) {
        x = (os_int)ioc_get_ext(ss, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
        if (x && (state_bits & OSAL_STATE_CONNECTED)) {
            dinfo_nc->reboot = OS_TRUE;
            os_get_timer(&dinfo_nc->modified_timer);
        }
    }

    ss = dinfo_nc->sigs.set_sig[IOC_DINFO_SET_NC_FACTORY_RST];
    if (ss) {
        x = (os_int)ioc_get_ext(ss, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
        if (x && (state_bits & OSAL_STATE_CONNECTED)) {
            dinfo_nc->factory_reset = OS_TRUE;
            os_get_timer(&dinfo_nc->modified_timer);
        }
    }
}


/**
****************************************************************************************************

  @brief Check if we need to save or reboot
  @anchor dinfo_run_node_conf

  The dinfo_run_node_conf() function is called repeatedly to check if we need to save
  configuration changes to persistent storage or reboot.

  @param   dinfo_nc Pointer to device info structure. This pointer is used as "handle".
  @param   ti Current timer value, If OS_NULL timer is read by function call.
  @return  None.

****************************************************************************************************
*/
void dinfo_run_node_conf(
    dinfoNodeConfState *dinfo_nc,
    os_timer *ti)
{
    osalNodeConfOverrides block;
    const dinfoSetSignalMapping *m;
    const iocSignal **set_sigs, *ss;
    os_timer tmp_ti;
    os_char buf[OSAL_HOST_BUF_SZ], *p, state_bits;
    os_boolean save_now;

    if (!dinfo_nc->modified_common && !dinfo_nc->reboot && !dinfo_nc->factory_reset) return;

    if (ti == OS_NULL) {
        os_get_timer(&tmp_ti);
        ti = &tmp_ti;
    }

    if (!os_has_elapsed_since(&dinfo_nc->modified_timer, ti, 500)) {
        return;
    }

    if (dinfo_nc->modified_common)
    {
        dinfo_nc->modified_common = OS_FALSE;
        os_load_persistent(OS_PBNR_NODE_CONF, (os_char*)&block, sizeof(block));
        save_now = OS_FALSE;
        set_sigs = dinfo_nc->sigs.set_sig;

        for (m = dinfo_sigmap; m->set_signal_nr >= 0; m++)
        {
            ss = set_sigs[(int)m->set_signal_nr];
            if (ss == OS_NULL) continue;
            if (m->sz < 0) {
                p = ((os_char*)&block) + m->offset;
                os_memclear(buf, sizeof(buf));
                state_bits = ioc_get_str(ss, buf, sizeof(buf));
                if (state_bits & OSAL_STATE_CONNECTED) {
                    if (os_strcmp(buf, p)) {
                        os_memcpy(p, buf, -m->sz);
                        save_now = OS_TRUE;
                    }
                }
            }
            /* else {
                x = (os_int)ioc_get_ext(ss, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
                if (state_bits & OSAL_STATE_CONNECTED) {
                    switch (m->sz & OSAL_TYPEID_MASK) {
                       case OS_BOOLEAN: // store integer x as boolean in block...
                            *p = (os_char)((m->sz & IOC_DINFO_NOT) ? !x : x);
                            save_now = OS_TRUE;
                            break;

                        default:
                            break;
                    }
                }
            } */
        }

        if (save_now) {
            os_save_persistent(OS_PBNR_NODE_CONF, (const os_char*)&block, sizeof(block), OS_FALSE);
        }
    }

    if (dinfo_nc->factory_reset)
    {
        osal_forget_secret();
        osal_reboot(0);
    }

    if (dinfo_nc->reboot) {
        osal_reboot(0);
    }
}
