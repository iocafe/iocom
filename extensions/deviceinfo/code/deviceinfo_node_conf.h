/**

  @file    deviceinfo_node_conf.h
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
#pragma once
#ifndef DEVICEINFO_NODE_CONF_H_
#define DEVICEINFO_NODE_CONF_H_
#include "deviceinfo.h"

/** Enumeration of published network state items.
 */
typedef enum dinfoNodeConfSigEnum
{
    IOC_DINFO_NC_NR,
    IOC_DINFO_NC_NET,
    IOC_DINFO_NC_CONNECT,
    IOC_DINFO_NC_CONNECT_2,
    IOC_DINFO_NC_WIFI,
    IOC_DINFO_NC_PASS,
    IOC_DINFO_NC_WIFI_2,
    IOC_DINFO_NC_PASS_2,
    IOC_DINFO_NC_DHCP,
    IOC_DINFO_NC_IP,
    IOC_DINFO_NC_SUBNET,
    IOC_DINFO_NC_GATEWAY,
    IOC_DINFO_NC_DNS,
    IOC_DINFO_NC_DNS2,
    IOC_DINFO_NC_SEND_UDP_MULTICASTS,
    IOC_DINFO_NC_MAC,
    IOC_DINFO_NC_DHCP_2,
    IOC_DINFO_NC_IP_2,
    IOC_DINFO_NC_SUBNET_2,
    IOC_DINFO_NC_GATEWAY_2,
    IOC_DINFO_NC_DNS_2,
    IOC_DINFO_NC_DNS2_2,
    IOC_DINFO_NC_SEND_UDP_MULTICASTS_2,
    IOC_DINFO_NC_MAC_2,
    IOC_DINFO_NC_STATUS,
    IOC_DINFO_NC_NRO_SIGNALS
}
dinfoNodeConfSigEnum;

/** Enumeration of network settings which can be overridden at run time.
 */
typedef enum dinfoNodeConfSetSigEnum
{
    IOC_DINFO_SET_NC_NR,
    IOC_DINFO_SET_NC_NET,
    IOC_DINFO_SET_NC_CONNECT,
    IOC_DINFO_SET_NC_CONNECT_2,
    IOC_DINFO_SET_NC_WIFI,
    IOC_DINFO_SET_NC_PASS,
    IOC_DINFO_SET_NC_WIFI_2,
    IOC_DINFO_SET_NC_PASS_2,

    IOC_DINFO_SET_NC_DHCP,
    IOC_DINFO_SET_NC_IP,
    IOC_DINFO_SET_NC_SUBNET,
    IOC_DINFO_SET_NC_GATEWAY,
    IOC_DINFO_SET_NC_DNS,
    IOC_DINFO_SET_NC_DNS2,
    IOC_DINFO_SET_NC_SEND_UDP_MULTICASTS,
    IOC_DINFO_SET_NC_MAC,
    IOC_DINFO_SET_NC_DHCP_2,
    IOC_DINFO_SET_NC_IP_2,
    IOC_DINFO_SET_NC_SUBNET_2,
    IOC_DINFO_SET_NC_GATEWAY_2,
    IOC_DINFO_SET_NC_DNS_2,
    IOC_DINFO_SET_NC_DNS2_2,
    IOC_DINFO_SET_NC_SEND_UDP_MULTICASTS_2,
    IOC_DINFO_SET_NC_REBOOT,
    IOC_DINFO_SET_NC_FORGET_IT,
    IOC_DINFO_SET_NC_FACTORY_RST,
    IOC_DINFO_SET_NC_COMLOOP,

    IOC_DINFO_NRO_SET_NC_SIGNALS
}
dinfoNodeConfSetSigEnum;

/** Structure holding network configuration signal pointers.
 */
typedef struct dinfoNodeConfSignals
{
    const iocSignal
        *sig[IOC_DINFO_NC_NRO_SIGNALS],
        *set_sig[IOC_DINFO_NRO_SET_NC_SIGNALS];
}
dinfoNodeConfSignals;

/** Macro for easy set up of default network configuration signals.
 */
#define DINFO_SET_COMMON_NET_CONF_SIGNALS_FOR_WIFI(sigs, staticsigs)  \
    os_memclear(&sigs, sizeof(dinfoNodeConfSignals)); \
    (sigs).sig[IOC_DINFO_NC_NR] = &(staticsigs).exp.nc_nr; \
    (sigs).sig[IOC_DINFO_NC_NET] = &(staticsigs).exp.nc_net; \
    (sigs).sig[IOC_DINFO_NC_CONNECT] = &(staticsigs).exp.nc_connect; \
    (sigs).sig[IOC_DINFO_NC_WIFI] = &(staticsigs).exp.nc_wifi; \
    (sigs).sig[IOC_DINFO_NC_PASS] = &(staticsigs).exp.nc_pass; \
    (sigs).sig[IOC_DINFO_NC_IP] = &(staticsigs).exp.nc_ip; \
    (sigs).sig[IOC_DINFO_NC_STATUS] = &(staticsigs).exp.nc_status; \
    (sigs).set_sig[IOC_DINFO_SET_NC_NR] = &(staticsigs).imp.set_nc_nr; \
    (sigs).set_sig[IOC_DINFO_SET_NC_NET] = &(staticsigs).imp.set_nc_net; \
    (sigs).set_sig[IOC_DINFO_SET_NC_CONNECT] = &(staticsigs).imp.set_nc_connect; \
    (sigs).set_sig[IOC_DINFO_SET_NC_WIFI] = &(staticsigs).imp.set_nc_wifi; \
    (sigs).set_sig[IOC_DINFO_SET_NC_PASS] = &(staticsigs).imp.set_nc_pass; \
    (sigs).set_sig[IOC_DINFO_SET_NC_REBOOT] = &(staticsigs).imp.reboot; \
    (sigs).set_sig[IOC_DINFO_SET_NC_FACTORY_RST] = &(staticsigs).imp.factory_rst;

/* Node configuration state
 */
typedef struct dinfoNodeConfState
{
    dinfoNodeConfSignals sigs;

    iocMemoryBlock *mblk;

    os_int
        min_set_addr,
        max_set_addr;

    os_boolean
        dhcp,                   /* DHCP used for first NIC */
        dhcp_2,                 /* DHCP used for second NIC */
        io_network_name_set,
        connect_to_set;

    os_boolean
        modified_common,
        reboot,
        forget_it,
        factory_reset;

    os_timer
        modified_timer;
}
dinfoNodeConfState;

/* Initialize node configuration state structure and store IO signal pointers.
 */
void dinfo_initialize_node_conf(
    dinfoNodeConfState *dinfo_nc,
    dinfoNodeConfSignals *sigs);

/* Set device information about network configuration.
 */
void dinfo_set_node_conf(
    dinfoNodeConfState *dinfo_nc,
    iocDeviceId *device_id,
    iocConnectionConfig *connconf,
    iocNetworkInterfaces *nics,
    iocWifiNetworks *wifis,
    osalSecurityConfig *security);

/* Check if we need to save or reboot. Can be called from application main loop.
 */
void dinfo_run_node_conf(
    dinfoNodeConfState *dinfo_nc,
    os_timer *ti);

/* Handle "set_*" signal changes.
 */
void dinfo_node_conf_callback(
    dinfoNodeConfState *dinfo_nc,
    const iocSignal *check_signals,
    os_int n_signals,
    os_ushort flags);

#endif
