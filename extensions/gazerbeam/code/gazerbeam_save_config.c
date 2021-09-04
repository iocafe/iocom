/**

  @file    gazerbeam_save_config.c
  @brief   LED light communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.3.2020

  Save WiFi configuration to persistent storage.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "gazerbeam.h"

#if GAZERBEAM_PINS_SUPPORT
#define PINS_OS_INT_HANDLER_HDRS 1
#include "pins.h"
#endif


/**
****************************************************************************************************

  @brief Receive and process gazerbeam WiFi configuration.

  Call this function periodically to get and process Wifi configuration messages from Android
  phone flash light -> phototransistor. If message is received

  @param   gb Pointer to the GazerbeamReceiver structure.
  @param   flags Reserved for future, set GAZERBEAM_DEFAULT (0) for now.
  @return  None.

****************************************************************************************************
*/
void gazerbeam_run_configurator(
    GazerbeamReceiver *gb,
    os_short flags)
{
    os_char buf[GAZERBEAM_MAX_MSG_SZ];
    os_memsz buf_sz;
    osalStatus s;

    /* Get Wifi configuration messages from Android phone flash light -> phototransistor.
     */
    buf_sz = gazerbeam_get_message(gb, buf, sizeof(buf), GAZERBEAM_DEFAULT);
    if (buf_sz > 0)
    {
        /* Save stuff and perhaps reboot.
         */
        s = gazerbeam_save_config(buf, buf_sz);
        if (s == OSAL_SUCCESS || s == OSAL_NOTHING_TO_DO)
        {
            os_get_timer(&gb->configuration_match_timer);
            gb->configuration_match = OS_TRUE;

            /* Here we reboot only if something changed.
             */
            if (s == OSAL_SUCCESS) {
                osal_reboot(0);
            }
        }
    }
}


/**
****************************************************************************************************

  @brief Save wifi configuration from gazerbeam message to persistent storage.

  @param   message Received Gazebeam message.
  @param   message_sz Message size in bytes.
  @return  OSAL_SUCCESS if field was successfully set.
           OSAL_NOTHING_TO_DO if field was unchanged.
           OSAL_STATUS_FAILED if field was not set in message.

****************************************************************************************************
*/
osalStatus gazerbeam_save_config(
    os_char *message,
    os_memsz message_sz)
{
    osalNodeConfOverrides block;
    osalStatus s, sload, rval = OSAL_NOTHING_TO_DO;
#if OSAL_SUPPORT_WIFI_NETWORK_CONF
    os_char command[16];
#endif
    sload = os_load_persistent(OS_PBNR_NODE_CONF, (os_char*)&block, sizeof(block));

#if OSAL_SUPPORT_WIFI_NETWORK_CONF
    command[0] = '\0';
    s = gazerbeam_get_config_item(GAZERBEAM_ID_COMMAND,
        command, sizeof(command),
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) {
        if (!os_strcmp(command, "reset")) {
            os_persistent_delete(-1, OSAL_PERSISTENT_DELETE_ALL);
            os_memclear(&block, sizeof(block));
            if (sload == OSAL_SUCCESS) rval = s;
            return s;
        }
        else if (!os_strcmp(command, "reboot")) {
            rval = s;
        }
#if OSAL_SECRET_SUPPORT
        else if (!os_strcmp(command, "forget")) {
            osal_forget_secret();
            rval = s;
        }
#endif
    }

    s = gazerbeam_get_config_item(GAZERBEAM_ID_WIFI_NETWORK,
        block.wifi[0].wifi_net_name, OSAL_WIFI_PRM_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

    s = gazerbeam_get_config_item(GAZERBEAM_ID_WIFI_PASSWORD,
        block.wifi[0].wifi_net_password, OSAL_WIFI_PRM_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

#if OSAL_MAX_NRO_WIFI_NETWORKS > 1
    s = gazerbeam_get_config_item(GAZERBEAM_ID_WIFI2_NETWORK,
        block.wifi[1].wifi_net_name, OSAL_WIFI_PRM_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

    s = gazerbeam_get_config_item(GAZERBEAM_ID_WIFI2_PASSWORD,
        block.wifi[1].wifi_net_password, OSAL_WIFI_PRM_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;
#endif
#endif

    s = gazerbeam_get_config_item(GAZERBEAM_ID_NETWORK_NAME_OVERRIDE,
        block.network_name_override, OSAL_NETWORK_NAME_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

    s = gazerbeam_get_config_item(GAZERBEAM_ID_DEVICE_NR_OVERRIDE,
        block.device_nr_override, OSAL_DEVICE_NR_STR_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

    s = gazerbeam_get_config_item(GAZERBEAM_ID_CONNECT_IP_OVERRIDE,
        block.connect_to_override[0].parameters, OSAL_HOST_BUF_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

    if (rval == OSAL_SUCCESS) {
        os_save_persistent(OS_PBNR_NODE_CONF, (const os_char*)&block, sizeof(block), OS_FALSE);
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get one field from received gazerbeam message.

  The gazerbeam_get_config_item function gets value of a field specified by id from message
  received through gazerbeam.

  If the message doesn't contain requested field, field is left unmodified.
  Value "*" can be used to clear the field.

  @param   id Identifier of the field to get.
  @param   field Pointer to buffer where to store '\0' terminated field value.
  @param   field_sz Size if field value buffer in bytes.
  @param   message Received Gazebeam message.
  @param   message_sz Message size in bytes.
  @param   flags Reserved for future, set GAZERBEAM_DEFAULT (0) for now.
  @return  OSAL_SUCCESS if field was successfully set.
           OSAL_NOTHING_TO_DO if field was unchanged.
           OSAL_STATUS_FAILED if field was not set in message.

****************************************************************************************************
*/
osalStatus gazerbeam_get_config_item(
    gazerbeamFieldId id,
    os_char *field,
    os_memsz field_sz,
    os_char *message,
    os_memsz message_sz,
    os_short flags)
{
    os_uchar *p, *next_p, *e;
    os_uint sz;
    OSAL_UNUSED(flags);

    p = (os_uchar*)message;
    e = p + message_sz;
    while (p + 2 < e)
    {
        sz = p[1];
        next_p = p + sz + 2;
        if (p[0] == id)
        {
            if (sz >= field_sz) {
                sz = (os_uint)field_sz - 1;
            }

            if (p[2] == '*' && sz == 1) {
                sz = 0;
            }

            /* If unchanged?
             */
            if (sz)
            {
                if (!os_memcmp(field, p + 2, sz) && field[sz] == '\0') {
                    return OSAL_NOTHING_TO_DO;
                }

                os_memcpy(field, p + 2, sz);
            }
            else if (field[sz] == '\0') {
                return OSAL_NOTHING_TO_DO;
            }

            field[sz] = '\0';
            return OSAL_SUCCESS;
        }
        p = next_p;
    }

    return OSAL_STATUS_FAILED;
}
