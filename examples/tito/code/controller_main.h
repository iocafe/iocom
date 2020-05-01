/**

  @file    controller_main.h
  @brief   Program entry point, Tito IO controller set up.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#include "nodeconf.h"
#include "ioserver.h"
#include "lighthouse.h"
#include "gina-for-tito.h"
#include "controller_root.h"
#include "iodevice_base_class.h"
#include "app_iodevice_gina.h"
#include "sequence_base_class.h"
#include "app_seq_blink_led.h"
#include "app_instance.h"

/* Include headers generated from JSON configuration.
 */
OSAL_C_HEADER_BEGINS
#include "network-defaults.h"
#include "signals.h"
#include "info-mblk-binary.h"
OSAL_C_HEADER_ENDS

/* IOCOM root object for this application
 */
extern iocRoot app_iocom_root;
