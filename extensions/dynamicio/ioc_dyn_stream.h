/**

  @file    ioc_dyn_stream.h
  @brief   Dynamic streamed data transfer API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  This is interface to ioc_streamer function to implement typical functionality easier
  in environment where use dynamic memory allocation is feasible. This interface is
  not suitable for microcontrollers with limited resources: In limited resource environment
  use ioc_streamer directly, it doesn't need dynamic memory allocation and doesn't make
  buffer all data to be transferred in RAM.

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
#if IOC_DYNAMIC_MBLK_CODE

/* Structure to allocate memory for signals.
 */
typedef struct
{
    iocSignal cmd;
    iocSignal select;
    iocSignal buf;
    iocSignal head;
    iocSignal tail;
    iocSignal state;
    iocSignal err;
    iocSignal cs;
}
iocStreamSignals;

/* Flags for functions, bit fields. The same numeric values are used IOC_IS_CONTROLLER and
 * IOC_IS_DEVICE as for IOC_BRICK_CONTROLLER  and IOC_BRICK_DEVICE ioc_brick.h.
 */
#define IOC_CALL_SYNC 1
#define IOC_IS_CONTROLLER 2
#define IOC_IS_DEVICE 4


/**
****************************************************************************************************
    Stream structure.
****************************************************************************************************
*/
typedef struct iocStream
{
    /** Pointer to IOCOM root object
     */
    iocRoot *root;

    /** Flags, either OSAL_STREAM_READ or OSAL_STREAM_WRITE.
     */
    os_int flags;

    /** Select persistent block number, etc, transfer option.
     */
    os_int select;

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

    /** Write buffer, plain buffer allocated with os_malloc. OS_NULL if none.
     */
    os_char *write_buf;
    os_int write_buf_sz;
    os_int write_buf_pos;
    os_boolean write_buf_allocated;

    /** Read buffer, stream buffer class. OS_NULL if none.
     */
    osalStream read_buf;

    /** Number of byted moved.
     */
    os_memsz bytes_moved;
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
    const os_char *frd_buf_name,
    const os_char *tod_buf_name,
    const os_char *exp_mblk_name,
    const os_char *imp_mblk_name,
    const os_char *device_name,
    os_uint device_nr,
    const os_char *network_name,
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
    const os_char *buf,
    os_memsz buf_sz,
    os_boolean copy_buf);

/* Call run repeatedly until data transfer is complete or has failed.
 */
osalStatus ioc_run_stream(
    iocStream *stream,
    os_int flags);

/* Get pointer to received data, valid until next stream function call.
   Does not allocate new copy, returns the pointer to data stored
   within the stream object.
 */
os_char *ioc_get_stream_data(
    iocStream *stream,
    os_memsz *buf_sz,
    os_int flags);

/* Get delayed stream status (for example when programming flash). Can be used after
   ioc_run_stream() has returned OSAL_COMPLETED, for now supported only for writing to device).
 */
osalStatus ioc_stream_status(
    iocStream *stream);

/* Setup initial stream signal states, either for device or controller.
 */
osalStatus ioc_stream_initconf(
    iocStream *stream,
    os_int flags);

/* Number of bytes moved trough the stream, macro.
 */
#define ioc_stream_nro_bytes_moved(st) ((st)->bytes_moved)


#endif
