/**

  @file    sequence_base_class.h
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

class ApplicationRoot;

/**
****************************************************************************************************

  Application instance running one IO network.

****************************************************************************************************
*/
class AppSequence
{
public:
    /* Constructor and virtual destructor.
     */
    AppSequence();
    virtual ~AppSequence();

    virtual void start(ApplicationRoot *app);
    virtual void stop();
    virtual void run(os_timer *ti) {};
    virtual void task() {};

#if OSAL_MULTITHREAD_SUPPORT
    osalEvent m_event;
    osalThread *m_thread;
#endif

    os_boolean m_stop_thread;
    os_boolean m_started;
};
