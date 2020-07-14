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

typedef struct
{
    iocSignal
        *nc_nr,
        *nc_net,
        *nc_connect,
        *nc_wifi,
        *nc_pass,
        *nc_ip,
        *nc_status,

        *set_nc_nr,
        *set_nc_net,
        *set_nc_connect,
        *set_nc_wifi,
        *set_nc_pass,
        *reboot,
        *factory_rst;
}
dinfoNodeConfSignals;


#define DINFO_SET_COMMON_NODE_CONF_SIGNALS(sigs, staticsigs) { \
    os_memclear(&sigs, sizeof(dinfoNodeConfSignals)); \
    sigs.nc_nr = staticsigs.exp.nc_nr; \
    sigs.nc_net = staticsigs.exp.nc_net;


typedef struct {
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
