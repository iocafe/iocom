/**

  @file    iocdomain.h
  @brief   IO domain controller library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.9.2019

  Library for implementing IO domain controller functionality.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#ifndef IODOMAIN_INCLUDED
#define IODOMAIN_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* Include IO topology.
 */
#include "extensions/iotopology/iotopology.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include all iodomain headers.
 */
#include "extensions/iodomain/code/iodomain_main.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
