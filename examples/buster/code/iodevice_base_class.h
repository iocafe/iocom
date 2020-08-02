/**

  @file    iodevice_base_class.h
  @brief   Base class for a IO device class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/**
****************************************************************************************************
  IO device interface wrapper class.
****************************************************************************************************
*/
class AppIoDevice
{
public:
    /* Constructor and virtual destructor.
     */
    AppIoDevice() {};
    virtual ~AppIoDevice();

    virtual void release () {};

    os_char m_device_name[IOC_NAME_SZ];
    os_short m_device_nr;
};
