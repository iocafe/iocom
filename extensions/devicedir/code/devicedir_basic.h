/**

  @file    devicedir_conf.h
  @brief   List IO networks, devices, memory blocks and IO signals.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.11.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


void devicedir_connections(
    iocRoot *root,
    osalStream list,
    os_short flags);

void devicedir_memory_blocks(
    iocRoot *root,
    osalStream list,
    os_short flags);



void devicedir_append_flag(
    osalStream list,
    os_char *flag_name,
    os_boolean *is_first);

void devicedir_append_str_param(
    osalStream list,
    os_char *param_name,
    os_char *str,
    os_boolean is_first);

void devicedir_append_int_param(
    osalStream list,
    os_char *param_name,
    os_int x);
