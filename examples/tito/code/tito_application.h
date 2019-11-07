/**

  @file    tito_application.h
  @brief   Controller application running for one IO device network.
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

  Application instance running one IO network.

****************************************************************************************************
*/
class TitoApplication
{
public:
    /* Constructor.
	 */
    TitoApplication(const os_char *network_name, os_short device_nr);

	/* Virtual destructor.
 	 */
    virtual ~TitoApplication();

    virtual void run();

    os_char m_controller_device_name[IOC_NAME_SZ];
    os_char m_network_name[IOC_NETWORK_NAME_SZ];
    os_short m_controller_device_nr;

    static const os_int MAX_IO_DEVICES = 20;
    os_int m_nro_io_devices;
    class TitoIoDevice *m_io_device[MAX_IO_DEVICES];

    osalEvent m_event;
    osalThreadHandle *m_thread;
    os_boolean m_stop_thread;
};
