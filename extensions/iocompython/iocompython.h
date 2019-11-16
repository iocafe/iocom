/**

  @file    iocompython.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  See https://www.hardikp.com/2017/12/30/python-cpp/
  
  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOCOMPYTHON_INCLUDED
#define IOCOMPYTHON_INCLUDED

#define PY_SSIZE_T_CLEAN
#include <python3.5/Python.h>
#include <python3.5/structmember.h>

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Trace prints.
 */
#define IOPYTHON_TRACE 1

/* Include all iocompythony headers.
 */
#include "extensions/iocompython/code/iopy_module.h"
#include "extensions/iocompython/code/iopy_root.h"
#include "extensions/iocompython/code/iopy_memory_block.h"
#include "extensions/iocompython/code/iopy_connection.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
