/**

  @file    tito_sequence.cpp
  @brief   Sequence base class.
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

static void tito_test_sequence_thread_func(void *prm, osalEvent done);


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoSequence::TitoSequence(TitoApplication *app)
{
    //gina1 = app->gina1;
    // gina2 = app->gina2;

    /* Start running test_sequence for this IO device network in own thread.
     */
    m_event = osal_event_create();
    m_stop_thread = OS_FALSE;
    m_thread = osal_thread_create(tito_test_sequence_thread_func, this,
        OSAL_THREAD_ATTACHED, 0, app->m_network_name);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoSequence::~TitoSequence()
{
    /* Join worker thread to this thread.
     */
    m_stop_thread = OS_TRUE;
    osal_event_set(m_event);
    osal_thread_join(m_thread);

    osal_event_delete(m_event);
}


void TitoSequence::run()
{
    while (!m_stop_thread && osal_go())
    {
        osal_event_wait(m_event, OSAL_EVENT_INFINITE);
    }
}


static void tito_test_sequence_thread_func(void *prm, osalEvent done)
{
    TitoSequence *seq = (TitoSequence*)prm;
    osal_event_set(done);
    seq->run();
}
