/**

  @file    ioc_dyn_stream.h
  @brief   Dynamic streamed data transfer API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    9.12.2019

  This is interface to ioc_streamer function to implement typical functionality easier.

  Read:
    call ioc_open_stream
    call ioc_start_stream_read
    call ioc_run_stream repeatedly until completed or failed
    call ioc_get_stream_data to get data
    call either ioc_release_stream or ioc_start_stream_read

  Write:
    call ioc_open_stream
    call ioc_start_stream_write
    call ioc_run_stream repeatedly until completed or failed
    call ioc_release_stream

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_DYN_STREAM_INCLUDED
#define IOC_DYN_STREAM_INCLUDED
#if IOC_DYNAMIC_MBLK_CODE


typedef struct
{
    iocSignal cmd;
    iocSignal select;
    iocSignal buf;
    iocSignal head;
    iocSignal tail;
    iocSignal state;
}
iocStreamSignals;


/**
****************************************************************************************************
    Stream structure.
****************************************************************************************************
*/
typedef struct iocStream
{
    /** IOCOM stream parameters.
     */
    iocStreamerParams prm;

    /** Signals for transferring data from device.
     */
    iocStreamSignals frd;

    /** Signals for transferring data to device.
     */
    iocStreamSignals tod;

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
}
iocStream;


/**
****************************************************************************************************
    Stream related functions.
****************************************************************************************************
*/
/* Allocate and initialize stream.
 */
iocStream *ioc_open_stream(
    iocRoot *root,
    os_int select,
    os_char *read_buf_signal_name,
    os_char *write_buf_signal_name,
    os_char *exp_mblk_path,
    os_char *imp_mblk_path,
    os_int flags);

/* Release stream structure.
 */
void ioc_release_stream(
    iocStream *stream);

/* Start reading data from stream.
 */
void ioc_start_stream_read(
    iocStream *stream);

/* Start writing data to stream.
 */
void ioc_start_stream_write(
    iocStream *stream,
    os_char *buf,
    os_memsz *buf_sz);

/* Call run repeatedly until data transfer is complete or has failed.
 */
osalStatus ioc_run_stream(
    iocStream *stream);

/* Get pointer to received data, valid until next stream function call.
   Does not allocate new copy, returns the pointer to data stored
   within the stream object.
 */
os_char *ioc_get_stream_data(
    iocStream *stream,
    os_memsz *buf_sz);

#endif
#endif
