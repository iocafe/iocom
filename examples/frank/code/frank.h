/**

  @file    frank.h
  @brief   Frank controller using static IO device configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.11.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#include "nodeconf.h"

OSAL_C_HEADER_BEGINS
#include "network-defaults.h"
#include "signals.h"
#include "accounts.h"
#include "account-defaults.h"
#include "info-mblk.h"
OSAL_C_HEADER_ENDS

#include "frank_main.h"
#include "frank_application.h"
#include "frank_get_netconf.h"

extern iocRoot ioapp_root;
