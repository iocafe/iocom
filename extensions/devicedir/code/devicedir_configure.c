/**

  @file    devicedir_configure.c
  @brief   Save configuration from console window line edit.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"
#if OS_CONTROL_CONSOLE_SUPPORT

/* Forward referred static functions.
 */
static osalStatus devicedir_get_config_item(
    os_char *param_name,
    os_char *field,
    os_memsz field_sz,
    os_char *line_buf);


/**
****************************************************************************************************

  @brief Save wifi configuration from console to persistent storage.

  @param   message Received Gazebeam message.
  @param   message_sz Message size in bytes.
  @return  OSAL_SUCCESS if field was succesfully set.
           OSAL_NOTHING_TO_DO if field was unchanged.
           OSAL_STATUS_FAILED if field was not set in message.

****************************************************************************************************
*/
osalStatus devicedir_save_config(
    os_char *line_buf)
{
    osalWifiPersistent block;
    osalStatus s, rval = OSAL_NOTHING_TO_DO;

    os_load_persistent(OS_PBNR_WIFI, (os_char*)&block, sizeof(block));

    s = devicedir_get_config_item("wifi",
        block.wifi[0].wifi_net_name, OSAL_WIFI_PRM_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    s = devicedir_get_config_item("pass",
        block.wifi[0].wifi_net_password, OSAL_WIFI_PRM_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    s = devicedir_get_config_item("net",
        block.network_name_overdrive, OSAL_NETWORK_NAME_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    s = devicedir_get_config_item("nr",
        block.device_nr_overdrive, OSAL_DEVICE_NR_STR_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    s = devicedir_get_config_item("connect",
        block.connect_to_overdrive, OSAL_HOST_BUF_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    if (rval == OSAL_SUCCESS) {
        os_save_persistent(OS_PBNR_WIFI, (const os_char*)&block, sizeof(block), OS_FALSE);
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get one field from line edit.

  The devicedir_get_config_item function parameter value from line edit string.
  If the line edit doesn't contain requested field, field is left unmodified and
  the function returns OSAL_NOTHING_TO_DO.

  @param   param_name Parameter name, like "wifi" or "pass".
  @param   field Pointer to buffer where to store '\0' terminated field value.
  @param   field_sz Size if field value buffer in bytes.
  @param   line_buf Line typed in by user from console.

  @return  OSAL_SUCCESS if field was succesfully set.
           OSAL_NOTHING_TO_DO if field was unchanged.

****************************************************************************************************
*/
static osalStatus devicedir_get_config_item(
    os_char *param_name,
    os_char *field,
    os_memsz field_sz,
    os_char *line_buf)
{
    const os_char *value_ptr;
    os_memsz n_chars;

    value_ptr = osal_str_get_item_value(line_buf, param_name, &n_chars, OSAL_STRING_DEFAULT);
    if (value_ptr == OS_NULL) return OSAL_NOTHING_TO_DO;

    if (n_chars >= field_sz) n_chars = field_sz - 1;

    /* If unchanged?
     */
    if (n_chars)
    {
        if (!os_memcmp(field, value_ptr, n_chars) && field[n_chars] == '\0') {
            return OSAL_NOTHING_TO_DO;
        }

        os_memcpy(field, value_ptr, n_chars);
    }
    else if (field[n_chars] == '\0') {
        return OSAL_NOTHING_TO_DO;
    }

    field[n_chars] = '\0';
    return OSAL_SUCCESS;
}


#endif

