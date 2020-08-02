/**

  @file    sequence_base_class.cpp
  @brief   Sequence base class.
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

#if OSAL_MULTITHREAD_SUPPORT
static void buster_test_sequence_thread_func(void *prm, osalEvent done);
#endif

/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
AppSequence::AppSequence()
{
#if OSAL_MULTITHREAD_SUPPORT
    m_event = osal_event_create();
#endif
    m_started = OS_FALSE;
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
AppSequence::~AppSequence()
{
    stop();
    osal_event_delete(m_event);
}


void AppSequence::start(ApplicationRoot *app)
{
    if (m_started) return;

    /* Start running test_sequence for this IO device network in own thread.
     */
    m_stop_thread = OS_FALSE;
#if OSAL_MULTITHREAD_SUPPORT
    m_thread = osal_thread_create(buster_test_sequence_thread_func, this,
        OS_NULL, OSAL_THREAD_ATTACHED);
#endif
    m_started = OS_TRUE;
}


/* Join worker thread to this thread.
 */
void AppSequence::stop()
{
    if (!m_started) return;

    m_stop_thread = OS_TRUE;
#if OSAL_MULTITHREAD_SUPPORT
    osal_event_set(m_event);
    osal_thread_join(m_thread);
#endif
    m_started = OS_FALSE;
}


#if OSAL_MULTITHREAD_SUPPORT
static void buster_test_sequence_thread_func(void *prm, osalEvent done)
{
    AppSequence *seq = (AppSequence*)prm;
    osal_event_set(done);
    seq->task();
}
#endif
