/**

  @file    iopy_module.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Explose Python module interface.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Select same name (at least for now)regardless we are building release or debug version.
 */
#define IOCOMPYTHON_NAME "iocompython"
#define IOCOMPYTHON_INIT_FUNC PyInit_iocompython

/* Python module definition
 */
extern struct PyModuleDef iocompythonmodule;

/* Python module rrror string
 */
extern PyObject *iocomError;

/* Initialize operating system abstraction layer and communication transport libraries.
 */
void iocom_python_initialize(const char *security);

/* Shut down operating system abstraction layer and communication transport libraries.
 */
void iocom_python_release(void);
