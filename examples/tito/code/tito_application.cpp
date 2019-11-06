/**

  @file    tito_application.cpp
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
#include "tito.h"

static void tito_application_thread_func(void *prm, osalEvent done);


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoApplication::TitoApplication(const os_char *network_name, os_short device_nr)
{
    /* Save IO device network topology related stuff.
     */
    os_strncpy(m_device_name, "tito", IOC_NAME_SZ);
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);
    m_device_nr = device_nr;

    /* Set up static IO boards, for now two gina boards 1 and 2
     */

    /* Start running application for this network in own thread.
     */
    osal_event_create();
    m_stop_thread = OS_FALSE;
    m_thread = osal_thread_create(tito_application_thread_func, this,
        OSAL_THREAD_ATTACHED, 0, network_name);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoApplication::~TitoApplication()
{
    /* Join worker thread to this thread.
     */
    m_stop_thread = OS_TRUE;
    osal_event_set(m_event);
    osal_thread_join(m_thread);
    osal_event_delete(m_event);
}


void TitoApplication::run()
{
    while (!m_stop_thread && osal_go())
    {
        osal_event_wait(m_event, OSAL_EVENT_INFINITE);
    }
}


static void tito_application_thread_func(void *prm, osalEvent done)
{
    TitoApplication *app = (TitoApplication*)prm;
    osal_event_set(done);
    app->run();
}
