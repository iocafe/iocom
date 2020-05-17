/**

  @file    iocompython.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  See https://www.hardikp.com/2017/12/30/python-cpp/
  
  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOCOMPYTHON_INCLUDED
#define IOCOMPYTHON_INCLUDED

/* Include eosal, just to get operating system before including Python headers.
 */
#include "eosal.h"

#define PY_SSIZE_T_CLEAN
#ifdef OSAL_WINDOWS
#include <Python.h>
#include <structmember.h>
#else
#include <python3.8/Python.h>
#include <python3.8/structmember.h>
// #include <python3.7m/Python.h>
// #include <python3.7m/structmember.h>
#endif

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"
#include "devicedir.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Trace prints.
 */
#define IOPYTHON_TRACE 1

/* Include all iocompythony headers.
 */
#include "code/iopy_module.h"
#include "code/iopy_root.h"
#include "code/iopy_memory_block.h"
#include "code/iopy_connection.h"
#include "code/iopy_end_point.h"
#include "code/iopy_signal.h"
#include "code/iopy_brick_buffer.h"
#include "code/iopy_stream.h"
#include "code/iopy_util.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
