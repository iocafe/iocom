/**

  @file    ioc_dyn_signal.h
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
#ifndef IOC_DYN_SIGNAL_INCLUDED
#define IOC_DYN_SIGNAL_INCLUDED
#if IOC_DYNAMIC_MBLK_CODE

typedef struct iocDynamicSignal
{
    /* Signal name
     */
    os_char *name;

    /* Actual signal object.
     */
    iocSignal x;

    /* Next dynamic signal with same hash key.
     */
    struct iocDynamicSignal *next;
}
iocDynamicSignal;


/* Allocate and initialize dynamic signal.
 */
iocDynamicSignal *ioc_initialize_dynamic_signal(void);

/* Release dynamic signal.
 */
void ioc_release_dynamic_signal(
    iocDynamicSignal *dsignal);


#endif
#endif
