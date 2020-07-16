/**

  @file    devicedir_get_json.c
  @brief   Get network, etc, information as JSON text.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    24.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"


/**
****************************************************************************************************

  @brief Get selected information as JSON text.
  @anchor devicedir_get_json

  The devicedir_get_json function....

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write the list as JSON
  @param   select Selects what information to get, see ddSelectJSON enumeration.
  @param   iopath IO path to select memory block.
  @param   flags Information to display, bit fields: IOC_DEVDIR_DEFAULT, IOC_DEVDIR_DATA,
           IOC_DEVDIR_BUFFERS.
  @return  OSAL_SUCCESS if all good, other values indicate an error.

****************************************************************************************************
*/
osalStatus devicedir_get_json(
    iocRoot *root,
    osalStream list,
    ddSelectJSON select,
    const os_char *iopath,
    os_short flags,
    const os_char **plabel)
{
    const os_char *label = OS_NULL;
    os_memsz n;
    osalStatus s = OSAL_STATUS_FAILED;

    label = "devicedir_get_json: unknown JSON selected";

    switch (select)
    {
        case IO_DD_CONNECTIONS:
            label = "connections";
            devicedir_connections(root, list, flags);
            s = OSAL_SUCCESS;
            break;

        case IO_DD_END_POINTS:
            label = "end points";
#ifdef OSAL_SOCKET_SUPPORT
            devicedir_end_points(root, list, flags);
#endif
            s = OSAL_SUCCESS;
            break;

        case IO_DD_MEMORY_BLOCKS:
            label = "memory blocks";
            devicedir_memory_blocks(root, list, iopath, flags);
            s = OSAL_SUCCESS;
            break;

        case IO_DD_OVERRIDES:
            label = "overrides";
            devicedir_overrides(list, flags);
            s = OSAL_SUCCESS;
            break;

        case IO_DD_INFO:
            label = "device info";
            devicedir_info(root, list, flags);
            s = OSAL_SUCCESS;
            break;

        case IO_DD_DYNAMIC_SIGNALS:
#if IOC_DYNAMIC_MBLK_CODE
            label = "dynamic signals";
            devicedir_dynamic_signals(root, list, iopath, flags);
            s = OSAL_SUCCESS;
#else
            label = "dynamic signal support not included in build";
#endif
            break;
    }

    osal_stream_write(list, "\0", 1, &n, OSAL_STREAM_DEFAULT);

    if (plabel) {
        *plabel = label;
    }
    return s;
}


