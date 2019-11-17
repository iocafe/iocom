/**

  @file    tito_test_sequence1.cpp
  @brief   Some example sequence as own thread.
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

  Start the sequence as a new thread.
  @return  None.

****************************************************************************************************
*/
TitoTestSequence1::TitoTestSequence1() : TitoSequence()
{
    gina1 = 0;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  Join worker thread to this thread and clean up.
  @return  None.

****************************************************************************************************
*/
TitoTestSequence1::~TitoTestSequence1()
{
}


void TitoTestSequence1::start(TitoTestApplication *app)
{
    if (m_started) return;
    TitoSequence::start(app);
}


void TitoTestSequence1::stop()
{
    if (!m_started) return;
    TitoSequence::stop();
}


/**
****************************************************************************************************

  @brief The thread function.

  This function should be overloaded by actual sequence.
  @return  None.

****************************************************************************************************
*/
void TitoTestSequence1::run()
{
    os_boolean led_on = OS_FALSE;

    while (!m_stop_thread && osal_go())
    {
        led_on = !led_on;
        ioc_sets_int(&gina2->imp.led_builtin, led_on, OSAL_STATE_CONNECTED);
        os_sleep(300);
    }
}


