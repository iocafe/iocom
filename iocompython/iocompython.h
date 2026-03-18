/**

  @file    iocompython.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  See https://www.hardikp.com/2017/12/30/python-cpp/
  
  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOCOMPYTHON_INCLUDED
#define IOCOMPYTHON_INCLUDED

/* Include eosal to get operating system in case we need to check it for including Python headers.
 */
#include "eosal.h"

/* Guess where Python header could be? Is there better way to do this?
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"
#include "devicedir.h"

/* If C++ compilation, all functions, etc. from this point on included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Trace prints.
 */
#define IOPYTHON_TRACE 1

/* Include all iocompython headers.
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
