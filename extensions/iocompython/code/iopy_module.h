/**

  @file    iopy_module.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Explose Python module interface.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iocompython/iocompython.h"


/* Select different python extension name depending if we are building release or debug version.
 */
#if OSAL_CC_DEBUG
  #define IOCOMPYTHON_NAME "iocompythond"
  #define IOCOMPYTHON_INIT_FUNC PyInit_iocompythond
#else
  #define IOCOMPYTHON_NAME "iocompython"
  #define IOCOMPYTHON_INIT_FUNC PyInit_iocompython
#endif


/* Python module definition
 */
extern struct PyModuleDef iocompythonmodule;

/* Python module rrror string
 */
extern PyObject *iocomError;

/* Initialize operating system abstraction layer and communication transport libraries.
 */
void iocom_python_initialize(void);

/* Shut down operating system abstraction layer and communication transport libraries.
 */
void iocom_python_release(void);
