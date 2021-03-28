/**

  @file    iocomx.h
  @brief   Extended iocom header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Extended iocom library main header file. This file includes both iocom base headers and
  extension headers related to dynamic IO configuration, used by generic server side apps
  and Python API. You should use #include "iocom.h", not #include "iocom.h".

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOCOMX_H_
#define IOCOMX_H_

/* Include base iocom headers.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

#if IOC_DYNAMIC_MBLK_CODE
#include "extensions/dynamicio/ioc_identifiers.h"
#include "extensions/dynamicio/ioc_dyn_signal.h"
#include "extensions/dynamicio/ioc_dyn_network.h"
#include "extensions/dynamicio/ioc_dyn_root.h"
#include "extensions/dynamicio/ioc_dyn_mblk_list.h"
#include "extensions/dynamicio/ioc_dyn_queue.h"
#include "extensions/dynamicio/ioc_dyn_stream.h"
#include "extensions/dynamicio/ioc_switchbox_auth_frame.h"
#include "extensions/dynamicio/ioc_switchbox_socket.h"
#endif

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
