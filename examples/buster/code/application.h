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


/**
****************************************************************************************************

  Buster main object.

****************************************************************************************************
*/
class Application : public AbstractApplication
{
public:
    /* Constructor and destructor.
     */
//    Application() {};

    /* Functions to start, stop and thread function to run the application.
     */
    void start(os_int argc, const os_char *argv[]);
    void stop();
    osalStatus run(os_timer *ti);

    /* Structure holding signals for the Buster.
     */
    buster_t m_signals;

    /* Basic server (ioserver extension) structure.
     */
    iocBServer m_bmain;

    Minion m_minion1;
    minion_t *m_minion1_def;

    BlinkLedSequence m_test_seq1;
};
