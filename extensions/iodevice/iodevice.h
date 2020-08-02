/**

  @file    iodevice_node_conf.h
  @brief   Publish device information.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  iodevice is iocom extension library to publish device's network configuration and state,
  software versions, used operating system and hardware, and resource/performance counters.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef DEVICEINFO_H_
#define DEVICEINFO_H_
#include "nodeconf.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include all iodevice headers.
 */
#include "code/iodevice_node_conf.h"
#include "code/iodevice_system_specs.h"
#include "code/iodevice_resource_monitor.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
