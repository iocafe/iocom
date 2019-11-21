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


void FrankApplication::start(const os_char *network_name, os_short device_nr)
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
    while (!m_stop_thread && osal_go())
    {
        osal_event_wait(m_event, OSAL_EVENT_INFINITE);
    }
}

static void frank_application_thread_func(void *prm, osalEvent done)
{
    FrankApplication *app = (FrankApplication*)prm;
    osal_event_set(done);
    app->run();
}



