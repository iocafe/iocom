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
#ifndef GAZERBEAM_INCLUDED
#define GAZERBEAM_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

#include "code/gazerbeamx.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
