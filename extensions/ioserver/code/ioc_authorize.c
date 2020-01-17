/**

  @file    ioc_authorize.c
  @brief   User/device accounts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "ioserver.h"


/** Working state structure while parsing node configuration.
 */
typedef struct
{
    iocUser *user;

    /** Network name of accounts memory block.
     */
    os_char *mblk_network_name;

    /* Pointer to allowed network list beging set up.
     */
    iocAllowedNetworkConf *allowed_networks;

    /** Pointers to data for current item in account info.
     */
    const os_char *user_name, *password, *priviliges, *ip;

    /** Latest information parsed from JSON
        tag or key, '-' for array elements
     */
    const os_char *tag;

    /** Decision on user authotization check.
     */
    os_boolean valid_user;
}
iocAccountsParserState;


const os_char ioc_accounts_device_name[] = "accounts";
// const os_int ioc_accounts_device_nr = 1;
const os_char ioc_accounts_data_mblk_name[] = "data";

/* Prototypes for forward referred static functions.
 */
static osalStatus ioc_authorize_process_block(
    iocAccountsParserState *state,
    os_char *array_tag,
    osalJsonIndex *jindex);

static osalStatus ioc_authorize_parse_accounts(
    iocAllowedNetworkConf *allowed_networks,
    iocUser *user,
    os_char *ip,
    os_char *network_name,
    const os_char *config,
    os_memsz config_sz);



/**
****************************************************************************************************

  @brief Check if the user connecting is a legimate one.

  The ioc_authorize() function checks if user is authorized to connect to this server
  and has root priviliges. The function fills list of networks which can be connected.

  @param   root Pointer to iocom root structure.
  @param   allowed_networks The function stores here list of networks the user is
           allowed to connect to.
  @param   user User account to check, received from the new connection.
  @param   ip From which IP address the connection came from.
  @param   contexe Pointer to extra implementation specific data, not used by this
           default implementation.
  @return  OSAL_SUCCESS if all is fine, other values indicate that user is not authenticated
           to connect (or an error).

****************************************************************************************************
*/
osalStatus ioc_authorize(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUser *user,
    os_char *ip,
    void *context)
{
    iocMemoryBlock *mblk;
    osalStatus s;

    /* Synchronize.
     */
    ioc_lock(root);

    /* Find account data memory block for the IO network matching to the the connecting device.
     */
    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        if (mblk->device_name[0] != ioc_accounts_device_name[0] /* ||
            mblk->device_nr != ioc_accounts_device_nr */) continue;
        if (mblk->network_name[0] != user->network_name[0] ||
            mblk->mblk_name[0] != ioc_accounts_data_mblk_name[0]) continue;
        if (os_strcmp(mblk->mblk_name, ioc_accounts_data_mblk_name)) continue;
        if (os_strcmp(mblk->device_name, ioc_accounts_device_name)) continue;
        if (os_strcmp(mblk->network_name, user->network_name)) continue;

        break;
    }
    if (mblk == OS_NULL) return OSAL_STATUS_FAILED;

    /* Check if user is allowed to connect to the network.
     */
    s = ioc_authorize_parse_accounts(allowed_networks, user,
        ip, mblk->network_name, mblk->buf, mblk->nbytes);

    /* All done
     */
    ioc_unlock(root);
    return s;
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle arrays.

  The ioc_authorize_process_array() function is called to process array in packed JSON.

  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_authorize_process_array(
    iocAccountsParserState *state,
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
                s = ioc_authorize_process_block(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                s = ioc_authorize_process_array(state, array_tag, jindex);
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

  The ioc_ioc_authorize_process_block() function is called to process a block in packed JSON.

  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_authorize_process_block(
    iocAccountsParserState *state,
    os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;
    os_char array_tag_buf[16];
    os_ushort flags;
    os_boolean match;

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            match = osal_pattern_match(state->user->user_name, state->user_name, 0);

            if (match && os_strcmp(state->password, "*"))
            {
                match = !os_strcmp(state->user->password, state->password);
                if (!match)
                {
                    /* Password error */

                }
            }
            /* if (!os_strcmp(state->user_name, "*"))
            {
                match = OS_TRUE;
            }
            else
            {
                match = !os_strcmp(state->user->user_name, state->user_name);
            }
            if (match && os_strcmp(state->password, "*") && state->password)
            {
                match = !os_strcmp(state->user->password, state->password);
            } */

            /* Check IP range, etc spec: to be implemented
               if ((match || state->user_name == OS_NULL) && state->ip)
            {
                match == ioc_is_ip_within_spec(state->user->ip, state->ip)
            } */

            /* White list
             */
            if (match)
            {
                if (!os_strcmp(array_tag, "whitelist"))
                {

                }
                else if (!os_strcmp(array_tag, "blacklist"))
                {

                }
                else if (!os_strcmp(array_tag, "accounts"))
                {
                    state->valid_user = OS_TRUE;
                    flags = 0;
                    if (!os_strcmp(state->priviliges, "admin")) flags |= IOC_AUTH_ADMINISTRATOR;
                    ioc_add_allowed_network(state->allowed_networks, state->mblk_network_name, flags);
                }
            }
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
                state->user_name = state->password = state->priviliges = state->ip = OS_NULL;
                s = ioc_authorize_process_block(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                os_strncpy(array_tag_buf, state->tag, sizeof(array_tag_buf));
                s = ioc_authorize_process_array(state, array_tag_buf, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
                if (!os_strcmp(state->tag, "user"))
                {
                    state->user_name = item.value.s;
                }
                else if (!os_strcmp(state->tag, "priviliges"))
                {
                    state->priviliges = item.value.s;
                }
                else if (!os_strcmp(state->tag, "password"))
                {
                    state->password = item.value.s;
                }
                else if (!os_strcmp(state->tag, "ip"))
                {
                    state->ip = item.value.s;
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

  @brief Check if user is authorized for specific IO device network.

  The ioc_authorize_parse_accounts() function checks if user is authorized to connect
  a specific IO device network.

  @param   root Pointer to iocom root structure.
  @param   allowed_networks The function stores here list of networks the user is
           allowed to connect to.
  @param   user User account to check, received from the new connection.
  @param   ip From which IP address the connection came from.
  @param   contexe Pointer to extra implementation specific data, not used by this
           default implementation.
  @param   config Pointer to user account information (packed JSON) for the IO network.
  @param   config_sz Account infomration size in bytes.

  @return  OSAL_SUCCESS if all is fine, other values indicate that user is not authenticated
           to connect (or an error).

****************************************************************************************************
*/
static osalStatus ioc_authorize_parse_accounts(
    iocAllowedNetworkConf *allowed_networks,
    iocUser *user,
    os_char *ip,
    os_char *network_name,
    const os_char *config,
    os_memsz config_sz)
{
    osalJsonIndex jindex;
    iocAccountsParserState state;
    osalStatus s;

    os_memclear(&state, sizeof(state));
    state.user = user;
    state.ip = ip;
    state.mblk_network_name = network_name;
    state.allowed_networks = allowed_networks;

    s = osal_create_json_indexer(&jindex, config, config_sz, 0); /* HERE WE SHOULD ALLOW ZERO PADDED DATA */
    if (!s)
    {
        s = ioc_authorize_process_block(&state, "", &jindex);
    }
#if OSAL_DEBUG
    if (s)
    {
        osal_debug_error_int("parsing user accounts failed:", s);
    }
#endif
    if (s) return s;

    s = state.valid_user ? OSAL_SUCCESS : OSAL_STATUS_FAILED;

    /* Security and testing is difficult with security on, define to turn it off.
     */
#if IOC_RELAX_SECURITY
    s = OSAL_SUCCESS;
#endif
    return s;
}

