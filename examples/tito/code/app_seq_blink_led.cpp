/**

  @file    app_seq_blink_led.cpp
  @brief   Some example sequence as own thread.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "controller_main.h"


/**
****************************************************************************************************

  @brief Constructor.

  Start the sequence as a new thread.
  @return  None.

****************************************************************************************************
*/
BlinkLedSequence::BlinkLedSequence() : AppSequence()
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
BlinkLedSequence::~BlinkLedSequence()
{
}


void BlinkLedSequence::start(ApplicationRoot *app)
{
    if (m_started) return;

    gina1 = app->m_gina1_def;
    gina2 = app->m_gina2_def;

    os_get_timer(&m_timer);
    m_led_on = OS_FALSE;

    AppSequence::start(app);
}


void BlinkLedSequence::stop()
{
    if (!m_started) return;
    AppSequence::stop();
}


/**
****************************************************************************************************

  @brief Run the sequence.

  If this is to run in both single thread and multithread mode, implement as state machine
  without sleeps.

  @return  None.

****************************************************************************************************
*/
void BlinkLedSequence::run()
{
    if (os_has_elapsed(&m_timer, 2000))
    {
        os_get_timer(&m_timer);
        m_led_on = !m_led_on;

        /* Blink IO ping on gina1 and gina2 boards.
         */
        ioc_sets_int(&gina1->imp.myoutput, m_led_on, OSAL_STATE_CONNECTED);
        ioc_sets_int(&gina2->imp.myoutput, m_led_on, OSAL_STATE_CONNECTED);

        /* Blink also local output pin */
        pin_set(&pins.outputs.led_builtin, m_led_on);
    }
}


/**
****************************************************************************************************

  @brief The thread function.

  This function should be overloaded by actual sequence.
  @return  None.

****************************************************************************************************
*/
void BlinkLedSequence::task()
{
    while (!m_stop_thread && osal_go())
    {
        run();
        os_timeslice();
    }
}

