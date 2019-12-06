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
extern const osalStreamInterface ioc_streamer_iface;
#endif

/** Define to get streamer interface pointer. The define is used so that this can
    be converted to function call.
 */
#define OSAL_STREAMER_IFACE &ioc_streamer_iface


/* Define 1 to enable bidirectional streamer support.
 */
#define IOC_SUPPORT_BIDIRECTIONAL_STREAMER 1

/* Maximum number of streamers when using static memory allocation.
 */
#if OSAL_DYNAMIC_MEMORY_ALLOCATION == 0
#define IOC_MAX_STREAMERS 4
#endif

/**
****************************************************************************************************

  @name Parameter structure for opening a streamer (options).

****************************************************************************************************
 */
typedef struct iocStreamerSignals
{
    iocSignal *cmd;
    iocSignal *select;
    iocSignal *buf;
    iocSignal *head;
    iocSignal *tail;
    iocSignal *state;
    os_boolean to_device;
}
iocStreamerSignals;


/**
****************************************************************************************************

  @name Parameter structure for opening a streamer (options).

  is_device set to OS_TRUE if this is IO device end of communication. This effects logic of
  streaming and who is initiating the transfers. Other end of stream must be marked as device
  and the other not.

  static_signals Signals are allocated statically, do not allocate copies. If not set, signal
  structures are duplicated in case originals are deleted.

****************************************************************************************************
 */
typedef struct iocStreamerParams
{
    os_boolean is_device;
    // os_boolean static_signals;

    iocStreamerSignals s1;
#if IOC_SUPPORT_BIDIRECTIONAL_STREAMER
    iocStreamerSignals s2;
#endif
}
iocStreamerParams;


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

    iocStreamerParams prm;

    os_boolean used;
}
iocStreamer;



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
osalStream ioc_streamer_open(
    const os_char *parameters,
    void *option,
    osalStatus *status,
    os_int flags);

/* Close streamer.
 */
void ioc_streamer_close(
    osalStream stream);

/* Accept connection from listening streamer.
 */
osalStream ioc_streamer_accept(
    osalStream stream,
    osalStatus *status,
    os_int flags);

/* Flush written data to streamer.
 */
osalStatus ioc_streamer_flush(
    osalStream stream,
    os_int flags);

/* Write data to streamer.
 */
osalStatus ioc_streamer_write(
    osalStream stream,
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags);

/* Read data from streamer.
 */
osalStatus ioc_streamer_read(
    osalStream stream,
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags);

/* Get streamer parameter.
 */
os_long ioc_streamer_get_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix);

/* Set streamer parameter.
 */
void ioc_streamer_set_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix,
    os_long value);

/* Wait for new data to read, time to write or operating system event, etc.
 */
#if OSAL_STREAMER_SELECT_SUPPORT
osalStatus ioc_streamer_select(
    osalStream *streams,
    os_int nstreams,
    osalEvent evnt,
    osalSelectData *selectdata,
    os_int timeout_ms,
    os_int flags);
#endif


/* Initialize streamer data structure.
 */
void ioc_streamer_initialize(
    void);

/*@}*/


#endif
