/**

  @file    deviceinfo_system_specs.c
  @brief   Publish software versions, used operating system and hardware.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Publish software version, operating system, architecture and IO device hardware information.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "deviceinfo.h"

/**
****************************************************************************************************

  @brief Publish specification in memory block signals.

  Set values for software version, operating system, architecture and IO device hardware
  information. signals.

  @param   sigs Structure containing pointers to system specfication related signals.
  @param   hw IO device hardware from string. This can be define based on JSON configuration,
           like CANDY_HW.
  @return  None

****************************************************************************************************
*/
void dinfo_set_system_specs(
    dinfoSystemSpeSignals *sigs,
    os_char *hw)
{
    ioc_set_str(sigs->sig[IOC_DINFO_SI_PACKAGE], OSAL_BUILD_DATETIME);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_EOSAL], EOSAL_VERSION);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_IOCOM], IOCOM_VERSION);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_OS], OSAL_OS_NAME);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_OSVER], OSAL_OSVER);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_ARCH], OSAL_ARCH);
    ioc_set_str(sigs->sig[IOC_DINFO_SI_HW], hw);
}


