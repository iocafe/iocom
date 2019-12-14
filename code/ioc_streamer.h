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


#ifndef IOC_DEVICE_STREAMER
#define IOC_DEVICE_STREAMER 1
#endif
#ifndef IOC_CONTROLLER_STREAMER
#define IOC_CONTROLLER_STREAMER 1
#endif

#define IOC_STREAMER_SUPPORT (IOC_DEVICE_STREAMER || IOC_CONTROLLER_STREAMER)
#if IOC_STREAMER_SUPPORT

/** Stream interface structure for streamers.
 */
extern const osalStreamInterface ioc_streamer_iface;

/** Define to get streamer interface pointer. The define is used so that this can
    be converted to function call.
 */
#define IOC_STREAMER_IFACE &ioc_streamer_iface

/* Maximum number of streamers when using static memory allocation.
 */
#if OSAL_DYNAMIC_MEMORY_ALLOCATION == 0
#ifndef IOC_MAX_STREAMERS
#define IOC_MAX_STREAMERS 4
#endif
#endif


/**
****************************************************************************************************

  @name Parameter structure for opening a streamer (options).

****************************************************************************************************
 */
typedef struct iocStreamerSignals
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


/**
****************************************************************************************************

  @name Streamer states.

****************************************************************************************************
 */
typedef enum
{
    IOC_STREAM_IDLE = 0,
    IOC_STREAM_RUNNING = 1,
    IOC_STREAM_COMPLETED = 2,
    IOC_STREAM_INTERRUPT = 3
}
iocStreamerState;


/**
****************************************************************************************************

  @name Parameter structure for opening a streamer (options).

  is_device set to OS_TRUE if this is IO device end of communication. This effects logic of
  streaming and who is initiating the transfers. Other end of stream must be marked as device
  and the other not.

****************************************************************************************************
 */
typedef struct iocStreamerParams
{
    os_boolean is_device;

    iocStreamerSignals frd;
    iocStreamerSignals tod;
}
iocStreamerParams;


typedef enum
{
    IOC_SSTEP_INITIALIZED,
    IOC_SSTEP_INITIALIZED2,
    IOC_SSTEP_TRANSFER_DATA,
    IOC_SSTEP_TRANSFER_DONE,
    IOC_SSTEP_ALL_COMPLETED,
    IOC_SSTEP_FAILED,
    IOC_SSTEP_FAILED2,
    IOC_SSTEP_FAILED_AND_IDLE_SET
}
iocStreamerStep;


/**
****************************************************************************************************

  @name Streamer structure (pointer to this structure is casted to osalStream for interface).

  tod = to IO device
  frd = from IO device

****************************************************************************************************
 */
typedef struct iocStreamer
{
    /** A stream structure must start with this generic stream header structure, which contains
        parameters common to every stream.
     */
    osalStreamHeader hdr;

    iocStreamerParams *prm;

    iocStreamerStep step;
    os_int select;

    os_int head;
    os_int tail;

    os_int flags;
    os_timer mytimer;
    os_boolean used;
}
iocStreamer;



/**
****************************************************************************************************

  @name IO device control stream transfer state structure.

****************************************************************************************************
 */
typedef struct
{
#if OSAL_DEBUG
    /* Initial signal values set flag, used for debugging.
     */
    os_boolean initialized;
#endif

    /* Transfer from device to controller.
     */
    osalStream frd;
    osPersistentHandle *fdr_persistent;
    os_boolean fdr_persistent_ok;

    /* To device from controller.
     */
    osalStream tod;
    osPersistentHandle *tod_persistent;
}
iocControlStreamState;


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
    osalStream stream,
    os_int flags);

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

/* Initialize streamer data structure.
 */
void ioc_streamer_initialize(
    void);

/* Initialize control stream.
 */
void ioc_init_control_stream(
    iocControlStreamState *ctrl,
    iocStreamerParams *params);

/* Keep control stream alive, move data to/from persistent memory (on IO device).
 */
void ioc_run_control_stream(
    iocControlStreamState *ctrl,
    iocStreamerParams *params);

/*@}*/

#endif
#endif