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

class TitoTestApplication;

/**
****************************************************************************************************

  Application instance running one IO network.

****************************************************************************************************
*/
class TitoSequence
{
public:
    /* Constructor and virtual destructor.
	 */
    TitoSequence();
    virtual ~TitoSequence();

    virtual void start(TitoTestApplication *app);
    virtual void stop();
    virtual void run() {};

    gina_t *gina1;
    gina_t *gina2;

    osalEvent m_event;
    osalThreadHandle *m_thread;
    os_boolean m_stop_thread;
    os_boolean m_started;
};
