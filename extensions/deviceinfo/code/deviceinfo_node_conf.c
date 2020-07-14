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

}


void dinfo_node_conf_callback(
    dinfoNodeConf *dinfo_nc,
    iocSignal *sig)
{
    // if (sig->addr < min_addr || sig->addr > max_addr) return;


}

