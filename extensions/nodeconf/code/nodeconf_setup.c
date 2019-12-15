/**

  @file    nodeconf_setup.c
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


/** Working state structure while parsing node configuration.
 */
typedef struct
{
    /** Pointer to node configuration beging set up.
     */
    iocNodeConf *node;

    /** Current network interface, wifi network and connection indices
     */
    os_int nic_ix;
    os_int wifi_ix;
    os_int connection_ix;

    /** Latest information parsed from JSON
        tag or key, '-' for array elements
     */
    const os_char *tag;
}
iocNconfParseState;


static osalStatus ioc_nconf_process_block(
    iocNconfParseState *state,
    os_char *array_tag,
    osalJsonIndex *jindex);

static void ioc_nconf_setup_structure(
    iocNodeConf *node,
    const os_char *config,
    os_memsz config_sz);


/**
****************************************************************************************************

  @brief Load network node configuration from persistent storage.

  The ioc_load_node_config() function loads node's network configuration from persistent storage.
  If loading network configuration fails, the default configuration given as argumen is used.

  @param   node Node (IO device) configuration to set up.
  @param   default_config Congifuration as packed JSON.
  @param   default_config_sz Default configuration size in bytes.
  @return  None.

****************************************************************************************************
*/
void ioc_load_node_config(
    iocNodeConf *node,
    const os_char *default_config,
    os_memsz default_config_sz)
{
    os_memclear(node, sizeof(iocNodeConf));

    ioc_nconf_setup_structure(node, default_config, default_config_sz);
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle arrays.

  The ioc_nconf_process_array() function is called to process array in packed JSON.

  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_nconf_process_array(
    iocNconfParseState *state,
    os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            return OSAL_STATUS_FAILED;
        }

        if (item.code == OSAL_JSON_END_ARRAY)
        {
            return OSAL_SUCCESS;
        }

        state->tag = item.tag_name;

        switch (item.code)
        {
            case OSAL_JSON_START_BLOCK:
                s = ioc_nconf_process_block(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                s = ioc_nconf_process_array(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
            case OSAL_JSON_VALUE_INTEGER:
            case OSAL_JSON_VALUE_FLOAT:
            case OSAL_JSON_VALUE_NULL:
            case OSAL_JSON_VALUE_TRUE:
            case OSAL_JSON_VALUE_FALSE:
                break;

            default:
                return OSAL_STATUS_FAILED;
        }
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle {} blocks.

  The ioc_ioc_nconf_process_block() function is called to process a block in packed JSON.

  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_nconf_process_block(
    iocNconfParseState *state,
    os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;
    osalNetworkInterface2 *nic;
    osalWifiNetworkConfig *wifi;
    iocOneConnectionConf *conn;
    os_boolean is_nic_block;
    os_boolean is_wifi_block;
    os_boolean is_connect_block;
    os_boolean is_security_block;
    os_char array_tag_buf[16];
    iocNodeConf *node;
    node = state->node;

    /* If this is beginning of signal block.
     */
    is_nic_block = OS_FALSE;
    is_wifi_block = OS_FALSE;
    is_connect_block = OS_FALSE;
    is_security_block = OS_FALSE;
    if (!os_strcmp(state->tag, "-"))
    {
        if (!os_strcmp(array_tag, "nic"))
        {
            is_nic_block = OS_TRUE;
            state->nic_ix++;
            state->wifi_ix = 0;
        }
        else if (!os_strcmp(array_tag, "wifi"))
        {
            is_wifi_block = OS_TRUE;
            state->wifi_ix++;
        }
        else if (!os_strcmp(array_tag, "connect"))
        {
            is_connect_block = OS_TRUE;
            state->connection_ix++;
        }
        else if (!os_strcmp(array_tag, "security"))
        {
            is_security_block = OS_TRUE;
        }
    }

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            return OSAL_SUCCESS;
        }

        if (item.code == OSAL_JSON_END_ARRAY)
        {
            return OSAL_STATUS_FAILED;
        }

        state->tag = item.tag_name;
        switch (item.code)
        {
            case OSAL_JSON_START_BLOCK:
                s = ioc_nconf_process_block(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                os_strncpy(array_tag_buf, state->tag, sizeof(array_tag_buf));
                s = ioc_nconf_process_array(state, array_tag_buf, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
                if (array_tag[0] == '\0')
                {
                    if (!os_strcmp(state->tag, "device_name"))
                    {
                        node->device_id.device_name = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "device_nr"))
                    {
                        node->device_id.device_nr = (os_int)osal_str_to_int(item.value.s, OS_NULL);
                    }
                    else if (!os_strcmp(state->tag, "network_name"))
                    {
                        node->device_id.network_name = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "user_name"))
                    {
                        node->device_id.user_name = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "password"))
                    {
                        node->device_id.password = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "cust1"))
                    {
                        node->device_id.cust1 = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "cust2"))
                    {
                        node->device_id.cust2 = item.value.s;
                    }
                }

                if (is_nic_block && state->nic_ix <= OSAL_MAX_NRO_NICS)
                {
                    nic = node->nic + state->nic_ix - 1;
                    if (!os_strcmp(state->tag, "name"))
                    {
                        nic->nic_name = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "ip"))
                    {
                        nic->ip_address = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "subnet"))
                    {
                        nic->subnet_mask = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "gateway"))
                    {
                        nic->gateway_address = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "dns"))
                    {
                        nic->dns_address = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "dns2"))
                    {
                        nic->dns_address_2 = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "mac"))
                    {
                        nic->mac = item.value.s;
                    }
                }

                if (is_wifi_block &&
                    state->nic_ix <= OSAL_MAX_NRO_NICS &&
                    state->wifi_ix <= OSAL_MAX_NRO_WIFI_NETWORKS)
                {
                    nic = node->nic + state->nic_ix - 1;
                    wifi = nic->wifinet + state->wifi_ix - 1;
                    if (!os_strcmp(state->tag, "network"))
                    {
                        wifi->wifi_net_name = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "password"))
                    {
                        wifi->wifi_net_password = item.value.s;
                    }
                }

                if (is_connect_block && state->connection_ix <= IOC_MAX_NCONF_CONNECTIONS)
                {
                    conn = node->connection + state->connection_ix - 1;
                    if (!os_strcmp(state->tag, "transport"))
                    {
                        if (!os_strcmp(item.value.s, "tls"))
                        {
                            conn->transport = IOC_TLS_SOCKET;
                        }
                        if (!os_strcmp(item.value.s, "socket"))
                        {
                            conn->transport = IOC_TCP_SOCKET;
                        }
                        else if (!os_strcmp(item.value.s, "serial"))
                        {
                            conn->transport = IOC_SERIAL_PORT;
                        }
                        else if (!os_strcmp(item.value.s, "bluetooth"))
                        {
                            conn->transport = IOC_BLUETOOTH;
                        }
                    }
                    else if (!os_strcmp(state->tag, "ip") ||
                        !os_strcmp(state->tag, "port"))
                    {
                        conn->parameters = item.value.s;
                    }
                }

                if (is_security_block)
                {
                    if (!os_strcmp(state->tag, "certfile"))
                    {
                        node->security_conf.certfile = item.value.s;
                    }
                }
                break;

            case OSAL_JSON_VALUE_INTEGER:
                if (array_tag[0] == '\0')
                {
                    if (!os_strcmp(state->tag, "device_nr"))
                    {
                        node->device_id.device_nr = (os_int)item.value.l;
                    }
                }

                if (is_nic_block && state->nic_ix <= OSAL_MAX_NRO_NICS)
                {
                    nic = node->nic + state->nic_ix - 1;
                    if (!os_strcmp(state->tag, "dhcp"))
                    {
                        nic->no_dhcp = (os_boolean)!item.value.l;
                    }
                }

                if (is_connect_block && state->connection_ix <= IOC_MAX_NCONF_CONNECTIONS)
                {
                    conn = node->connection + state->connection_ix - 1;
                    if (!os_strcmp(state->tag, "downward"))
                    {
                        conn->downward = (os_boolean)item.value.l;
                    }
                    else if (!os_strcmp(state->tag, "listen"))
                    {
                        conn->listen = (os_boolean)item.value.l;
                    }
                }
                break;

            case OSAL_JSON_VALUE_FLOAT:
            case OSAL_JSON_VALUE_NULL:
            case OSAL_JSON_VALUE_TRUE:
            case OSAL_JSON_VALUE_FALSE:
                break;

            default:
                return OSAL_STATUS_FAILED;
        }
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Set up node configuuration search structure (pointers).

  Call this function when device configuration has been loaded or default configuration is seleted
  for use. The function sets. Pointers to configuration data so that it is easy to use it
  from application.

  @param   node Pointer to node configuration in which to set the pointer.
  @param   config Pointer to packed JSON configuration loaded from persistent storage or
           to default configuration.
  @param   config_sz Configuration size in bytes.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static void ioc_nconf_setup_structure(
    iocNodeConf *node,
    const os_char *config,
    os_memsz config_sz)
{
    osalJsonIndex jindex;
    iocNconfParseState state;
    osalStatus s;
    os_int n_nics, n_connections;

    os_memclear(&state, sizeof(state));
    state.node = node;

    s = osal_create_json_indexer(&jindex, config, config_sz, 0);
    if (!s)
    {
        s = ioc_nconf_process_block(&state, "", &jindex);
    }

    n_nics = state.nic_ix;
    if (n_nics > OSAL_MAX_NRO_NICS) n_nics = OSAL_MAX_NRO_NICS;
    node->nics.nic = node->nic;
    node->nics.n_nics = n_nics;

    n_connections = state.connection_ix;
    if (n_connections > IOC_MAX_NCONF_CONNECTIONS) n_connections = IOC_MAX_NCONF_CONNECTIONS;
    node->connections.connection = node->connection;
    node->connections.n_connections = n_connections;

    if (s)
    {
        osal_debug_error_int("parsing node configuration failed:", s);
    }
}
