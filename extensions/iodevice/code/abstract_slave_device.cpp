/**

  @file    abstract_slave_device.cpp
  @brief   Base class for an abstract slave device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iodevice.h"

using IoDevice::AbstractSlaveDevice;


/**
****************************************************************************************************
  Constructor.
****************************************************************************************************
*/
AbstractSlaveDevice::AbstractSlaveDevice()
{
    m_device_name[0] = '\0';
    m_device_nr = 0;
}


/**
****************************************************************************************************
  Virtual destructor.
****************************************************************************************************
*/
AbstractSlaveDevice::~AbstractSlaveDevice()
{
}
