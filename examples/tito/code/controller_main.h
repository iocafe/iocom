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

/* Do we need pins library (0/1).
 */
#define PINS_LIBRARY 1

#include "iocom.h"
#include "nodeconf.h"
#include "ioserver.h"
#include "lighthouse.h"

/* Include headers generated from JSON configuration.
 */
OSAL_C_HEADER_BEGINS
#if PINS_LIBRARY
#include "pinsx.h"
#include "pins-io.h"
#endif

#include "network-defaults.h"
#include "signals.h"
#include "info-mblk-binary.h"
OSAL_C_HEADER_ENDS

/* Include header files of this module
 */
#include "gina-for-tito.h"
#include "controller_root.h"
#include "iodevice_base_class.h"
#include "app_iodevice_gina.h"
#include "sequence_base_class.h"
#include "app_seq_blink_led.h"
#include "app_instance.h"

/* IOCOM root object for this program.
 */
extern iocRoot iocom_root;
