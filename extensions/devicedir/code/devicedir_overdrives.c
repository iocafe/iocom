/**

  @file    devicedir_overrides.c
  @brief   Get configuration overrides for wifi, network name, etc.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    25.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"


static void devicedir_override_prm(
    const os_char *param_name,
    const os_char *override_value,
    osalStream list,
    os_short flags,
    os_boolean is_first)
{

    if (override_value[0] == '\0') {
        override_value = osal_str_asterisk;
    }

    if (flags & IOC_HELP_MODE)
    {
        if (!is_first) osal_stream_print_str(list, ",", 0);
        osal_stream_print_str(list, param_name, 0);
        osal_stream_print_str(list, "=", 0);
        osal_stream_print_str(list, override_value, 0);
    }
    else
    {
        devicedir_append_str_param(list, param_name, override_value,
            is_first
            ? DEVICEDIR_FIRST|DEVICEDIR_NEW_LINE|DEVICEDIR_TAB
            : DEVICEDIR_NEW_LINE|DEVICEDIR_TAB );
    }
}


/**
****************************************************************************************************

  @brief Get configuration overrides for wifi, network name, etc.

****************************************************************************************************
*/
osalStatus devicedir_overrides(
    iocRoot *root,
    osalStream list,
    os_short flags)
{
    osalWifiPersistent block;
    const os_char hidden_password[] = "<hidden>";

    os_load_persistent(OS_PBNR_WIFI, (os_char*)&block, sizeof(block));

    if ((flags & IOC_HELP_MODE) == 0) {
        osal_stream_print_str(list, "{", 0);
    }

#if OSAL_MAX_NRO_WIFI_NETWORKS > 0
    devicedir_override_prm("wifi", block.wifi[0].wifi_net_name, list, flags, OS_TRUE);
    devicedir_override_prm("pass", block.wifi[0].wifi_net_password[0]
        ? hidden_password : osal_str_empty, list, flags, OS_FALSE);

#if OSAL_MAX_NRO_WIFI_NETWORKS > 1
    os_char buf[32], nbuf[OSAL_NBUF_SZ];
    os_int i;
    if ((flags & IOC_HELP_MODE) == 0) {
        for (i = 1; i<OSAL_MAX_NRO_WIFI_NETWORKS; i++)
        {
            osal_int_to_str(nbuf, sizeof(nbuf), i+1);
            os_strncpy(buf, "wifi", sizeof(buf));
            os_strncat(buf, nbuf, sizeof(buf));
            devicedir_override_prm(buf, block.wifi[i].wifi_net_name, list, flags, OS_FALSE);
            os_strncpy(buf, "pass", sizeof(buf));
            os_strncat(buf, nbuf, sizeof(buf));
            devicedir_override_prm(buf, block.wifi[i].wifi_net_password[0]
                ? hidden_password : osal_str_empty, list, flags, OS_FALSE);
        }
    }
#endif
    devicedir_override_prm("net", block.network_name_override, list, flags, OS_FALSE);

#else
    devicedir_override_prm("net", block.network_name_override, list, flags, OS_TRUE);
#endif

    devicedir_override_prm("connect", block.connect_to_override, list, flags, OS_FALSE);
    devicedir_override_prm("nr", block.device_nr_override, list, flags, OS_FALSE);

    if ((flags & IOC_HELP_MODE) == 0) {
        osal_stream_print_str(list, "\n}\n", 0);
    }

    return OSAL_SUCCESS;
}
