/**

  @file    ioc_selectwifi_config.c
  @brief   Set wifi network name and password over blue tooth or serial port.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    3.2.2020

  Include communication configuration code generated by scripts from JSON.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#define SELECTWIFI_INTERNALS
#include "selectwifi.h"

#include "swf-info-mblk.c"
#include "swf-signals.c"