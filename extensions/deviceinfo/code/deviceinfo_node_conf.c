/**

  @file    deviceinfo_node_conf.c
  @brief   Publish device's network configuration and state.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "deviceinfo.h"


static void dinfo_nc_net_state_notification_handler(
    struct osalNetworkState *net_state,
    void *context);


/**
****************************************************************************************************

  @brief Clear network configuration in device information and store IO signal pointers.

  Called at startup.

  @param   X
  @return  X

****************************************************************************************************
*/
void dinfo_initialize_node_conf(
    dinfoNodeConf *dinfo_nc,
    dinfoNodeConfSignals *sigs)
{
    os_memclear(dinfo_nc, sizeof(dinfoNodeConf));
    os_memcpy(&dinfo_nc->sigs, sigs, sizeof(dinfoNodeConfSignals));

}


/**
****************************************************************************************************

  @brief Set device information about network configuration.

  Called at startup after memory block "exp" has been created. Adds network state change
  notification handler to detect changes run time.

  @param   X
  @return  X

****************************************************************************************************
*/
void dinfo_set_node_conf(
    dinfoNodeConf *dinfo_nc,
    iocDeviceId *device_id,
    iocConnectionConfig *connconf,
    iocNetworkInterfaces *nics,
    osalWifiNetworks *wifis,
    osalSecurityConfig *security)
{
    iocMemoryBlock *mblk;
    os_int addr, mina, maxa, i;
    os_boolean dhcp;
    os_char nbuf[OSAL_NBUF_SZ];
    static OS_FLASH_MEM os_char one[] = "1", zero[] = "0";

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
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_NET], device_id->network_name);
        dinfo_nc->io_network_name_set = (os_boolean)(device_id->network_name[0] != '\0' &&
            os_strcmp(device_id->network_name, osal_str_asterisk));
    }

    /* Connections.
     */
    if (connconf)
    {
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_CONNECT], connconf->connection[0].parameters);
    }

    /* Wifi
     * WE COULD IFDEF OSAL_WIFI_SUPPORT, ETC
     */
    if (wifis) if (wifis->n_wifi >= 1)
    {
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_WIFI], wifis->wifi[0].wifi_net_name);
        ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_PASS], wifis->wifi[0].wifi_net_password[0] ? osal_str_asterisk : osal_str_empty);
        if (wifis->n_wifi >= 2) {
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_WIFI_2], wifis->wifi[1].wifi_net_name);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_PASS_2], wifis->wifi[1].wifi_net_password[0] ? osal_str_asterisk : osal_str_empty);
        }
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
        //ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_RECEIVE_UDP_MULTICASTS], nics->nic[0].receive_udp_multicasts ? one : zero);
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
            //ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_RECEIVE_UDP_MULTICASTS_2], nics->nic[1].receive_udp_multicasts ? one : zero);
            ioc_set_str(dinfo_nc->sigs.sig[IOC_DINFO_NC_MAC_2], nics->nic[1].mac);
        }
#endif
    }

    mina = 0x7FFFFFFF;
    maxa = -1;
    for (i = 0; i<IOC_DINFO_NRO_SET_SIGNALS; i++) {
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
  changes. Determines from network state if all is ok or something is wrong, and sets morse code
  accordingly.

  @param   net_state Network state structure.
  @param   context Morse code structure.
  @return  None.

****************************************************************************************************
*/
static void dinfo_nc_net_state_notification_handler(
    struct osalNetworkState *net_state,
    void *context)
{
    const iocSignal *sig;
    os_char buf[OSAL_IPADDR_SZ+1], *p;
    osalMorseCodeEnum code;
    dinfoNodeConf *dinfo_nc;
    dinfo_nc = (dinfoNodeConf*)context;

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

    /* Status code.
     */
    code = osal_network_state_to_morse_code(net_state);
    ioc_set(dinfo_nc->sigs.sig[IOC_DINFO_NC_STATUS], code);

}




void dinfo_node_conf_callback(
    dinfoNodeConf *dinfo_nc,
    const iocSignal *sig,
    os_int n_signals,
    os_ushort flags)
{
    if ((flags & IOC_MBLK_CALLBACK_RECEIVE) == 0) {
        return;
    }

    if (sig[0].addr > dinfo_nc->max_set_addr ||
        sig[n_signals - 1].addr < dinfo_nc->min_set_addr ||
        sig->handle->mblk != dinfo_nc->mblk)
    {
        return;
    }

}

