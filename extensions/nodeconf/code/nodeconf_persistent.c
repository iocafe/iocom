/**

  @file    nodeconf_persistent.c
  @brief   Data structures, defines and functions for managing network node configuration and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/nodeconf/nodeconf.h"


/**
****************************************************************************************************

  @brief Load node's network node configuration from persistent storage.

  The ioc_load_node_config()

  @return  None.

****************************************************************************************************
*/
void ioc_load_node_config(
    iocNodeConf *node,
    const os_char *default_config)
{
}

/* void ioc_save_node_config(
    iocNodeConf *node)
{
}
*/

/**
****************************************************************************************************

  @brief Release all memory allocated for node configuration structure.

  The ioc_release_node_config() function releases all memory allocated for
  IO node configuration structure.

  @param   node Pointer to node's network node configuration to release.
  @return  None.

****************************************************************************************************
*/
#if 0
void ioc_release_node_config(
    iocNodeConf *node)
{
#if OSAL_MULTITHREAD_SUPPORT
    osalMutex lock;
    lock = node->lock;
    osal_mutex_lock(lock);
#endif

//    nodeconf_release_string(&node->node_name);
//    nodeconf_release_string(&node->network_name);

    os_memclear(node, sizeof(iocNodeConf));

#if OSAL_MULTITHREAD_SUPPORT
    osal_mutex_unlock(lock);
    osal_mutex_delete(lock);
#endif
}
#endif
