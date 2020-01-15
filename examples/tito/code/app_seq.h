/**

  @file    app_seq.h
  @brief   Sequence base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/

class AppInstance;

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

    virtual void start(AppInstance *app);
    virtual void stop();
    virtual void run() {};

    gina_t *gina1;
    gina_t *gina2;

    osalEvent m_event;
    osalThreadHandle *m_thread;
    os_boolean m_stop_thread;
    os_boolean m_started;
};
