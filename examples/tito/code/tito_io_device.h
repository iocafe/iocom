/**

  @file    tito_io_device.h
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

/**
****************************************************************************************************
  IO device interface wrapper class.
****************************************************************************************************
*/
class TitoIoDevice
{
public:
    /* Constructor.
	 */
    TitoIoDevice(const os_char *device_name, os_short device_nr);

	/* Virtual destructor.
 	 */
    virtual ~TitoIoDevice();
};