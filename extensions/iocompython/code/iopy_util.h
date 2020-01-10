/**

  @file    iopy_util.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Convert JSON text to packed binary JSON.
 */
PyObject *iocom_python_json2bin(
    PyObject *self,
    PyObject *args);

/* Convert packed binary JSON to text.
 */
PyObject *iocom_python_bin2json(
    PyObject *self,
    PyObject *args);
