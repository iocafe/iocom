/**

  @file    application.h
  @brief   Buster application's main class.
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
#ifndef IOC_APPLICATION_H_
#define IOC_APPLICATION_H_
#include "buster.h"


/* Global signals. This allows mapping IO pins directly to signals from JSON, but we can have only
   one application instance. OSAL_C_HEADER_BEGINS/OSAL_C_HEADER_ENDS is used to keep the global
   symbol same in C and C++.
 */
OSAL_C_HEADER_BEGINS
extern struct buster_t buster;
OSAL_C_HEADER_ENDS


/**
****************************************************************************************************

  Buster main object.

****************************************************************************************************
*/
class Application : public AbstractApplication
{
public:
    /* Functions to start, stop and thread function to run the application.
     */
    void start(os_int argc, const os_char *argv[]);
    void stop();
    osalStatus run(os_timer *ti);

    void steering(
        os_timer *ti);

    void set_angle_to_range(
        os_double *d);

    virtual void communication_callback_1(
        struct iocHandle *handle,
        os_int start_addr,
        os_int end_addr,
        os_ushort flags);

    /* Structure holding signals for the Buster.
     */
    buster_t *m_signals;

    /* Device information (nc = network configuration, rm resource monitor). */
    dinfoNodeConfState m_dinfo_nc;
    dinfoResMonState m_dinfo_rm;

    /* Basic server (ioserver extension) structure.
     */
    iocBServer m_bmain;

/* Blink LED - morse code to indicate network status */
#if IOCOM_USE_MORSE
    MorseCode m_morse;
#endif

    os_timer m_analogs_timer;
    os_timer m_gamecontroller_timer;
    os_ushort m_gamecontroller_alive;

    Minion m_minion1;
    minion_t *m_minion1_def;

    BlinkLedSequence m_test_seq1;

#if PINS_CAMERA
    Camera m_camera1;
#endif

};

#endif
