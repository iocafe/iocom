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

struct pinsPhoto;

#include "iocom.h"
#include "pinsx.h"
#include "nodeconf.h"

/* Enable wifi configuration using blue tooth (0 or 1) ?.
 */
#ifndef GINA_USE_SELECTWIFI
#define GINA_USE_SELECTWIFI 0
#endif

/* Use Gazerbeamm library to enable wifi configuration by Android phone's flash light and
   phototransistor connected to microcontroller (0 or 1).
 */
#ifndef GINA_USE_GAZERBEAM
#define GINA_USE_GAZERBEAM 1
#endif

/* Get controller IP address from UDP multicast (0 or 1) ?.
 */
#ifndef GINA_USE_LIGHTHOUSE
#define GINA_USE_LIGHTHOUSE 1
#endif

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
