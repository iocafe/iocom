/**

  @file    ioc_streamer.h
  @brief   Data stream trough memory block API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.12.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#ifndef IOC_STREAMER_INCLUDED
#define IOC_STREAMER_INCLUDED


/** Stream interface structure for streamers.
 */
#if OSAL_FUNCTION_POINTER_SUPPORT
extern const osalStreamInterface osal_streamer_iface;
#endif

/** Define to get streamer interface pointer. The define is used so that this can
    be converted to function call.
 */
#define OSAL_STREAMER_IFACE &osal_streamer_iface


/**
****************************************************************************************************

  @name Streamer structure (pointer to this structure is casted to osalStream for interface).

****************************************************************************************************
 */
typedef struct iocStreamer
{
    /** A stream structure must start with this generic stream header structure, which contains
        parameters common to every stream.
     */
    osalStreamHeader hdr;

    iocSignal *cmd;
    iocSignal *select;
    iocSignal *buf;
    iocSignal *head;
    iocSignal *tail;
    iocSignal *state;
}
iocStreamer;


/**
****************************************************************************************************

  @name Parameter structure for opening a streamer (options).

****************************************************************************************************
 */
typedef struct iocStreamerParams
{
    iocSignal *cmd;
    iocSignal *select;
    iocSignal *buf;
    iocSignal *head;
    iocSignal *tail;
    iocSignal *state;
}
iocStreamerParams;



/**
****************************************************************************************************

  @name OSAL Streamer Functions.

  These functions implement streamers as OSAL stream. These functions can either be called
  directly or through stream interface.

****************************************************************************************************
 */
/*@{*/

/* Open streamer.
 */
osalStream osal_streamer_open(
    const os_char *parameters,
    void *option,
    osalStatus *status,
    os_int flags);

/* Close streamer.
 */
void osal_streamer_close(
    osalStream stream);

/* Accept connection from listening streamer.
 */
osalStream osal_streamer_accept(
    osalStream stream,
    osalStatus *status,
    os_int flags);

/* Flush written data to streamer.
 */
osalStatus osal_streamer_flush(
    osalStream stream,
    os_int flags);

/* Write data to streamer.
 */
osalStatus osal_streamer_write(
    osalStream stream,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags);

/* Read data from streamer.
 */
osalStatus osal_streamer_read(
    osalStream stream,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags);

/* Get streamer parameter.
 */
os_long osal_streamer_get_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix);

/* Set streamer parameter.
 */
void osal_streamer_set_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix,
    os_long value);

/* Wait for new data to read, time to write or operating system event, etc.
 */
#if OSAL_STREAMER_SELECT_SUPPORT
osalStatus osal_streamer_select(
    osalStream *streams,
    os_int nstreams,
    osalEvent evnt,
    osalSelectData *selectdata,
    os_int timeout_ms,
    os_int flags);
#endif


/*@}*/



/* Setup signals for streaming.
*/
void ioc_set_streamer_xxpool(
    iocStreamer *streamer,
    iocSignal *cmd,
    iocSignal *select,
    iocSignal *buf,
    iocSignal *head,
    iocSignal *tail,
    iocSignal *state);

os_memsz write(
    iocStreamer *streamer,
    const os_char *buf,
    os_memsz buf_sz);

os_memsz read(
    iocStreamer *streamer,
    const os_char *buf,
    os_memsz buf_sz);


#endif
