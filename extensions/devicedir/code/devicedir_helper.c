/**

  @file    devicedir_conf.c
  @brief   List IO networks, devices, memory blocks and IO signals.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.11.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"


void devicedir_append_flag(
    osalStream list,
    os_char *flag_name,
    os_boolean *is_first)
{
    if (!*is_first)
    {
        osal_stream_print_str(list, ",", 0);
    }
    *is_first = OS_FALSE;

    osal_stream_print_str(list, flag_name, 0);
}


void devicedir_append_str_param(
    osalStream list,
    os_char *param_name,
    os_char *str,
    os_boolean is_first)
{
    osal_stream_print_str(list, is_first ? "\"" : ", \"", 0);
    osal_stream_print_str(list, param_name, 0);
    osal_stream_print_str(list, "\":\"", 0);
    osal_stream_print_str(list, str, 0);
    osal_stream_print_str(list, "\"", 0);
}

void devicedir_append_int_param(
    osalStream list,
    os_char *param_name,
    os_int x,
    os_boolean is_first)
{
    os_char nbuf[OSAL_NBUF_SZ];

    osal_int_to_str(nbuf, sizeof(nbuf), x);
    osal_stream_print_str(list, is_first ? "\"" :  ", \"", 0);
    osal_stream_print_str(list, param_name, 0);
    osal_stream_print_str(list, "\":", 0);
    osal_stream_print_str(list, nbuf, 0);
}
