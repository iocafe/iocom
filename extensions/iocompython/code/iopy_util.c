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
  Convert JSON text to packed binary JSON file.
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
