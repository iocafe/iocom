/**

  @file    ioc_dyn_stream.c
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

#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE


/**
****************************************************************************************************

  @brief Allocate and initialize stream.
  @anchor ioc_initialize_dynamic_signal

  The ioc_open_stream() function allocates and sets up a new stream structure.

  @return  Pointer to stream structure, or OS_NULL if memory allocation failed.

****************************************************************************************************
*/
iocStream *ioc_open_stream(
    iocRoot *root,
    os_int select,
    os_char *read_buf_signal_name,
    os_char *write_buf_signal_name,
    os_char *exp_mblk_path,
    os_char *imp_mblk_path,
    os_int flags)
{
    iocStream *stream;

    stream = (iocStream*)os_malloc(sizeof(iocStream), OS_NULL);
    if (stream == OS_NULL) return OS_NULL;
    os_memclear(stream, sizeof(iocStream));

    return stream;
}


/**
****************************************************************************************************

  @brief Release stream structure.
  @anchor ioc_release_stream

  The ioc_release_stream() function frees memory allocated for the stream structure.

  @param   dsignal Pointer to stream structure to release.
  @return  None.

****************************************************************************************************
*/
void ioc_release_stream(
    iocStream *stream)
{
    os_free(stream, sizeof(iocStream));
}


/* Start reading data from stream.
 */
void ioc_start_stream_read(
    iocStream *stream)
{
}

/* Start writing data to stream.
 */
void ioc_start_stream_write(
    iocStream *stream,
    os_char *buf,
    os_memsz *buf_sz)
{
}

/* Call run repeatedly until data transfer is complete or has failed.
 */
osalStatus ioc_run_stream(
    iocStream *stream)
{
    return OSAL_STATUS_FAILED;
}

/* Get pointer to received data, valid until next stream function call.
   Does not allocate new copy, returns the pointer to data stored
   within the stream object.
 */
os_char *ioc_get_stream_data(
    iocStream *stream,
    os_memsz *buf_sz)
{
    *buf_sz = 0;
    return OS_NULL;
}

#endif
