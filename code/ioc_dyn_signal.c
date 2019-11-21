/**

  @file    ioc_dyn_signal.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic signal is extended signal structure, which is part of dynamic IO network
  information.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/* Allocate and initialize dynamic signal.
 */
iocDynamicSignal *ioc_initialize_dynamic_signal(
    const os_char *signal_name)
{
    iocDynamicSignal *dsignal;
    os_memsz sz;

    dsignal = (iocDynamicSignal*)os_malloc(sizeof(iocDynamicSignal), OS_NULL);
    os_memclear(dsignal, sizeof(iocDynamicSignal));

    sz = os_strlen(signal_name);
    dsignal->signal_name = os_malloc(sz, OS_NULL);
    os_memcpy(dsignal->signal_name, signal_name, sz);

    return dsignal;
}


/* Release dynamic signal.
 */
void ioc_release_dynamic_signal(
    iocDynamicSignal *dsignal)
{
    os_memsz sz;
    sz = os_strlen(dsignal->signal_name);
    os_free(dsignal->signal_name, sz);
    os_free(dsignal, sizeof(iocDynamicSignal));
}

#endif
