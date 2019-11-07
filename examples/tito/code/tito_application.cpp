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
    os_strncpy(m_controller_device_name, "tito", IOC_NAME_SZ);
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);
    m_controller_device_nr = device_nr;

    /* Set up static IO boards, for now two gina boards 1 and 2
     */
    m_nro_io_devices = 0;
    gina1 = m_io_device[m_nro_io_devices++] = new TitoIoDevice("gina", 1);
    gina2 = m_io_device[m_nro_io_devices++] = new TitoIoDevice("gina", 2);
    osal_debug_assert(m_nro_io_devices <= MAX_IO_DEVICES);

    /* Start running application for this IO device network in own thread.
     */
    m_event = osal_event_create();
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

    /* Finish with IO devices.
     */
    for (os_int i = 0; i < m_nro_io_devices; i++) delete m_io_device[i];

    osal_event_delete(m_event);
}


void TitoApplication::run()
{
    TitoTestSequence1 test_seq1(this);

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
