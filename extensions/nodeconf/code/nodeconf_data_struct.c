/**

  @file    nodeconf_data_struct.c
  @brief   Data structures, defines and functions for accessing network node configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.12.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/nodeconf/nodeconf.h"



/**
****************************************************************************************************

  @brief Get network interface configuration from node's nconf data.

  The ioc_get_nics() function fills in the network interface structure NIC by
  information in the nconf configuration.

  @param   node Pointer to node's network node configuration.
  @param   nic Pointer to array of network interface structure to fill in.
  @param   n_nics Number of network interfaces in nic array.
  @return  None.

****************************************************************************************************
*/




/* Get network interface configuration.
 */
iocNetworkInterfaces *ioc_get_nics(
    iocNodeConf *node)
{
    return &node->nics;
}

/* Get network interface configuration.
 */
osalSecurityConfig *ioc_get_security_conf(
    iocNodeConf *node)
{
    return &node->security_conf;
}

/* Get connection configuration.
 */
iocConnectPoints *ioc_get_connect_conf(
    iocNodeConf *node)
{
    return &node->cpoints;
}
