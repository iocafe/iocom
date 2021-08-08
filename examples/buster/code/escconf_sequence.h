/**

  @file    escconf_sequence.h
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
#pragma once
#ifndef IOC_ESCCONF_SEQUENCE_H_
#define IOC_ESCCONF_SEQUENCE_H_
#include "buster.h"

/**
****************************************************************************************************

  X...

****************************************************************************************************
*/
class EscConfSequence : public AbstractSequence
{
public:
    /* Constructor and virtual destructor.
     */
    EscConfSequence();
    virtual ~EscConfSequence();

    os_long m_set_min_throttle;
    os_boolean m_set_min_throttle_state_bits;
    os_boolean m_min_throttle_pressed;
    os_timer m_min_throttle_timer;

    os_long m_set_max_throttle;
    os_boolean m_set_max_throttle_state_bits;
    os_boolean m_max_throttle_pressed;
    os_timer m_max_throttle_timer;

    virtual void start(AbstractApplication *app);
    virtual void stop();
    virtual void run(os_timer *ti);
    virtual void task();
};

#endif
