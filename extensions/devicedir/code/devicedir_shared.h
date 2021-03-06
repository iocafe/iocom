/**

  @file    devicedir_conf.h
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
#pragma once
#ifndef DEVICEDIR_SHARED_H_
#define DEVICEDIR_SHARED_H_
#include "devicedir.h"

#if OSAL_CONTROL_CONSOLE_SUPPORT
    osalStatus io_device_console(iocRoot *root);
#else
    #define io_device_console(r) (OSAL_SUCCESS)
#endif

/* Flags for devicedir_memory_blocks
 */
#define IOC_DEVDIR_DEFAULT 0
#define IOC_DEVDIR_DATA 1
#define IOC_DEVDIR_BUFFERS 2

/* Flags for devicedir_overrides
*/
#define IOC_HELP_MODE 4

void devicedir_connections(
    iocRoot *root,
    osalStream list,
    os_short flags);

void devicedir_end_points(
    iocRoot *root,
    osalStream list,
    os_short flags);

osalStatus devicedir_overrides(
    osalStream list,
    os_short flags);

void devicedir_memory_blocks(
    iocRoot *root,
    osalStream list,
    const os_char *iopath,
    os_short flags);

void devicedir_list_mblks_target_buffers(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags);

void devicedir_list_mblks_source_buffers(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags);

void devicedir_append_mblk_binary(
    iocMemoryBlock *mblk,
    osalStream list,
    os_short flags);

#if OSAL_JSON_TEXT_SUPPORT
osalStatus devicedir_static_mblk_to_json(
    iocMemoryBlock *mblk,
    osalStream list);
#endif

void devicedir_append_flag(
    osalStream list,
    const os_char *flag_name,
    os_boolean *is_first);

#define DEVICEDIR_FIRST 1
#define DEVICEDIR_CONTINUES 0
#define DEVICEDIR_NEW_LINE 2
#define DEVICEDIR_TAB 4

void devicedir_append_str_param(
    osalStream list,
    const os_char *param_name,
    const os_char *str,
    os_short flags);

void devicedir_append_int_param(
    osalStream list,
    const os_char *param_name,
    os_int x,
    os_short flags);

void devicedir_dynamic_signals(
    iocRoot *root,
    osalStream list,
    const os_char *iopath,
    os_short flags);

void devicedir_info(
    iocRoot *root,
    osalStream list,
    os_short flags);

#endif
