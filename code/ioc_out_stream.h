/**

  @file    ioc_out_stream.h
  @brief   Sending stream data.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

#if IOC_STREAMER_SUPPORT


/**
****************************************************************************************************

  @name Parameter structure for opening a streamer (options).

****************************************************************************************************
 */
/* typedef struct iocStreamerSignals
{
    const iocSignal *cmd;
    const iocSignal *select;
    const iocSignal *buf;
    const iocSignal *head;
    const iocSignal *tail;
    const iocSignal *state;
    os_boolean to_device;
}
iocStreamerSignals;
*/




/**
****************************************************************************************************

****************************************************************************************************
 */
typedef struct iocOutputStream
{
    iocStreamerSignals sig;

    osalStream *stream;
}
iocOutputStream;


/**
****************************************************************************************************

****************************************************************************************************
 */

/* Initialize output stream.
 */
void ioc_initialize_output_stream(
    iocOutputStream *o,
    iocStreamerSignals *signals);

/* Release output stream.
 */
void ioc_release_output_stream(
    iocOutputStream *o);

/* Check if output stream is open?
 */
os_boolean ioc_output_stream_is_open(
    iocOutputStream *o);

/* Release memory block object.
 */
osalStatus ioc_write_item_to_output_stream(
    iocOutputStream *o,
    const os_char *data,
    os_memsz data_sz);

#endif
