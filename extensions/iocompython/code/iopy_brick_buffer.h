/**

  @file    iopy_brick_buffer.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    16.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

struct Root;


/** Python brick_buffer class.
 */
typedef struct
{
    PyObject_HEAD

    /** Pointer to python toot object, notice reference counting.
     */
    Root *pyroot;

    /** IOCOM brick_buffer object.
     */
    iocBrickBuffer brick_buffer;

    iocSignal sig_cmd;
    iocSignal sig_select;
    iocSignal sig_buf;
    iocSignal sig_head;
    iocSignal sig_tail;
    iocSignal sig_state;

    /** Memory block handle.
     */
    iocHandle handle;

    /** Identifiers for the brick_buffer.
     */
    iocIdentifiers identifiers;

    /** If brick_buffer is matrix formulated as array, number of columns. For one
        dimensional arrays and single variables ncolumns is 1.
     */
//     os_int ncolumns;

    /** Connection initialization status: 0 = all good, other values are errors.
        Set by the constructor function.
     */
    int status;
}
BrickBuffer;

/* For setting up the class within Python module.
 */
extern PyTypeObject BrickBufferType;

