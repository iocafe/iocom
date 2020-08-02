/**

  @file    minion.h
  @brief   Minion camera IO example.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.4.2020

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
#include "deviceinfo.h"
#include "devicedir.h"

/* Include IO pin and communication configuration headers generated by "config" JSON files.
 */
#include "json_io_config.h"

struct pinsPhoto;

/* Gazerbeamm library enables wifi configuration by Android phone's flash light. The library
   if used if "gazerbeam" input in pins_io.json.
 */
#ifndef IOCOM_USE_GAZERBEAM
  #ifdef PINS_INPUTS_GAZERBEAM
    #define IOCOM_USE_GAZERBEAM 1
  #else
    #define IOCOM_USE_GAZERBEAM 0
  #endif
#endif

/* Blink network status as morse code if we have an output IO pin named "led_morse".
 */
#ifndef IOCOM_USE_MORSE
  #ifdef PINS_OUTPUTS_LED_MORSE
    #define IOCOM_USE_MORSE 1
  #else
    #define IOCOM_USE_MORSE 0
  #endif
#endif

/* Use lighthouse library to get server's IP address from UDP multicast (0 or 1) in same LAN segment?
 */
#ifndef IOCOM_USE_LIGHTHOUSE
#define IOCOM_USE_LIGHTHOUSE 1
#endif

/* Sensibility check: Allow lighthouse only for socket clients.
 */
#if (IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_SOCKET)==0 || (IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_SERVER)
#undef IOCOM_USE_LIGHTHOUSE
#define IOCOM_USE_LIGHTHOUSE 0
#endif
