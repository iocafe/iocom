/**

  @file    deviceinfo.h
  @brief   Switchbox connection passtrough library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef SWITCHBOX_H_
#define SWITCHBOX_H_
// #include "iocom.h"
#include "eosalx.h"

/* If C++ compilation, all functions, etc. from this point on included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include all deviceinfo headers.
 */
#include "code/switchbox_root.h"
#include "code/switchbox_end_point.h"
#include "code/switchbox_connection.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif

