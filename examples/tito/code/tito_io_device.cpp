/**

  @file    tito_io_device.cpp
  @brief   Wrapper representing IO device interface.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.11.2019

  Copyright 2012 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

#include "tito.h"



/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoIoDevice::TitoIoDevice(const os_char *device_name, os_short device_nr)
{
    /* Save IO device network topology related stuff.
     */
    // os_strncpy(m_device_name, "tito", IOC_NAME_SZ);
    // os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);
    // m_device_nr = device_nr;

    /* Set up static IO boards, for now two gina boards 1 and 2
     */



}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoIoDevice::~TitoIoDevice()
{
}

