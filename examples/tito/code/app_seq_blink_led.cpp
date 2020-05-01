/**

  @file    app_seq_blink_led.cpp
  @brief   Some example sequence as own thread.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

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


void BlinkLedSequence::start(AppInstance *app)
{
    if (m_started) return;

    gina1 = app->m_gina1_def;
    gina2 = app->m_gina2_def;
    AppSequence::start(app);
}


void BlinkLedSequence::stop()
{
    if (!m_started) return;
    AppSequence::stop();
}


/**
****************************************************************************************************

  @brief The thread function.

  This function should be overloaded by actual sequence.
  @return  None.

****************************************************************************************************
*/
void BlinkLedSequence::run()
{
/*
    os_boolean led_on = OS_TRUE;
    os_char state_bits;
    os_int dip, elap, touch_sensor, brig = 1, up = 1;
    os_timer end_t, start_t;

    os_get_timer(&start_t);
*/
    while (!m_stop_thread && osal_go())
    {
#if 0
        if (os_has_elapsed(&start_t, 50))
        {
            touch_sensor = ioc_gets_int(&gina2->exp.dip_switch_3, &state_bits, IOC_SIGNAL_DEFAULT);
            //osal_trace_int("touch = ", touch_sensor);

            if (up) brig *= 2;
            else brig /= 2;
            if (brig > 4090) brig = 4090;
            if (touch_sensor) brig = 1;
            ioc_sets_int(&gina2->imp.dimmer_led, brig, OSAL_STATE_CONNECTED);
            ioc_sets_int(&gina1->imp.dimmer_led, brig, OSAL_STATE_CONNECTED);
            if (brig >= 4090) up = 0;
            if (brig == 0) {up = 1; brig = 1;}

            os_get_timer(&start_t);
            led_on = !led_on;
            // if (touch_sensor > 0) led_on = OS_FALSE;
            ioc_sets_int(&gina2->imp.led_builtin, led_on, OSAL_STATE_CONNECTED);
            ioc_sets_int(&gina1->imp.led_builtin, led_on, OSAL_STATE_CONNECTED);
        }


        /* os_get_timer(&start_t);
        ioc_sets_int(&gina2->imp.led_builtin, led_on, OSAL_STATE_CONNECTED);
        do {
            os_timeslice();
            if (os_has_elapsed(&start_t, 1000)) break;
            dip = ioc_gets_int(&gina2->exp.dip_switch_3, &state_bits);
        } while (dip != led_on);

        os_get_timer(&end_t);
        elap = (end_t - start_t);



        osal_trace_int("elap = ", elap);
        osal_trace_int("led = ", led_on);
        osal_trace_int("dip = ", dip);
        os_sleep(500);
        led_on = !led_on;
        */
#endif
        os_timeslice();
    }
}


