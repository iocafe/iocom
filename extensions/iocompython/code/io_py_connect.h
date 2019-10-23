/**

  @file    iotopology_conf.h
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

/* Enumeration of features to check.
 */
typedef enum
{
    IOTOPOLOGY_TCP,
    IOTOPOLOGY_TLS,
    IOTOPOLOGY_SERIAL
}
iotopologyFeatureEnum;


/** X
*/
void iotopology_initialize_node_configuration(
    iotopologyNode *node);

void iotopology_release_node_configuration(
    iotopologyNode *node);

#if OSAL_MULTITHREAD_SUPPORT
    #define iotopology_lock_node_configuration(n) osal_mutex_lock((n)->lock)
    #define iotopology_unlock_node_configuration(n) osal_mutex_unlock((n)->lock)
#else
    #define iotopology_lock_node_configuration(node)
    #define iotopology_unlock_node_configuration(node)
#endif

/* Set application name and version.
 */
void iotopology_set_application_name(
    iotopologyNode *node,
    const os_char *app_name,
    const os_char *app_version);

/* Get network interface configuration from node's topology data.
 */
void iotopology_get_nic_conf(
    iotopologyNode *node,
    osalNetworkInterface *nic,
    os_int n_nics);

os_boolean iotopology_is_feature_used(
    iotopologyNode *node,
    iotopologyFeatureEnum feature);

void iotopology_set_node_name(
    iotopologyNode *node,
    const os_char *node_name);

const os_char *iotopology_get_node_name(
    iotopologyNode *node);

void iotopology_set_network_name(
    iotopologyNode *node,
    const os_char *network_name);

os_char *iotopology_get_network_name(
    iotopologyNode *node);

void iotopology_set_connection(
    iotopologyNode *node,
    os_int connection_nr,
    os_int flags,
    const os_char *parameters);

os_int iotopology_get_connection(
    iotopologyNode *node,
    os_int connection_nr,
    os_char *parameters,
    os_memsz parameters_sz);

void iotopology_set_key_pair(
    iotopologyNode *node,
    os_char *private_key,
    os_char *public_key);

os_char *iotopology_get_private_key(
    iotopologyNode *node);

os_char *iotopology_get_public_key(
    iotopologyNode *node);

void iotopology_set_client_certificate(
    iotopologyNode *node,
    os_char *client_cert);

os_char *iotopology_get_client_certificate(
    iotopologyNode *node);

void iotopology_autohorize(
    iotopologyNode *node,
    os_char *node_name,
    os_char *network_name);

os_boolean iotopology_is_authorized(
    iotopologyNode *node,
    os_char *node_name,
    os_char *network_name,
    os_char *client_cert_signed_by);

