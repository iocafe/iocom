/**

  @file    nodeconf_data_struct.c
  @brief   Data structures, defines and functions for accessing network node configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "nodeconf.h"


/**
****************************************************************************************************
  Get device identification and custom parameters.
****************************************************************************************************
*/
iocDeviceId *ioc_get_device_id(
    iocNodeConf *node)
{
    return &node->device_id;
}

/**
****************************************************************************************************
  Get network interface configuration.
****************************************************************************************************
*/
iocNetworkInterfaces *ioc_get_nics(
    iocNodeConf *node)
{
    return &node->nics;
}

/**
****************************************************************************************************
  Get security configuration.
****************************************************************************************************
*/
osalSecurityConfig *ioc_get_security_conf(
    iocNodeConf *node)
{
    return &node->security_conf;
}

/**
****************************************************************************************************
  Get connection configuration.
****************************************************************************************************
*/
iocConnectionConfig *ioc_get_connection_conf(
    iocNodeConf *node)
{
    return &node->connections;
}
