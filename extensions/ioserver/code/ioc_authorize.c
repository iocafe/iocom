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
    iocUserAuthorizationData *auth_data;
    iocUserAccount *user_account;

    /** Pointer to node configuration beging set up.
     */
    // iocNodeConf *node;

    /** Current network interface, wifi network and connection indices
     */
    // os_int nic_ix;
    // os_int wifi_ix;
    // os_int connection_ix;

    /** Latest information parsed from JSON
        tag or key, '-' for array elements
     */
    const os_char *tag;
}
iocAccountsParserState;


const os_char ioc_accounts_device_name[] = "accounts";
const os_int ioc_accounts_device_nr = 1;
const os_char ioc_accounts_data_mblk_name[] = "data";

/* Prototypes for forward referred static functions.
 */
static osalStatus ioc_authorize_process_block(
    iocAccountsParserState *state,
    os_char *array_tag,
    osalJsonIndex *jindex);

static osalStatus ioc_authorize_parse_accounts(
    iocUserAuthorizationData *auth_data,
    iocUserAccount *user_account,
    const os_char *config,
    os_memsz config_sz);



/**
****************************************************************************************************

  @brief Check if user is authorized.

  The ioc_authorize() function checks if user (as specified by user_account) is authorized to
  connect to this server and has root priviliges.

  @param   user_account User account to check. This is the received user information.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_authorize(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUserAccount *user_account,
    void *context)
{
    iocMemoryBlock *mblk;
    iocUserAuthorizationData auth_data;
    osalStatus s;

    ioc_lock(root);

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        if (mblk->device_name[0] != ioc_accounts_device_name[0] ||
            mblk->device_nr != ioc_accounts_device_nr) continue;
        if (mblk->network_name[0] != user_account->network_name[0] ||
            mblk->mblk_name[0] != ioc_accounts_data_mblk_name[0]) continue;
        if (os_strcmp(mblk->mblk_name, ioc_accounts_data_mblk_name)) continue;
        if (os_strcmp(mblk->device_name, ioc_accounts_device_name)) continue;
        if (os_strcmp(mblk->network_name, user_account->network_name)) continue;

        break;
    }
    if (mblk == OS_NULL) return OSAL_STATUS_FAILED;

    s = ioc_authorize_parse_accounts(&auth_data, user_account,
        mblk->buf, mblk->nbytes);

    ioc_unlock(root);
    return OSAL_SUCCESS;
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
    os_boolean is_nic_block;
    os_char array_tag_buf[16];

    /* If this is beginning of signal block.
     */
    is_nic_block = OS_FALSE;
    if (!os_strcmp(state->tag, "-"))
    {
        if (!os_strcmp(array_tag, "nic"))
        {
            is_nic_block = OS_TRUE;
//            state->nic_ix++;
//            state->wifi_ix = 0;
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
                s = ioc_authorize_process_block(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                os_strncpy(array_tag_buf, state->tag, sizeof(array_tag_buf));
                s = ioc_authorize_process_array(state, array_tag_buf, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
                if (array_tag[0] == '\0')
                {
                    if (!os_strcmp(state->tag, "device_name"))
                    {
                        // node->device_id.device_name = item.value.s;
                    }
                }

                //if (is_nic_block && state->nic_ix <= OSAL_MAX_NRO_NICS)
                {
                    // nic = node->nic + state->nic_ix - 1;
                    if (!os_strcmp(state->tag, "name"))
                    {
                        // nic->nic_name = item.value.s;
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

  @brief X.

  X.

  @param   auth_data Authorization data to be filled in by this function.
  @param   user_account User account to check.
  @param   config Pointer to user account information (packed JSON) for the IO network.
  @param   config_sz Account infomration size in bytes.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_authorize_parse_accounts(
    iocUserAuthorizationData *auth_data,
    iocUserAccount *user_account,
    const os_char *config,
    os_memsz config_sz)
{
    osalJsonIndex jindex;
    iocAccountsParserState state;
    osalStatus s;

    os_memclear(&state, sizeof(state));
    state.auth_data = auth_data;
    state.user_account = user_account;

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
    return s;
}

