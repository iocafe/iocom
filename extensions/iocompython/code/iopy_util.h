/**

  @file    iopy_util.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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

/* Get security secret.
 */
PyObject *iocom_python_get_secret(
    PyObject *self);

/* Get automatically generated device password.
 */
PyObject *iocom_python_get_password(
    PyObject *self);

/* Hash password (run SHA-256 hash on password).
 */
PyObject *iocom_python_hash_password(
    PyObject *self,
    PyObject *args);

/* Forget the secret (and password).
 */
PyObject *iocom_python_forget_secret(
    PyObject *self);
