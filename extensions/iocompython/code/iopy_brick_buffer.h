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

#define IOPY_BB_PREFIX_SZ IOC_SIGNAL_NAME_SZ

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

    /* Signal structures.
     */
    iocSignal sig_cmd;
    iocSignal sig_select;
    iocSignal sig_buf;
    iocSignal sig_head;
    iocSignal sig_tail;
    iocSignal sig_state;

    /** Memory block handles.
     */
    iocHandle h_exp;
    iocHandle h_imp;

    /** Identifiers for the brick_buffer.
     */
    iocIdentifiers exp_ids;
    iocIdentifiers imp_ids;

    os_char prefix[IOPY_BB_PREFIX_SZ];

    /** Flags. The is_device indicates that this python code is acting as device and
        of brick data transfer. The from_device flag indicates that the device is the
        data source and controller is target.
     */
    os_boolean is_device;
    os_boolean from_device;
    os_boolean flat_buffer;

    /** Connection initialization status: 0 = all good, other values are errors.
        Set by the constructor function.
     */
    int status;
}
BrickBuffer;

/* For setting up the class within Python module.
 */
extern PyTypeObject BrickBufferType;

