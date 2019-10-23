/**

  @file    io_py_connect.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iocompython/iocompython.h"


/* Prototyped for forward referred static functions.
 */


#if 0

/**
****************************************************************************************************

  @brief Initialize node configuration structure.

  The iotopology_initialize_node_configuration() function initalizes iotopologyNode structure
  and creates mutex to synchronize access to node configuration information.

  @param   node Pointer to node's network topology configuration to initialize.
  @return  None.

****************************************************************************************************
*/
void iotopology_initialize_node_configuration(
    iotopologyNode *node)
{
    os_memclear(node, sizeof(iotopologyNode));

#if OSAL_MULTITHREAD_SUPPORT
    node->lock = osal_mutex_create();
#endif
}


/**
****************************************************************************************************

  @brief Release all memory allocated for node configuration structure.

  The iotopology_release_node_configuration() function releases all memory allocated for
  IO node configuration structure.

  @param   node Pointer to node's network topology configuration to release.
  @return  None.

****************************************************************************************************
*/
void iotopology_release_node_configuration(
    iotopologyNode *node)
{
#if OSAL_MULTITHREAD_SUPPORT
    osalMutex lock;
    lock = node->lock;
    osal_mutex_lock(lock);
#endif

//    iotopology_release_string(&node->node_name);
//    iotopology_release_string(&node->network_name);

    os_memclear(node, sizeof(iotopologyNode));

#if OSAL_MULTITHREAD_SUPPORT
    osal_mutex_unlock(lock);
    osal_mutex_delete(lock);
#endif
}


/**
****************************************************************************************************

  @brief Set application name and version.

  The iotopology_set_application_name() function stores application name and version into node
  configuration. Application name and version are used to identify the software which the
  IO device or controller runs.

  @param   node Pointer to node's network topology configuration.
  @param   app_name Name of the application.
  @param   app_version Application version string.
  @return  None.

****************************************************************************************************
*/
void iotopology_set_application_name(
    iotopologyNode *node,
    const os_char *app_name,
    const os_char *app_version)
{
    os_strncpy(node->app_name, app_name, IOTOPOLOGY_APP_NAME_SZ);
    os_strncpy(node->app_version, app_version, IOTOPOLOGY_APP_VERSION_SZ);
}


#endif
