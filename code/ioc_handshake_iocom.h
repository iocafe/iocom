/**

  @file    ioc_handshake_iocom.h
  @brief   Iocom specific code for first handshake.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_HANDSHAKE_IOCOM_H_
#define IOC_HANDSHAKE_IOCOM_H_
#include "iocom.h"
#if OSAL_SOCKET_SUPPORT

struct iocConnection;

/* Do first handshake for base IOCOM protocol (only sockets).
 */
osalStatus ioc_first_handshake(
    struct iocConnection *con);

#endif
#endif
