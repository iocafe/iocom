/**

  @file    app_instance.cpp
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
#include "app_main.h"


/**
****************************************************************************************************
  Application constructor.
****************************************************************************************************
*/
AppInstance::AppInstance(const os_char *network_name)
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
AppInstance::~AppInstance()
{
    ioc_delete_signal(m_float_test);
    ioc_delete_signal(m_str_test);
    ioc_delete_signal(m_str_to_device);
}


/**
****************************************************************************************************
  Keep the application alive, called repeatedly.

  return If working in something, the function returns OSAL_SUCCESS. Return value
         OSAL_NOTHING_TO_DO indicates that this thread can be switched to slow
         idle mode as far as the application instance knows.

****************************************************************************************************
*/
osalStatus AppInstance::run()
{
    os_char buf[32];
    os_float floats[5];

    if (os_has_elapsed(&m_timer, 2000))
    {
        os_get_timer(&m_timer);
        ioc_maintain_signal(&app_iocom_root, "teststr", m_network_name, &m_str_test);
        ioc_gets_str(m_str_test, buf, sizeof(buf));
        os_strncat(buf, "Mighty", sizeof(buf));
        buf[3] = '0' + (os_char)m_count;
        if (++m_count > 9) m_count = 0;
        ioc_maintain_signal(&app_iocom_root, "strtodevice", m_network_name, &m_str_to_device);
        // ioc_sets_str(m_str_to_device, buf);

        ioc_maintain_signal(&app_iocom_root, "testfloat", m_network_name, &m_float_test);
        ioc_gets_array(m_float_test, floats, sizeof(floats)/sizeof(os_float));
    }

    return OSAL_NOTHING_TO_DO;
}



