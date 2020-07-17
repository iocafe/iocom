/**

  @file    deviceinfo_system_specs.c
  @brief   Publish software versions, used operating system and hardware.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "deviceinfo.h"


/**
****************************************************************************************************

  @brief X

  X

  @param   X
  @return  X

****************************************************************************************************
*/
void dinfo_set_system_specs(
    dinfoSystemSpeSignals *sigs)
{
    ioc_set_str(sigs->sig[IOC_DINFO_SI_PACKAGE], OSAL_BUILD_DATETIME);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_EOSAL], EOSAL_VERSION);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_IOCOM], IOCOM_VERSION);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_OS], OSAL_OS_NAME);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_ARCH], OSAL_ARCH_NAME);
    // ioc_set_str(sigs->sig[IOC_DINFO_SI_HW], MY_HW_NAME);
}


