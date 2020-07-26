/**

  @file    ioc_parameters.h
  @brief   Persistent and volatile IO device parameters
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.6.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_PARAMETERS_H_
#define IOC_PARAMETERS_H_
#include "iocom.h"

#if IOC_DEVICE_PARAMETER_SUPPORT

struct iocSignal;

typedef struct iocParameterStorage
{
    os_int block_nr;
    os_boolean changed;
    os_timer ti;
}
iocParameterStorage;;

extern iocParameterStorage ioc_prm_storage;

/* Set parameter value by signal (used from communication callback)
 */
osalStatus ioc_set_parameter_by_signal(
    const struct iocSignal *sig,
    const struct iocSignal **pin_sig);


#endif
#endif
