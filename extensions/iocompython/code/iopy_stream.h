/**

  @file    iopy_stream.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.12.2019

  Streaming data trough memory block.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
struct Root;


/** Python stream class.
 */
typedef struct
{
    PyObject_HEAD

    /** Pointer to python toot object, notice reference counting.
     */
    Root *pyroot;

    /** Contained IOCOM stream object.
     */
    iocStream *stream;

    /** Connection initialization status: 0 = all good, other values are errors.
        Set by the constructor function.
     */
    int status;
}
Stream;

/* Global blocking function to get configuration.
 */
PyObject *iocom_stream_getconf(
    PyObject *self,
    PyObject *args,
    PyObject *kwds);

/* Global blocking function to set configuration.
 */
PyObject *iocom_stream_setconf(
    PyObject *self,
    PyObject *args,
    PyObject *kwds);

/* Initialize configuration stream signals.
 */
PyObject *iocom_initconf(
    PyObject *self,
    PyObject *args,
    PyObject *kwds);

/* For setting up the class within Python module.
 */
extern PyTypeObject StreamType;

