/**

  @file    tito_io_device.h
  @brief   Wrapper representing IO device interface.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

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
class TitoIoDevice
{
public:
    /* Constructor and virtual destructor.
	 */
    TitoIoDevice(TitoTestApplication());
    virtual ~TitoIoDevice();

    virtual void release () {};

    os_char m_device_name[IOC_NAME_SZ];
    os_short m_device_nr;
};
