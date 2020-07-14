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


/** Enumeration of published network state items.
 */
typedef enum dinfoNodeConfSigEnum
{
    IOC_DINFO_NC_NR,
    IOC_DINFO_NC_NET,
    IOC_DINFO_NC_CONNECT,
    IOC_DINFO_NC_WIFI,
    IOC_DINFO_NC_PASS,
    IOC_DINFO_NC_IP,
    IOC_DINFO_NC_STATUS,
    IOC_DINFO_NRO_SIGNALS
}
dinfoNodeConfSigEnum;

/** Enumeration of network settings which can be overridden at run time.
 */
typedef enum dinfoNodeConfSetSigEnum
{
    IOC_DINFO_SET_NC_NR,
    IOC_DINFO_SET_NC_NET,
    IOC_DINFO_SET_NC_CONNECT,
    IOC_DINFO_SET_NC_WIFI,
    IOC_DINFO_SET_NC_PASS,
    IOC_DINFO_NRO_SET_SIGNALS
}
dinfoNodeConfSetSigEnum;

/** Structure holding network configuration signal pointers.
 */
typedef struct dinfoNodeConfSignals
{
    const iocSignal
        *sig[IOC_DINFO_NRO_SIGNALS],
        *set_sig[IOC_DINFO_NRO_SET_SIGNALS],
        *reboot,
        *factory_rst,
        *comloop;
}
dinfoNodeConfSignals;

/** Macro for easy set up of default network configuration signals.
 */
#define DINFO_SET_COMMON_NODE_CONF_SIGNALS(sigs, staticsigs)  \
    os_memclear(&sigs, sizeof(dinfoNodeConfSignals)); \
    sigs.sig[IOC_DINFO_NC_NR] = &staticsigs.exp.nc_nr; \
    sigs.sig[IOC_DINFO_NC_NET] = &staticsigs.exp.nc_net; \
    sigs.sig[IOC_DINFO_NC_CONNECT] = &staticsigs.exp.nc_connect; \
    sigs.sig[IOC_DINFO_NC_WIFI] = &staticsigs.exp.nc_wifi; \
    sigs.sig[IOC_DINFO_NC_PASS] = &staticsigs.exp.nc_pass; \
    sigs.sig[IOC_DINFO_NC_IP] = &staticsigs.exp.nc_ip; \
    sigs.sig[IOC_DINFO_NC_STATUS] = &staticsigs.exp.nc_status; \
    sigs.set_sig[IOC_DINFO_SET_NC_NR] = &staticsigs.imp.set_nc_nr; \
    sigs.set_sig[IOC_DINFO_SET_NC_NET] = &staticsigs.imp.set_nc_net; \
    sigs.set_sig[IOC_DINFO_SET_NC_CONNECT] = &staticsigs.imp.set_nc_connect; \
    sigs.set_sig[IOC_DINFO_SET_NC_WIFI] = &staticsigs.imp.set_nc_wifi; \
    sigs.set_sig[IOC_DINFO_SET_NC_PASS] = &staticsigs.imp.set_nc_pass; \
    sigs.reboot = &staticsigs.imp.reboot; \
    sigs.factory_rst = &staticsigs.imp.factory_rst;


/* Node configuration state
 */
typedef struct
{
    dinfoNodeConfSignals sigs;
}
dinfoNodeConf;


void dinfo_initialize_node_conf(
    dinfoNodeConf *dinfo_nc,
    dinfoNodeConfSignals *sigs);

void dinfo_run_node_conf(
    dinfoNodeConf *dinfo_nc,
    os_timer *ti);

void dinfo_node_conf_callback(
    dinfoNodeConf *dinfo_nc,
    iocSignal *sig);
