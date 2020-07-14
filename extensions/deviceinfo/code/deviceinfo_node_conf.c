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

  @brief X

  X

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
    osalMorseCodeEnum code;
    dinfoNodeConf *dinfo_nc;
    dinfo_nc = (dinfoNodeConf*)context;

    code = osal_network_state_to_morse_code(net_state);
}


void dinfo_node_conf_callback(
    dinfoNodeConf *dinfo_nc,
    iocSignal *sig)
{
    // if (sig->addr < min_addr || sig->addr > max_addr) return;


}

