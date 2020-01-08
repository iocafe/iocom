/**

  @file    ioc_server_util.c
  @brief   Server side helper functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_SERVER_EXTENSIONS

/**
****************************************************************************************************

  @brief Store memory block handle pointer within all signals in signal structure.
  @anchor ioc_set_handle_to_signals

  The ioc_set_handle_to_signals() function stores memory block handle pointer for all signals
  in signals sturcture.

  @param   mblk_hdr Header of signal structure for the memory block.
  @param   handle Memory block handle pointer to store.
  @return  None.

****************************************************************************************************
*/
void ioc_set_handle_to_signals(
    iocMblkSignalHdr *mblk_hdr,
    iocHandle *handle)
{
    iocSignal *sig;
    os_int count;

    mblk_hdr->handle = handle;

    count = mblk_hdr->n_signals;
    sig = mblk_hdr->first_signal;

    while (count--)
    {
        sig->handle = handle;
        sig++;
    }
}

#endif
