/**

  @file    devicedir_conf.c
  @brief   List IO networks, devices, memory blocks and IO signals.
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


void devicedir_append_flag(
    osalStream list,
    const os_char *flag_name,
    os_boolean *is_first)
{
    if (!*is_first)
    {
        osal_stream_print_str(list, ",", 0);
    }
    *is_first = OS_FALSE;

    osal_stream_print_str(list, flag_name, 0);
}


static void devicedir_append_organize(
    osalStream list,
    os_short flags)
{
    if ((flags & DEVICEDIR_FIRST) == 0) {
        osal_stream_print_str(list, ", ", 0);
    }
    if (flags & DEVICEDIR_NEW_LINE)  {
        osal_stream_print_str(list, "\n", 0);
    }
    if (flags & DEVICEDIR_TAB) {
        osal_stream_print_str(list, "  ", 0);
    }
    osal_stream_print_str(list, "\"", 0);
}

void devicedir_append_str_param(
    osalStream list,
    const os_char *param_name,
    const os_char *str,
    os_short flags)
{
    devicedir_append_organize(list, flags);
    osal_stream_print_str(list, param_name, 0);
    osal_stream_print_str(list, "\":\"", 0);
    osal_stream_print_str(list, str, 0);
    osal_stream_print_str(list, "\"", 0);
}

void devicedir_append_int_param(
    osalStream list,
    const os_char *param_name,
    os_int x,
    os_short flags)
{
    os_char nbuf[OSAL_NBUF_SZ];

    devicedir_append_organize(list, flags);
    osal_stream_print_str(list, param_name, 0);
    osal_stream_print_str(list, "\":", 0);
    osal_int_to_str(nbuf, sizeof(nbuf), x);
    osal_stream_print_str(list, nbuf, 0);
}
