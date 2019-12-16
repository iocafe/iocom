/**

  @file    frank_get_netconf.h
  @brief   Get IO device's network configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    9.12.2019

  Copyright 2012 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


/**
****************************************************************************************************
  Class to get network configuration in separate thread.
****************************************************************************************************
*/
class FrankGetNetConf
{
public:
    /* Constructor and virtual destructor.
	 */
    FrankGetNetConf();
    ~FrankGetNetConf();

    /* Functions to start, stop and thread function to run the application.
     */
    void start(const os_char *device_name, os_uint device_nr, const os_char *network_name);
    void stop();
    void run();
    void run_write();

    /* Which IO device?
     */
    os_char m_device_name[IOC_NAME_SZ];
    os_uint m_device_nr;
    os_char m_network_name[IOC_NETWORK_NAME_SZ];

    /* Thread control.
     */
    osalEvent m_event;
    osalThreadHandle *m_thread;
    os_boolean m_stop_thread;
    os_boolean m_started;
};
