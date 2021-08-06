/**

  @file    blink_sequence.cpp
  @brief   Some example sequence as own thread.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "buster.h"


/**
****************************************************************************************************

  @brief Constructor.

  Start the sequence as a new thread.
  @return  None.

****************************************************************************************************
*/
BlinkLedSequence::BlinkLedSequence() : AbstractSequence()
{
    minion1 = 0;
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


void BlinkLedSequence::start(AbstractApplication *app)
{
    if (m_started) return;

    /* gina1 = app->m_gina1_def;
    gina2 = app->m_gina2_def;

    os_get_timer(&m_timer);
    m_led_on = OS_FALSE;

    AppSequence::start(app); */
}


void BlinkLedSequence::stop()
{
    /* if (!m_started) return;
    AppSequence::stop(); */
}


/**
****************************************************************************************************

  @brief Run the sequence.

  If this is to run in both single thread and multithread mode, implement as state machine
  without sleeps.

  @return  None.

****************************************************************************************************
*/
void BlinkLedSequence::run(os_timer *ti)
{
    if (os_has_elapsed_since(&m_timer, ti, 2000))
    {
        m_timer = *ti;
        m_led_on = !m_led_on;

        /* Blink IO ping on gina1 and gina2 boards.
         */
        ioc_set_ext(&minion1->imp.set_headlight, m_led_on, OSAL_STATE_CONNECTED);

        /* Blink also local output pin (if we got one).
         */
#ifdef PINS_OUTPUTS_LED_BUILTIN
        pin_set(&pins.outputs.led_builtin, m_led_on);
#endif
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
    os_timer ti;
    while (!m_stop_thread && osal_go())
    {
// static long ulledoo; if (++ulledoo > 10009) {osal_debug_error("ulledoo blink\n"); ulledoo = 0;}
        os_get_timer(&ti);
        run(&ti);
        os_timeslice();
    }
}

