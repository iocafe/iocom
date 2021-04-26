/**

  @file    devicedir.h
  @brief   List IO networks, devices, memory blocks and IO signals.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The devicedir library provides data structure for storing IO network node configuration, and functions
  for accessing, modifying and saving/loading it.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef DEVICEDIR_H_
#define DEVICEDIR_H_
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include all devicedir headers.
 */
#include "code/devicedir_shared.h"
#include "code/devicedir_console.h"
#include "code/devicedir_get_json.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
