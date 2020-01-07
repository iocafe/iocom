/**

  @file    frank_application.cpp
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
#include "frank.h"

static void frank_application_thread_func(void *prm, osalEvent done);


/**
****************************************************************************************************

  @brief Application constructor.

  @return  None.

****************************************************************************************************
*/
FrankApplication::FrankApplication()
{
    m_event = osal_event_create();
}


/**
****************************************************************************************************

  @brief Application destructor.

  Join worker thread to this thread and clean up.

  @return  None.

****************************************************************************************************
*/
FrankApplication::~FrankApplication()
{
    stop();
    osal_event_delete(m_event);
}


void FrankApplication::start(const os_char *network_name, os_uint device_nr)
{
    os_strncpy(m_controller_device_name, "frank", IOC_NAME_SZ);
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);
    m_controller_device_nr = device_nr;

    m_stop_thread = OS_FALSE;
    m_thread = osal_thread_create(frank_application_thread_func, this,
        OS_NULL, OSAL_THREAD_ATTACHED);

    m_started = OS_TRUE;
}

void FrankApplication::stop()
{
    if (!m_started) return;

    m_stop_thread = OS_TRUE;
    osal_event_set(m_event);
    osal_thread_join(m_thread);
    m_started = OS_FALSE;
}

void FrankApplication::run()
{
    os_uint i = 0;
    os_char segments[8], buf[32], state_bits;
    os_float floats[5];
    os_timer ti;

    // os_long v;
    iocSignal *seven_segment = OS_NULL;
    iocSignal *float_test = OS_NULL;
    iocSignal *str_test = OS_NULL;
    iocSignal *c_test = OS_NULL;
    iocSignal *strtodevice = OS_NULL;

    os_memclear(segments, sizeof(segments));
    os_get_timer(&ti);

    while (!m_stop_thread && osal_go())
    {
        os_sleep(500);
        segments[i] = !segments[i];
        if (++i >= sizeof(segments)) i = 0;

        // ioc_maintain_signal(&ioapp_root, "seven_segment", m_network_name, &seven_segment);
        // ioc_sets_array(seven_segment, segments, sizeof(segments));

        if (os_elapsed(&ti, 2000))
        {
            os_get_timer(&ti);

            ioc_maintain_signal(&ioapp_root, "frd_select", m_network_name, &c_test);
            ioc_sets0_int(c_test, i+10);
        }

#if 0
            ioc_maintain_signal(&ioapp_root, "teststr", m_network_name, &str_test);
            ioc_gets_str(str_test, buf, sizeof(buf));
            os_strncat(buf, "Mighty", sizeof(buf));
            buf[3] = i;
            ioc_maintain_signal(&ioapp_root, "strtodevice", m_network_name, &strtodevice);
            ioc_sets_str(strtodevice, buf);
        }

        ioc_maintain_signal(&ioapp_root, "testfloat", m_network_name, &float_test);
        ioc_gets_array(float_test, floats, sizeof(floats)/sizeof(os_float));

        ioc_maintain_signal(&ioapp_root, "C", m_network_name, &c_test);
        /* v = */ ioc_gets_int(c_test, &state_bits, IOC_SIGNAL_DEFAULT);
#endif
    }

    ioc_delete_signal(seven_segment);
    ioc_delete_signal(float_test);
    ioc_delete_signal(str_test);
    ioc_delete_signal(c_test);
    ioc_delete_signal(strtodevice);
}

static void frank_application_thread_func(void *prm, osalEvent done)
{
    FrankApplication *app = (FrankApplication*)prm;
    osal_event_set(done);
    app->run();
}



