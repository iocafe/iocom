/**

  @file    lighthouse.h
  @brief   Service discovery using UDP multicasts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.2.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef LIGHTHOUSE_INCLUDED
#define LIGHTHOUSE_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

#include "code/lighthouse_server.h"
#include "code/lighthouse_client.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
