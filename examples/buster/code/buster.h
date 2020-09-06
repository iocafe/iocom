/**

  @file    buster.h
  @brief   Shared top header file for Buster app.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef BUSTER_H_
#define BUSTER_H_
#include "iodevice.h"

/* Include IO pin and communication configuration headers generated by "config" JSON files.
 */
#include "json_io_config.h"

/* Blink network status as morse code if we have an output IO pin named "led_morse". Either PWM
   or normal digital output is fine, define IOCOM_MORSEPPIN as pin to use. PWM is marked with
   define value 2.
 */
#ifndef IOCOM_USE_MORSE
  #ifdef PINS_OUTPUTS_LED_MORSE
    #define IOCOM_USE_MORSE 1
    #define IOCOM_MORSEPPIN &pins.outputs.led_morse
  #endif
#endif

using IoDevice::AbstractApplication;
using IoDevice::AbstractCamera;
using IoDevice::AbstractAppParams;
using IoDevice::AbstractSlaveDevice;
using IoDevice::AbstractSequence;


/* Include application headers.
 */
#include "minion.h"
#include "blink_sequence.h"
#include "camera.h"
#include "application.h"

#endif
