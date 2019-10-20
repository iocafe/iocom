/**

  @file    iotopology.h
  @brief   Data structures, defines and functions for managing network topology and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  The iotopology library provides data structure for storing IO network topology, and functions
  for accessing, modifying and saving/loading it.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOTOPOLOGY_INCLUDED
#define IOTOPOLOGY_INCLUDED

/* Include operating system abstraction layer with extension headers.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include all iotopology headers.
 */
#include "extensions/iotopology/code/iotopology_types.h"
#include "extensions/iotopology/code/iotopology_conf.h"
#include "extensions/iotopology/code/iotopology_persistent.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
