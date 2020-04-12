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
#include "iocom.h"
#if IOC_STREAMER_SUPPORT

/**
****************************************************************************************************

  @brief Initialize output stream.
  @anchor ioc_initialize_output_stream

  @param   o Sturcture to hold output stream state.
  @param   signals Signals to control the output stream.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_output_stream(
    iocOutputStream *o,
    iocStreamerSignals *signals)
{
    os_memclear(o, sizeof(iocOutputStream));
    os_memcpy(&o->sig, signals, sizeof(iocStreamerSignals));

    ioc_sets0_int(o->sig.state, 0);

    /* o->stream = ioc_streamer_open(OS_NULL, fs
        const os_char *parameters,
    void *option,
    osalStatus *status,
    os_int flags);
    */

}

/* Release output stream.
 */
void ioc_release_output_stream(
    iocOutputStream *o)
{
    /* void ioc_streamer_close(
    osalStream stream,
    os_int flags); */
}


/* Release memory block object.
 */
osalStatus ioc_write_item_to_output_stream(
    iocOutputStream *o,
    const os_char *data,
    os_memsz data_sz)
{
    /* Make sure that all data firts
     */
    /* os_long ioc_streamer_get_parameter(
    osalStream stream,
    osalStreamParameterIx parameter_ix); */

    return OSAL_STATUS_FAILED;
}


#endif
