/**

  @file    nodeconf_conf.h
  @brief   Data structures, defines and functions for managing network node configuration and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Enumeration of features to check.
 */
typedef enum
{
    NODECONF_TCP,
    NODECONF_TLS,
    NODECONF_SERIAL
}
nodeconfFeatureEnum;


/** X
*/
void nodeconf_initialize_node_configuration(
    nodeconfNode *node);

void nodeconf_release_node_configuration(
    nodeconfNode *node);

#if OSAL_MULTITHREAD_SUPPORT
    #define nodeconf_lock_node_configuration(n) osal_mutex_lock((n)->lock)
    #define nodeconf_unlock_node_configuration(n) osal_mutex_unlock((n)->lock)
#else
    #define nodeconf_lock_node_configuration(node)
    #define nodeconf_unlock_node_configuration(node)
#endif

/* Set application name and version.
 */
void nodeconf_set_application_name(
    nodeconfNode *node,
    const os_char *app_name,
    const os_char *app_version);

/* Get network interface configuration from node's nconf data.
 */
void nodeconf_get_nic_conf(
    nodeconfNode *node,
    osalNetworkInterface *nic,
    os_int n_nics);

os_boolean nodeconf_is_feature_used(
    nodeconfNode *node,
    nodeconfFeatureEnum feature);

void nodeconf_set_node_name(
    nodeconfNode *node,
    const os_char *node_name);

const os_char *nodeconf_get_node_name(
    nodeconfNode *node);

void nodeconf_set_network_name(
    nodeconfNode *node,
    const os_char *network_name);

os_char *nodeconf_get_network_name(
    nodeconfNode *node);

void nodeconf_set_connection(
    nodeconfNode *node,
    os_int connection_nr,
    os_int flags,
    const os_char *parameters);

os_int nodeconf_get_connection(
    nodeconfNode *node,
    os_int connection_nr,
    os_char *parameters,
    os_memsz parameters_sz);

void nodeconf_set_key_pair(
    nodeconfNode *node,
    os_char *private_key,
    os_char *public_key);

os_char *nodeconf_get_private_key(
    nodeconfNode *node);

os_char *nodeconf_get_public_key(
    nodeconfNode *node);

void nodeconf_set_client_certificate(
    nodeconfNode *node,
    os_char *client_cert);

os_char *nodeconf_get_client_certificate(
    nodeconfNode *node);

void nodeconf_autohorize(
    nodeconfNode *node,
    os_char *node_name,
    os_char *network_name);

os_boolean nodeconf_is_authorized(
    nodeconfNode *node,
    os_char *node_name,
    os_char *network_name,
    os_char *client_cert_signed_by);

