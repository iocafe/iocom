/**

  @file    tito_application.h
  @brief   Controller application base class.
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
  Tito application base class.
****************************************************************************************************
*/
class TitoApplication
{
public:
    /* Constructor and virtual destructor.
	 */
    TitoApplication();
    virtual ~TitoApplication();

    /* Functions to start, stop and thread function to run the application.
     */
    virtual void start(const os_char *network_name, os_short device_nr) {};
    virtual void stop();
    virtual void run();

    /* Network topology stuff.
     */
    os_char m_controller_device_name[IOC_NAME_SZ];
    os_char m_network_name[IOC_NETWORK_NAME_SZ];
    os_short m_controller_device_nr;

    /* Thread control.
     */
    osalEvent m_event;
    osalThreadHandle *m_thread;
    os_boolean m_stop_thread;
    os_boolean m_started;

// protected:
    void initialize(const os_char *network_name, os_short device_nr);
    void startapp();
};
