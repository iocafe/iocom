/**

  @file    blink_sequence.h
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
#pragma once
#ifndef IOC_BLINK_SEQUENCE_H_
#define IOC_BLINK_SEQUENCE_H_
#include "buster.h"

/**
****************************************************************************************************

  X...

****************************************************************************************************
*/
class BlinkLedSequence : public AbstractSequence
{
public:
    /* Constructor and virtual destructor.
     */
    BlinkLedSequence();
    virtual ~BlinkLedSequence();

    os_timer m_timer;
    os_boolean m_led_on;

    minion_t *minion1;

    virtual void start(AbstractApplication *app);
    virtual void stop();
    virtual void run(os_timer *ti);
    virtual void task();
};

#endif
