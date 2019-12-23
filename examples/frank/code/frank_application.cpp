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
    os_char segments[8];
    os_float floats[5];
    iocSignal *seven_segment = OS_NULL;
    iocSignal *float_test  = OS_NULL;

    os_memclear(segments, sizeof(segments));

    while (!m_stop_thread && osal_go())
    {
        os_sleep(500);
        segments[i] = !segments[i];
        if (++i >= sizeof(segments)) i = 0;

        // ioc_maintain_signal(&frank_root, "seven_segment", m_network_name, &seven_segment);
        // ioc_sets_array(seven_segment, segments, sizeof(segments));

        ioc_maintain_signal(&frank_root, "testfloat", m_network_name, &float_test);
        ioc_gets_array(float_test, floats, sizeof(floats)/sizeof(os_float));
    }

    ioc_delete_signal(seven_segment);
}

static void frank_application_thread_func(void *prm, osalEvent done)
{
    FrankApplication *app = (FrankApplication*)prm;
    osal_event_set(done);
    app->run();
}



