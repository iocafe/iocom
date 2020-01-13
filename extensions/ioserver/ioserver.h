/**

  @file    ioserver.h
  @brief   Extension library for implementing server with authentication and authorization.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  The ioserver is optional extension library for implementing server. This should be sufficient
  for simple secure servers as is, or as starting point for implementing your own authentication
  and authorization.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOSERVER_INCLUDED
#define IOSERVER_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

#include "code/ioc_accounts.h"
#include "code/ioc_server_util.h"
#include "code/ioc_basic_server.h"
#include "code/ioc_load_persistent_into_mblk.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
