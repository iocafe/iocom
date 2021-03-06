/**

  @file    iopy_signal.h
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

struct Root;


/** Python signal class.
 */
typedef struct
{
    PyObject_HEAD

    /** Pointer to python toot object, notice reference counting.
     */
    Root *pyroot;

    /** IOCOM signal object.
     */
    iocSignal signal;

    /** Memory block handle.
     */
    iocHandle handle;

    /** Identifiers for the signal.
     */
    iocIdentifiers identifiers;

    /** If signal is matrix formulated as array, number of columns. For one
        dimensional arrays and single variables ncolumns is 1.
     */
    os_int ncolumns;

    /** Connection initialization status: 0 = all good, other values are errors.
        Set by the constructor function.
     */
    int status;
}
Signal;

/* For setting up the class within Python module.
 */
extern PyTypeObject SignalType;

