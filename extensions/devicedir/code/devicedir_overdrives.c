/**

  @file    devicedir_overdrives.c
  @brief   Get configuration overdrives for wifi, network name, etc.
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


static void devicedir_overdrive_prm(
    os_char *param_name,
    os_char *overdrive_value,
    osalStream list,
    os_short flags,
    os_boolean is_first)
{
    os_char *v;

    v = overdrive_value;
    if (v[0] == '\0') v = "*";

    if (flags & IOC_HELP_MODE)
    {
        if (!is_first) osal_stream_print_str(list, ",", 0);
        osal_stream_print_str(list, param_name, 0);
        osal_stream_print_str(list, "=", 0);
        osal_stream_print_str(list, v, 0);
    }
    else
    {
        devicedir_append_str_param(list, param_name, v,
            is_first
            ? DEVICEDIR_FIRST|DEVICEDIR_NEW_LINE|DEVICEDIR_TAB
            : DEVICEDIR_NEW_LINE|DEVICEDIR_TAB );
    }
}


/**
****************************************************************************************************

  @brief Get configuration overdrives for wifi, network name, etc.

****************************************************************************************************
*/
osalStatus devicedir_overdrives(
    iocRoot *root,
    osalStream list,
    os_short flags)
{
    osalWifiPersistent block;

    os_load_persistent(OS_PBNR_WIFI, (os_char*)&block, sizeof(block));

    if ((flags & IOC_HELP_MODE) == 0) {
        osal_stream_print_str(list, "{", 0);
    }

    devicedir_overdrive_prm("wifi", block.wifi[0].wifi_net_name, list, flags, OS_TRUE);
    devicedir_overdrive_prm("pass", block.wifi[0].wifi_net_password, list, flags, OS_FALSE);
    devicedir_overdrive_prm("net", block.network_name_overdrive, list, flags, OS_FALSE);
    devicedir_overdrive_prm("connect", block.connect_to_overdrive, list, flags, OS_FALSE);
    devicedir_overdrive_prm("nr", block.device_nr_overdrive, list, flags, OS_FALSE);

    if ((flags & IOC_HELP_MODE) == 0) {
        osal_stream_print_str(list, "\n}\n", 0);
    }

    return OSAL_SUCCESS;
}
