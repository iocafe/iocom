/**

  @file    devicedir_conf.c
  @brief   List IO networks, devices, memory blocks and IO signals.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.11.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/devicedir/devicedir.h"



/**
****************************************************************************************************

  @brief Initialize node configuration structure.

  The devicedir_initialize_node_configuration() function initalizes devicedirNode structure
  and creates mutex to synchronize access to node configuration information.

  @param   node Pointer to node's network node configuration to initialize.
  @return  None.

****************************************************************************************************
*/
void devicedir_list(
    os_char *path,
    osalStream *device_list,
    os_short flags)
{
//    os_memclear(node, sizeof(devicedirNode));

#if OSAL_MULTITHREAD_SUPPORT
//     node->lock = osal_mutex_create();
#endif
}



void devicedir_list_end_points(
    os_char *path,
    osalStream *device_list,
    os_short flags)
{




}
