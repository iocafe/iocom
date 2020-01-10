/**

  @file    nodeconf.h
  @brief   Data structures, defines and functions for managing network node configuration and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  The nodeconf library provides data structure for storing IO network node configuration, and functions
  for accessing, modifying and saving/loading it.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef NODECONF_INCLUDED
#define NODECONF_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include all nodeconf headers.
 */
#include "extensions/nodeconf/code/nodeconf_data_struct.h"
#include "extensions/nodeconf/code/nodeconf_setup.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
