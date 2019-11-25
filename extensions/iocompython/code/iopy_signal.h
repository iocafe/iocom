/**

  @file    iopy_signal.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  An signal listens for incoming connections.

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

    /* Pointer to python toot object, notice reference counting.
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

    /** Connection initialization status: 0 = all good, other values are errors.
        Set by the constructor function.
     */
    int status;
}
Signal;

/* For setting up the class within Python module.
 */
extern PyTypeObject SignalType;

