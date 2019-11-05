/**

  @file    ionconf_conf.h
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
    IONCONF_TCP,
    IONCONF_TLS,
    IONCONF_SERIAL
}
ionconfFeatureEnum;


/** X
*/
void ionconf_initialize_node_configuration(
    ionconfNode *node);

void ionconf_release_node_configuration(
    ionconfNode *node);

#if OSAL_MULTITHREAD_SUPPORT
    #define ionconf_lock_node_configuration(n) osal_mutex_lock((n)->lock)
    #define ionconf_unlock_node_configuration(n) osal_mutex_unlock((n)->lock)
#else
    #define ionconf_lock_node_configuration(node)
    #define ionconf_unlock_node_configuration(node)
#endif

/* Set application name and version.
 */
void ionconf_set_application_name(
    ionconfNode *node,
    const os_char *app_name,
    const os_char *app_version);

/* Get network interface configuration from node's nconf data.
 */
void ionconf_get_nic_conf(
    ionconfNode *node,
    osalNetworkInterface *nic,
    os_int n_nics);

os_boolean ionconf_is_feature_used(
    ionconfNode *node,
    ionconfFeatureEnum feature);

void ionconf_set_node_name(
    ionconfNode *node,
    const os_char *node_name);

const os_char *ionconf_get_node_name(
    ionconfNode *node);

void ionconf_set_network_name(
    ionconfNode *node,
    const os_char *network_name);

os_char *ionconf_get_network_name(
    ionconfNode *node);

void ionconf_set_connection(
    ionconfNode *node,
    os_int connection_nr,
    os_int flags,
    const os_char *parameters);

os_int ionconf_get_connection(
    ionconfNode *node,
    os_int connection_nr,
    os_char *parameters,
    os_memsz parameters_sz);

void ionconf_set_key_pair(
    ionconfNode *node,
    os_char *private_key,
    os_char *public_key);

os_char *ionconf_get_private_key(
    ionconfNode *node);

os_char *ionconf_get_public_key(
    ionconfNode *node);

void ionconf_set_client_certificate(
    ionconfNode *node,
    os_char *client_cert);

os_char *ionconf_get_client_certificate(
    ionconfNode *node);

void ionconf_autohorize(
    ionconfNode *node,
    os_char *node_name,
    os_char *network_name);

os_boolean ionconf_is_authorized(
    ionconfNode *node,
    os_char *node_name,
    os_char *network_name,
    os_char *client_cert_signed_by);

