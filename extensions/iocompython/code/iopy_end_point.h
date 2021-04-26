/**

  @file    iopy_end_point.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  An end point listens for incoming connections.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/** Python end point class.
 */
typedef struct
{
    PyObject_HEAD

    /** Pointer to IOCOM end point object, OS_NULL if none. Used for sockets.
     */
    iocEndPoint *epoint;

    /** Pointer to IOCOM end point object, OS_NULL if none. Used for serial communication.
     */
    iocConnection *con;

    /** Connection initialization status: 0 = all good, other values are errors.
        Set by the constructor function.
     */
    int status;
}
EndPoint;

/* For setting up the class within Python module.
 */
extern PyTypeObject EndPointType;

