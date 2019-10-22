/**

  @file    iotopology_conf.c
  @brief   Data structures, defines and functions for managing network topology and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iotopology/iotopology.h"


/* Prototyped for forward referred static functions.
 */
/* static void iotopology_set_string(
    os_char **pstr,
    const os_char *x);

static void iotopology_release_string(
    os_char **pstr);

*/


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


/**
****************************************************************************************************

  @brief Get network interface configuration from node's topology data.

  The iotopology_get_nic_conf() function fills in the network interface structure NIC by
  information in the topology configuration.

  @param   node Pointer to node's network topology configuration.
  @param   nic Pointer to array of network interface structure to fill in.
  @param   n_nics Number of network interfaces in nic array.
  @return  None.

****************************************************************************************************
*/
void iotopology_get_nic_conf(
    iotopologyNode *node,
    osalNetworkInterface *nic,
    os_int n_nics)
{
    iotopologyNIC *src;
    os_char *p;
    os_int i;

    os_memclear(&nic, n_nics * sizeof(osalNetworkInterface));

    if (n_nics > IOTOPOLOGY_MAX_NICS)
    {
        n_nics = IOTOPOLOGY_MAX_NICS;
    }

    src = node->config.nic;
    for (i = 0; i < n_nics; i++)
    {
        os_strncpy(nic->host_name, node->config.node_name, OSAL_IPADDR_SZ);
        os_strncpy(nic->ip_address, src->ip_address, OSAL_IPADDR_SZ);

        p = src->subnet_mask;
        if (*p == '\0') p = "255.255.255.0";
        os_strncpy(nic->subnet_mask, p, OSAL_IPADDR_SZ);

        os_strncpy(nic->gateway_address, src->gateway_address, OSAL_IPADDR_SZ);
        os_strncpy(nic->dns_address, src->dns_address, OSAL_IPADDR_SZ);
        os_strncpy(nic->mac, src->mac, OSAL_MAC_SZ);

        if (osal_string_get_item_value(src->options, "dhcp", OS_NULL, OSAL_STRING_DEFAULT))
        {
            nic->dhcp = OS_TRUE;
        }

        os_strncpy(nic->wifi_net_name, src->wifi_net_name, OSAL_WIFI_PRM_SZ);
        os_strncpy(nic->wifi_net_password, src->wifi_net_password, OSAL_WIFI_PRM_SZ);

        src++;
        nic++;
    }
}


os_boolean iotopology_is_feature_used(
    iotopologyNode *node,
    iotopologyFeatureEnum feature)
{
    return OS_TRUE; // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
}


void iotopology_set_node_name(
    iotopologyNode *node,
    const os_char *node_name)
{
    os_strncpy(node->config.node_name, node_name, IOTOPOLOGY_NODE_NAME_SZ);
}


const os_char *iotopology_get_node_name(
    iotopologyNode *node)
{
    return node->config.node_name;
}

void iotopology_set_network_name(
    iotopologyNode *node,
    const os_char *network_name)

{
    os_strncpy(node->config.network_name, network_name, IOTOPOLOGY_NETWORK_NAME_SZ);
}

os_char *iotopology_get_network_name(
    iotopologyNode *node)
{
    return node->config.network_name;
}


#if 0
/**
****************************************************************************************************

  @brief Store copy of string in newly allocated memory.

  The iotopology_set_string() strores copy of string x and sets pstr to point it.

  @param   pstr Pointer to string pointer used to hold copy. This must be initialized before
           calling this function either to OS_NULL or set by earlier iotopology_set_string()
           function call.
  @param   x Pointer to new string value. If OS_NULL, the pstr pointer is set to OS_NULL.
  @return  None.

****************************************************************************************************
*/
static void iotopology_set_string(
    os_char **pstr,
    const os_char *x)
{
    os_memsz sz;

    /* If we have old string value, release memory allocated for it.
     */
    iotopology_release_string(pstr);

    /* If we have non empty string argument x, allocate memory for it and store a copy
       of string.
     */
    if (x) if (*x != '\0')
    {
        sz = os_strlen(x);

        *pstr = os_malloc(sz, OS_NULL);
        os_memcpy(*pstr, x, sz);
    }
}


/**
****************************************************************************************************

  @brief Release memory allocated for string.

  The iotopology_release_string() releases memoty allocated to hold a string, if any.

  @param   pstr Pointer to string pointer used to hold copy. This must be initialized before
           calling this function either to OS_NULL or set by earlier iotopology_set_string()
           function call. At return pstr will be OS_NULL.
  @return  None.

****************************************************************************************************
*/
static void iotopology_release_string(
    os_char **pstr)
{
    if (*pstr)
    {
        os_free(*pstr, os_strlen(*pstr));
        *pstr = OS_NULL;
    }
}
#endif
