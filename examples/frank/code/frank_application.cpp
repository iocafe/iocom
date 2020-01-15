/**

  @file    frank_application.cpp
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
#include "frank.h"


/**
****************************************************************************************************
  Application constructor.
****************************************************************************************************
*/
FrankApplication::FrankApplication(const os_char *network_name)
{
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);
    m_float_test = m_str_test = m_str_to_device = OS_NULL;
    m_count = 0;
    os_get_timer(&m_timer);
}


/**
****************************************************************************************************
  Application destructor.
****************************************************************************************************
*/
FrankApplication::~FrankApplication()
{
    ioc_delete_signal(m_float_test);
    ioc_delete_signal(m_str_test);
    ioc_delete_signal(m_str_to_device);
}


/**
****************************************************************************************************
  Keep the application alive, called repeatedly.
****************************************************************************************************
*/
void FrankApplication::run()
{
    os_char buf[32];
    os_float floats[5];

    if (os_elapsed(&m_timer, 2000))
    {
        os_get_timer(&m_timer);
        ioc_maintain_signal(&ioapp_root, "teststr", m_network_name, &m_str_test);
        ioc_gets_str(m_str_test, buf, sizeof(buf));
        os_strncat(buf, "Mighty", sizeof(buf));
        buf[3] = '0' + (os_char)m_count;
        if (++m_count > 9) m_count = 0;
        ioc_maintain_signal(&ioapp_root, "strtodevice", m_network_name, &m_str_to_device);
        ioc_sets_str(m_str_to_device, buf);

        ioc_maintain_signal(&ioapp_root, "testfloat", m_network_name, &m_float_test);
        ioc_gets_array(m_float_test, floats, sizeof(floats)/sizeof(os_float));
    }
}



