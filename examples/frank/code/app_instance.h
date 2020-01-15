/**

  @file    app_instance.h
  @brief   IO controller application's base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  The application instance class wraps actual application functionality and current state for one
  IO device network.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/


class AppInstance
{
public:
    AppInstance(const os_char *network_name);
    ~AppInstance();

    void run();

    os_char *network_name()
        {return m_network_name;}

private:
    os_char
        m_network_name[IOC_NETWORK_NAME_SZ];

    iocSignal
        *m_float_test,
        *m_str_test,
        *m_str_to_device;

    os_timer
        m_timer;

    os_int
        m_count;
};
