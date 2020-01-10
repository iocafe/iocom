/**

  @file    frank_application.h
  @brief   Controller application base class.
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
  Frank application base class.
****************************************************************************************************
*/
class FrankApplication
{
public:
    /* Constructor and virtual destructor.
	 */
    FrankApplication();
    ~FrankApplication();

    /* Functions to start, stop and thread function to run the application.
     */
    void start(const os_char *network_name, os_uint device_nr);
    void stop();
    void run();

    /* Network topology stuff.
     */
    os_char m_controller_device_name[IOC_NAME_SZ];
    os_char m_network_name[IOC_NETWORK_NAME_SZ];
    os_uint m_controller_device_nr;

    /* Thread control.
     */
    osalEvent m_event;
    osalThreadHandle *m_thread;
    os_boolean m_stop_thread;
    os_boolean m_started;
};
