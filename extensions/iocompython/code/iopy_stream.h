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


typedef struct
{
    iocSignal cmd;
    iocSignal select;
    iocSignal buf;
    iocSignal head;
    iocSignal tail;
    iocSignal state;
}
iocStreamSignalsStruct;


/** Python stream class.
 */
typedef struct
{
    PyObject_HEAD

    /** Pointer to python toot object, notice reference counting.
     */
    Root *pyroot;

    /** IOCOM stream parameters.
     */
    iocStreamerParams prm;

    /** Signals for transferring data from device.
     */
    iocStreamSignalsStruct frd;

    /** Signals for transferring data to device.
     */
    iocStreamSignalsStruct tod;

    /** Buffer names
     */
    os_char frd_signal_name_prefix[IOC_SIGNAL_NAME_SZ];
    os_char tod_signal_name_prefix[IOC_SIGNAL_NAME_SZ];

    /** Memory block handles.
     */
    iocHandle exp_handle;
    iocHandle imp_handle;

    /** Streamer handle (ioc_streamer)
     */
    osalStream streamer;

    /** Flag indicating the streamer has been opened successfully and cannot be opened again
        for this instance of Python API stream object.
     */
    os_boolean streamer_opened;

    /** Identifiers for the signals.
     */
    iocIdentifiers exp_identifiers;
    iocIdentifiers imp_identifiers;

    /** Connection initialization status: 0 = all good, other values are errors.
        Set by the constructor function.
     */
    int status;
}
Stream;

/* For setting up the class within Python module.
 */
extern PyTypeObject StreamType;

