/**

  @file    frank.h
  @brief   Frank controller using static IO device configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#include "nodeconf.h"
#include "ioserver.h"

OSAL_C_HEADER_BEGINS
#include "network-defaults.h"
#include "signals.h"
#include "info-mblk-binary.h"
OSAL_C_HEADER_ENDS

#include "frank_main.h"
#include "frank_application.h"

extern iocRoot ioapp_root;
