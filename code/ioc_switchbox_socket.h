/**

  @file    ioc_switchbox_socket.h
  @brief   Stream class to route an IO sevice end point to switchbox cloud server.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  SwitchboxSocket speficic function prototypes and definitions to implement OSAL stream API for sockets.
  OSAL stream API is abstraction which makes streams (including sockets) look similar to upper
  levels of code, regardless of operating system or network library implementation.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_SWITCHBOX_SOCKET_H_
#define IOC_SWITCHBOX_SOCKET_H_
#include "iocom.h"
#if IOC_SWITCHBOX_SUPPORT

/** Stream interface structure for sockets.
 */
extern OS_CONST_H osalStreamInterface ioc_switchbox_socket_iface;

/* Default socket port number for SWITCHBOX. Only TLS can be used with switchbox.
 */
#define IOC_DEFAULT_IOCOM_SWITCHBOX_TLS_PORT 6362
#define IOC_DEFAULT_IOCOM_SWITCHBOX_TLS_PORT_STR "6372"

#define IOC_DEFAULT_ECOM_SWITCHBOX_TLS_PORT 6363
#define IOC_DEFAULT_ECOM_SWITCHBOX_TLS_PORT_STR "6373"

/** Define to get socket interface pointer. The define is used so that this can
    be converted to function call.
 */
#define IOC_SWITCHBOX_SOCKET_IFACE &ioc_switchbox_socket_iface


#endif
#endif
