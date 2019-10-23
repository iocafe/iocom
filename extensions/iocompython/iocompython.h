/**

  @file    iocompython.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019
  
  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOCOMPYTHON_INCLUDED
#define IOCOMPYTHON_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include all iocompythony headers.
 */
#include "extensions/iocompython/code/io_py_connect.h"
#include "extensions/iocompython/code/io_py_memblk.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
