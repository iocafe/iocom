/**

  @file    ioc_bserver_config.c
  @brief   Basic server, compile code based on JSON configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "ioserver.h"

#include "config/include/generic/account-signals.h"
#include "config/include/account-defaults.h"
#include "config/include/accounts-mblk-binary.h"

#include "config/include/account-signals.c"
#include "config/include/account-defaults.c"
#include "config/include/accounts-mblk-binary.c"

