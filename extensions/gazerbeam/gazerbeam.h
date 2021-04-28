/**

  @file    gazerbeam.h
  @brief   Library's top header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    9.3.2020

  The gazerbeam is small library to allow setting wifi network name (SSID) and password (PSK)
  using flash light of Android phone and photo diode connected to microcontroller. This is
  microcontroller end of code.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef GAZERBEAM_H_
#define GAZERBEAM_H_
#include "iocom.h"

/* Do we want to include code for pins library support (GPIO input for
   phototransistor and pin state change interrupt handling.
 */
#define GAZERBEAM_PINS_SUPPORT 1

/* If C++ compilation, all functions, etc. from this point on included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Type of AD conveted value in software.
 */
#define GAZERBEAM_VALUE os_int

#include "code/gazerbeam_minmax.h"
#include "code/gazerbeam_receive.h"
#include "code/gazerbeam_save_config.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
