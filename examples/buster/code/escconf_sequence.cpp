/**

  @file    escconf_sequence_sequence.cpp
  @brief   Configure BLHelo littlebee 30A brushless motor control ESC.
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
EscConfSequence::EscConfSequence() : AbstractSequence()
{
    m_set_min_throttle = 0;
    m_set_min_throttle_state_bits = 0;
    m_min_throttle_pressed = OS_FALSE;
    m_min_throttle_timer = 0;

    m_set_max_throttle = 0;
    m_set_max_throttle_state_bits = 0;
    m_max_throttle_pressed = OS_FALSE;
    m_max_throttle_timer = 0;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  Join worker thread to this thread and clean up.
  @return  None.

****************************************************************************************************
*/
EscConfSequence::~EscConfSequence()
{
}


void EscConfSequence::start(AbstractApplication *app)
{
    if (m_started) return;

    m_set_min_throttle = 0;
    m_set_min_throttle_state_bits = 0;
    m_min_throttle_pressed = OS_FALSE;

    m_set_max_throttle = 0;
    m_set_max_throttle_state_bits = 0;
    m_max_throttle_pressed = OS_FALSE;

    AbstractSequence::start(app);
}


void EscConfSequence::stop()
{
    if (!m_started) return;
    AbstractSequence::stop();
}


/**
****************************************************************************************************

  @brief Run the sequence.

  If this is to run in both single thread and multithread mode, implement as state machine
  without sleeps.

  @return  None.

****************************************************************************************************
*/
void EscConfSequence::run(os_timer *ti)
{
    os_long value;
    os_char state_bits;

    value = ioc_get_ext(&buster.imp.set_min_throttle, &state_bits, IOC_SIGNAL_DEFAULT);
    if (value != m_set_min_throttle || state_bits != m_set_min_throttle_state_bits) {
        m_set_min_throttle_state_bits = state_bits;
        m_set_min_throttle = value;

        if (value && !m_min_throttle_pressed && (state_bits & OSAL_STATE_CONNECTED)) {
            os_get_timer(&m_min_throttle_timer);
            m_min_throttle_pressed = OS_TRUE;
            ioc_set_double(&buster.exp.throttle, -100);
        }
    }

    if (m_min_throttle_pressed) {
        if (os_has_elapsed(&m_min_throttle_timer, 2000)) {
            ioc_set(&buster.exp.min_throttle, OS_FALSE);
            m_min_throttle_pressed = OS_FALSE;
        }
    }

    value = ioc_get_ext(&buster.imp.set_max_throttle, &state_bits, IOC_SIGNAL_DEFAULT);
    if (value != m_set_max_throttle || state_bits != m_set_max_throttle_state_bits) {
        m_set_max_throttle_state_bits = state_bits;
        m_set_max_throttle = value;

        if (value && !m_max_throttle_pressed && (state_bits & OSAL_STATE_CONNECTED)) {
            os_get_timer(&m_max_throttle_timer);
            m_max_throttle_pressed = OS_TRUE;
            ioc_set_double(&buster.exp.throttle, 100);
        }
    }

    if (m_max_throttle_pressed) {
        if (os_has_elapsed(&m_max_throttle_timer, 2000)) {
            ioc_set(&buster.exp.max_throttle, OS_FALSE);
            m_max_throttle_pressed = OS_FALSE;
        }
    }
}


/**
****************************************************************************************************

  @brief The thread function.

  This function should be overloaded by actual sequence.
  @return  None.

****************************************************************************************************
*/
void EscConfSequence::task()
{
    os_timer ti;
    while (!m_stop_thread && osal_go())
    {
// static long ulledoo; if (++ulledoo > 10009) {osal_debug_error("ulledoo blink\n"); ulledoo = 0;}
        os_get_timer(&ti);
        run(&ti);
        os_sleep(50);
    }
}

