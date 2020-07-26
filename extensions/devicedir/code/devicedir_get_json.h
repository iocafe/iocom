/**

  @file    devicedir_get_json.h
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
#pragma once
#ifndef DEVICEDIR_GET_JSON_H_
#define DEVICEDIR_GET_JSON_H_
#include "devicedir.h"

/* Select what information to present as JSON.
 */
typedef enum ddSelectJSON
{
    IO_DD_CONNECTIONS = 10,
    IO_DD_END_POINTS = 20,
    IO_DD_MEMORY_BLOCKS = 30,
    IO_DD_OVERRIDES = 40,
    IO_DD_INFO = 50,
    IO_DD_DYNAMIC_SIGNALS = 60
}
ddSelectJSON;

/* Convert selected information to JSON text.
*/
osalStatus devicedir_get_json(
    iocRoot *root,
    osalStream list,
    ddSelectJSON select,
    const os_char *iopath,
    os_short flags,
    const os_char **plabel);

#endif
