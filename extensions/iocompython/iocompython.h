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

#define PY_SSIZE_T_CLEAN
#include <python3.5m/Python.h>

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* Select different python extension name depending if we are building release or debug version.
 */
#if OSAL_CC_DEBUG
  #define IOCOMPYTHON_NAME "iocompythond"
  #define IOCOMPYTHON_INIT_FUNC PyInit_iocompythond
#else
  #define IOCOMPYTHON_NAME "iocompython"
  #define IOCOMPYTHON_INIT_FUNC PyInit_iocompython
#endif

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
