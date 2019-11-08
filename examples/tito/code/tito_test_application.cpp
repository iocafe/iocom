/**

  @file    tito_test_application.cpp
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


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoTestApplication::TitoTestApplication() : TitoApplication()
{
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoTestApplication::~TitoTestApplication()
{
}


void TitoTestApplication::start(const os_char *network_name, os_short device_nr)
{
    if (m_started) return;

    m_gina1_def = m_gina1.inititalize(1);
    m_gina2_def = m_gina2.inititalize(2);

    TitoApplication::start(network_name, device_nr);

    m_gina1.inititalize(1);
    m_gina2.inititalize(2);
    m_test_seq1.start(this);
}

void TitoTestApplication::stop()
{
    if (!m_started) return;

    TitoApplication::stop();
}

void TitoTestApplication::run()
{

    while (!m_stop_thread && osal_go())
    {
        osal_event_wait(m_event, OSAL_EVENT_INFINITE);
    }

    m_test_seq1.stop();
}
