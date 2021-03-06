/**

  @file    selectwifi.h
  @brief   Library's top header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    3.2.2020

  The selectwifi is small library to allow setting wifi network name (SSID) and password (pre
  shared key) over blue tooth or serial port. This library can be used when connecting IO
  board without display/keyboard to WiFi network. Functionality is limited to WiFi connection
  settigs: Since the blue tooth and serial port connections are unsecure, there is no support
  for software updates or further network or security configuration.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef SELECTWIFI_INCLUDED
#define SELECTWIFI_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

#include "code/selectwifix.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
