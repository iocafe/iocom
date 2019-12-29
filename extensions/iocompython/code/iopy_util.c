/**

  @file    iopy_util.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocompython.h"


/**
****************************************************************************************************
  Convert JSON text to packed binary JSON.
****************************************************************************************************
*/
PyObject *iocom_python_json2bin(
    PyObject *self,
    PyObject *args)
{
    const char *json_text = NULL;
    osalStream compressed;
    PyObject *rval;
    os_char *data;
    os_memsz data_sz;

    if (!PyArg_ParseTuple(args, "s", &json_text))
    {
        PyErr_SetString(iocomError, "JSON string expected as argument");
        return NULL;
    }

    compressed = osal_stream_buffer_open(OS_NULL, OS_NULL, OS_NULL, OSAL_STREAM_DEFAULT);
    osal_compress_json(compressed, json_text, "title", OSAL_JSON_SIMPLIFY);
    data = osal_stream_buffer_content(compressed, &data_sz);
    rval = PyBytes_FromStringAndSize(data, data_sz);
    osal_stream_close(compressed, OSAL_STREAM_DEFAULT);
    return rval;
}


/**
****************************************************************************************************
  Convert packed binary JSON to text.
****************************************************************************************************
*/
PyObject *iocom_python_bin2json(
    PyObject *self,
    PyObject *args)
{
    osalStream uncompressed;
    PyObject *rval, *pydata = NULL;
    os_char *data = OS_NULL;
    os_memsz data_sz;
    osalStatus s;
    char *buffer;
    Py_ssize_t length;

    if (!PyArg_ParseTuple(args, "O", &pydata))
    {
        PyErr_SetString(iocomError, "Binary JSON object expected as argument");
        return NULL;
    }

    if (PyBytes_AsStringAndSize(pydata, &buffer, &length) == -1)
    {
        PyErr_SetString(iocomError, "Argument is not valud Bytes object");
        return NULL;
    }
    uncompressed = osal_stream_buffer_open(OS_NULL, OS_NULL, OS_NULL, OSAL_STREAM_DEFAULT);
    s = osal_uncompress_json(uncompressed, buffer, length, 0);
    if (s == OSAL_SUCCESS)
    {
        data = osal_stream_buffer_content(uncompressed, &data_sz);
    }

    if (data)
    {
        rval = Py_BuildValue("s#", (char *)data, (int)data_sz);
    }
    else
    {
        Py_INCREF(Py_None);
        rval = Py_None;
    }

    osal_stream_close(uncompressed, OSAL_STREAM_DEFAULT);
    return rval;
}
