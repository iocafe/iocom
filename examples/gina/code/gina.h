/**

  @file    gina.h
  @brief   Gina IO board example featuring  IoT device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#define IOCOM_IOBOARD
#include "iocom.h"
#include "pinsx.h"
#include "nodeconf.h"

#include "pins-io.h"
#include "signals.h"
#include "info-mblk.h"
#include "network-defaults.h"


void ioboard_communication_callback(
    struct iocHandle *mblk,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

void ioboard_camera_callback(
    struct pinsPhoto *photo,
    void *context);
