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

    /** Received full user name to use in checking. in addition to  user.user_name this
        may include  also network name, if checking in device's root network accounts
        permissions for other IO networks.
     */
    const os_char *checked_user_name;

    /** Network name of accounts memory block.
     */
    const os_char *mblk_network_name;

    /** The authentication info was received from IP.
     */
    const os_char *recieved_ip;

    /** Pointer to allowed network list beging set up.
     */
    iocAllowedNetworkConf *allowed_networks;

    /** Pointers to data for current item in account info.
     */
    const os_char *user_name, *password, *privileges, *ip_start, *ip_end;

    /** Latest information parsed from JSON
        tag or key, '-' for array elements
     */
    const os_char *tag;

    /** Decision on user authorization check.
     */
    os_boolean valid_user;

    /** Cause for denied access.
     */
    iocNoteCode ncode;
}
iocAccountsParserState;


const os_char ioc_accounts_device_name[] = "accounts";
const os_char ioc_accounts_data_mblk_name[] = "data";

/* Prototypes for forward referred static functions.
 */
static osalStatus ioc_authorize_process_block(
    iocAccountsParserState *state,
    os_char *array_tag,
    osalJsonIndex *jindex);

static os_boolean ioc_check_whitelist(
    iocAccountsParserState *state);

static os_short ioc_compare_ip(
    os_uchar *a,
    os_uchar *b,
    os_int sz);

static void ioc_authorize_parse_accounts(
    iocAllowedNetworkConf *allowed_networks,
    os_boolean *is_valid_user,
    iocUser *user,
    os_char *ip,
    const os_char *user_name,
    const os_char *network_name,
    const os_char *config,
    os_memsz config_sz,
    void *context);



/**
****************************************************************************************************

  @brief Check if the user connecting is a legimate one.

  The ioc_authorize() function checks if user is authorized to connect to this server
  and has root privileges. The function fills list of networks which can be connected.

  @param   root Pointer to iocom root structure.
  @param   allowed_networks The function stores here list of networks the user is
           allowed to connect to.
  @param   user User account to check, received from the new connection.
  @param   ip From which IP address the connection came from.
  @param   contexe Pointer to extra implementation specific data, not used by this
           default implementation.
  @return  OSAL_SUCCESS if all is fine, value OSAL_STATUS_NO_ACCESS_RIGHT indicate
           that user is not authenticated to connect (interprent other return values
           as errors, these may be used in future).

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
    os_boolean is_valid_user = OS_FALSE;
    os_char *check_root_network = OS_NULL;
    os_char user_and_net[IOC_DEVICE_ID_SZ + IOC_NETWORK_NAME_SZ];
    os_short n_to_check = 1;

    /* User and network names are needed to check anything.
     */
    if (user->user_name[0] == '\0' ||
        user->network_name[0] == '\0')
    {
        osal_debug_error("Authorization check without user or network name");
        return OSAL_STATUS_FAILED;
    }

    /* Synchronize.
     */
    ioc_lock(root);

    /* Is the network for which accessed root network of this device, if not,
       check also root network.
     */
    if (os_strcmp(user->network_name, root->network_name))
    {
        check_root_network = root->network_name;
        n_to_check = 2;
    }

    os_strncpy(user_and_net, user->user_name, sizeof(user_and_net));
    os_strncat(user_and_net, ".", sizeof(user_and_net));
    os_strncat(user_and_net, user->network_name, sizeof(user_and_net));

    /* Find account data memory block for the IO network matching to the the connecting device.
     */
    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        if (mblk->device_name[0] != ioc_accounts_device_name[0]) continue;
        if (mblk->mblk_name[0] != ioc_accounts_data_mblk_name[0]) continue;
        if (os_strcmp(mblk->mblk_name, ioc_accounts_data_mblk_name)) continue;
        if (os_strcmp(mblk->device_name, ioc_accounts_device_name)) continue;

        /* Check if user is allowed to connect to the network specified in network name.
         */
        if (!os_strcmp(mblk->network_name, user->network_name))
        {
            ioc_authorize_parse_accounts(allowed_networks, &is_valid_user, user,
                ip, user->user_name, mblk->network_name, mblk->buf, mblk->nbytes, context);
            if (n_to_check-- <= 1) break;
        }

        /* Check if user is allowed to connect trough account in device's root network.
           This is needed for connecting controller in local net to cloud server.
         */
        else if (!os_strcmp(mblk->network_name, check_root_network))
        {
            ioc_authorize_parse_accounts(allowed_networks, &is_valid_user, user,
                ip, user_and_net, user->network_name, mblk->buf, mblk->nbytes, context);
            if (n_to_check-- <= 1) break;
        }
    }

#if OSAL_DEBUG
    if (!is_valid_user)
    {
        osal_debug_error_str("User not AUTHORIZED: ", user_and_net);
    }
#endif

    /* All done
     */
    ioc_unlock(root);
#if IOC_RELAX_SECURITY
    return OSAL_SUCCESS;
#else
    return is_valid_user ? OSAL_SUCCESS : OSAL_STATUS_NO_ACCESS_RIGHT;
#endif
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
            if (!os_strcmp(array_tag, "whitelist"))
            {
                if (!state->valid_user)
                {
                    if (ioc_check_whitelist(state))
                    {
                        state->valid_user = OS_TRUE;
                        ioc_add_allowed_network(state->allowed_networks, state->mblk_network_name, 0);
                    }
                }
            }
            else if (!os_strcmp(array_tag, "accounts"))
            {
                /* Find maching user name in user accounts.
                 */
                match = osal_pattern_match(state->checked_user_name, state->user_name, 0);

                /* Check password, unless '*' in user accounts accepts any passowrd.
                 */
                if (match)
                {
                    if (os_strcmp(state->password, osal_str_asterisk)) {
                        match = !os_strcmp(state->user->password, state->password);
                        state->ncode = IOC_NOTE_WRONG_IO_DEVICE_PASSWORD;
                    }
                }

                if (match)
                {
                    state->valid_user = OS_TRUE;
                    flags = 0;
                    if (!os_strcmp(state->privileges, "admin")) flags |= IOC_AUTH_ADMINISTRATOR;
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
                state->user_name = state->password = state->privileges = state->ip_start = state->ip_end = OS_NULL;
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
                else if (!os_strcmp(state->tag, "privileges"))
                {
                    state->privileges = item.value.s;
                }
                else if (!os_strcmp(state->tag, "password"))
                {
                    state->password = item.value.s;
                }
                else if (!os_strcmp(state->tag, "ip"))
                {
                    state->ip_start = item.value.s;
                }
                else if (!os_strcmp(state->tag, "last_ip"))
                {
                    state->ip_end = item.value.s;
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

  @brief Check if IP address from which authentication was received is white listed.

  White list defines address ranges from which devices can connect without being authenticated.
  This can be used for low security "in device" network, etc, to make configuration easier.

  @param   a First IP address. IP address is byte array, most significant byte first format highest.
  @param   b Second IP address
  @return  -1 if a < b, 0 if a == b, 1 if a > b.

****************************************************************************************************
*/
static os_boolean ioc_check_whitelist(
    iocAccountsParserState *state)
{
    os_uchar first_ip[16];
    os_uchar last_ip[16];
    os_uchar received_ip[16];
    osalStatus s;

    s = osal_ip_from_str(first_ip, sizeof(first_ip), state->ip_start);
    if (s != OSAL_SUCCESS && s != OSAL_IS_IPV6) return OS_FALSE;
    s = osal_ip_from_str(last_ip, sizeof(last_ip), state->ip_end);
    if (s != OSAL_SUCCESS && s != OSAL_IS_IPV6) return OS_FALSE;

    s = osal_ip_from_str(received_ip, sizeof(received_ip), state->recieved_ip);
    if (s != OSAL_SUCCESS && s != OSAL_IS_IPV6) return OS_FALSE;

    if (ioc_compare_ip(received_ip, first_ip, sizeof(received_ip)) < 0) return OS_FALSE;
    if (ioc_compare_ip(received_ip, last_ip, sizeof(received_ip)) > 0) return OS_FALSE;
    return OS_TRUE;
}


/**
****************************************************************************************************

  @brief Compare two IP addressess.

  The ioc_compare_ip() function compares two binary IP addressess. The IP addressess can be
  either IPv4 or IPv6 addressess, but both have to be in same format.

  @param   a First IP address. IP address is byte array, most significant byte first format highest.
  @param   b Second IP address
  @return  -1 if a < b, 0 if a == b, 1 if a > b.

****************************************************************************************************
*/
static os_short ioc_compare_ip(
    os_uchar *a,
    os_uchar *b,
    os_int sz)
{
    while (sz--)
    {
        if (*a < *b) return -1;
        if (*a > *b) return 1;
        a++;
        b++;
    }
    return 0;
}


/**
****************************************************************************************************

  @brief Check if user is authorized for specific IO device network.

  The ioc_authorize_parse_accounts() function checks if user is authorized to connect
  a specific IO device network.

  @param   root Pointer to iocom root structure.
  @param   allowed_networks The function stores here list of networks the user is
           allowed to connect to.
  @param   is_valid_user Pointer to boolean which is set if user is valid. If user is not valid
           value is not changed.
  @param   user User account to check, received from the new connection.
  @param   ip From which IP address the connection came from.
  @param   contexe Pointer to extra implementation specific data, not used by this
           default implementation.
  @param   config Pointer to user account information (packed JSON) for the IO network.
  @param   config_sz Account infomration size in bytes.

  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static void ioc_authorize_parse_accounts(
    iocAllowedNetworkConf *allowed_networks,
    os_boolean *is_valid_user,
    iocUser *user,
    os_char *ip,
    const os_char *user_name,
    const os_char *network_name,
    const os_char *config,
    os_memsz config_sz,
    void *context)
{
#if IOC_RELAX_SECURITY
    /* Security and testing is difficult with security on, IOC_RELAX_SECURITY define
     * can be used to turn it off.
     */
    *is_valid_user = OS_TRUE;
    return;
#else
    osalJsonIndex jindex;
    iocAccountsParserState state;
    iocSecurityNotification note;
    osalStatus s;

    os_memclear(&state, sizeof(state));
    state.user = user;
    state.recieved_ip = ip;
    state.checked_user_name = user_name;
    state.mblk_network_name = network_name;
    state.allowed_networks = allowed_networks;
    state.ncode = IOC_NOTE_NEW_IO_DEVICE;

    s = osal_create_json_indexer(&jindex, config, config_sz, 0); /* HERE WE SHOULD ALLOW ZERO PADDED DATA */
    if (s)
    {
        osal_debug_error_int("User account data is corrupted (A):", s);
    }
    else
    {
        s = ioc_authorize_process_block(&state, osal_str_empty, &jindex);
        if (s)
        {
            osal_debug_error_int("User account data is corrupted (B):", s);
        }
    }

    if (state.valid_user)
    {
        *is_valid_user = OS_TRUE;
    }

    /* Generate rest of notifications.
     */
    else if (context)
    {
        os_memclear(&note, sizeof(note));
        note.network_name = network_name;
        note.user = user_name;
        note.password = user->password;
        note.ip = ip;
        ioc_security_notify((iocBServer*)context, state.ncode, &note);
    }

#endif
}

