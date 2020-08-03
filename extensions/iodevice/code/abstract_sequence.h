/**

  @file    abstract_sequence.h
  @brief   Abstract sequence base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

namespace IoDevice
{

    class AbstractApplication;

    /**
    ************************************************************************************************

      AbstractApplication instance running one IO network.

    ************************************************************************************************
    */
    class AbstractSequence
    {
    public:
        /* Constructor and virtual destructor.
         */
        AbstractSequence();
        virtual ~AbstractSequence();

        virtual void start(AbstractApplication *app);
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

}
