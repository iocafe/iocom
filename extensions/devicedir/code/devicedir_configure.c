/**

  @file    devicedir_configure.c
  @brief   Save configuration from console window line edit.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"
#if OSAL_CONTROL_CONSOLE_SUPPORT

/* Forward referred static functions.
 */
static osalStatus devicedir_get_config_item(
    const os_char *param_name,
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

****************************************************************************************************
*/
osalStatus devicedir_save_config(
    os_char *line_buf)
{
    osalNodeConfOverrides block;
    osalStatus s, rval = OSAL_NOTHING_TO_DO;

    os_load_persistent(OS_PBNR_NODE_CONF, (os_char*)&block, sizeof(block));

#if OSAL_SUPPORT_WIFI_NETWORK_CONF
    os_char buf[32], nbuf[OSAL_NBUF_SZ];
    os_int i;
    for (i = 0; i<OSAL_MAX_NRO_WIFI_NETWORKS; i++)
    {
        osal_int_to_str(nbuf, sizeof(nbuf), i+1);

        os_strncpy(buf, "wifi", sizeof(buf));
        if (i) os_strncat(buf, nbuf, sizeof(buf));
        s = devicedir_get_config_item(buf,
            block.wifi[i].wifi_net_name, OSAL_WIFI_PRM_SZ, line_buf);
        if (s == OSAL_SUCCESS) rval = s;

        os_strncpy(buf, "pass", sizeof(buf));
        if (i) os_strncat(buf, nbuf, sizeof(buf));
        s = devicedir_get_config_item(buf,
            block.wifi[i].wifi_net_password, OSAL_WIFI_PRM_SZ, line_buf);
        if (s == OSAL_SUCCESS) rval = s;
    }
#endif

    s = devicedir_get_config_item("net",
        block.network_name_override, OSAL_NETWORK_NAME_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    s = devicedir_get_config_item("nr",
        block.device_nr_override, OSAL_DEVICE_NR_STR_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    s = devicedir_get_config_item("connect",
        block.connect_to_override[0].parameters, OSAL_HOST_BUF_SZ, line_buf);
    if (s == OSAL_SUCCESS) rval = s;

    if (rval == OSAL_SUCCESS) {
        os_save_persistent(OS_PBNR_NODE_CONF, (const os_char*)&block, sizeof(block), OS_FALSE);
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get one parameter value from line edit buffer.

  The devicedir_get_config_item function gets a parameter value from line edit strings.
  If the line edit doesn't contain requested field, field is left unmodified and
  the function returns OSAL_NOTHING_TO_DO. Value "*" can be used to clear the field.

  @param   param_name Parameter name, like "wifi" or "pass".
  @param   field Pointer to buffer where to store '\0' terminated field value.
  @param   field_sz Size if field value buffer in bytes.
  @param   line_buf Line typed in by user from console.

  @return  OSAL_SUCCESS if field was succesfully set.
           OSAL_NOTHING_TO_DO if field was unchanged.

****************************************************************************************************
*/
static osalStatus devicedir_get_config_item(
    const os_char *param_name,
    os_char *field,
    os_memsz field_sz,
    os_char *line_buf)
{
    const os_char *value_ptr;
    os_memsz n_chars;

    /* Get pointer to value within line buffer by parameter name.
     */
    value_ptr = osal_str_get_item_value(line_buf, param_name, &n_chars, OSAL_STRING_DEFAULT);
    if (value_ptr == OS_NULL) {
        return OSAL_NOTHING_TO_DO;
    }

    /* Do not crash on too long values.
     */
    if (n_chars >= field_sz) {
        n_chars = field_sz - 1;
    }

    /* "*" can be used to clear the override
     */
    if (*value_ptr == '*' && n_chars == 1) {
        n_chars = 0;
    }

    /* If unchanged, return OSAL_NOTHING_TO_DO
     */
    if (n_chars) {
        if (!os_memcmp(field, value_ptr, n_chars) && field[n_chars] == '\0') {
            return OSAL_NOTHING_TO_DO;
        }

        os_memcpy(field, value_ptr, n_chars);
    }
    else if (field[n_chars] == '\0') {
        return OSAL_NOTHING_TO_DO;
    }

    /* Value changed, terminate with NULL charater and return OSAL_SUCCESS.
     */
    field[n_chars] = '\0';
    return OSAL_SUCCESS;
}


#endif

