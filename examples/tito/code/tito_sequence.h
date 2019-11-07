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

/**
****************************************************************************************************

  Application instance running one IO network.

****************************************************************************************************
*/
class TitoSequence
{
public:
    /* Constructor.
	 */
    TitoSequence(TitoApplication *app);

	/* Virtual destructor.
 	 */
    virtual ~TitoSequence();

    virtual void run();

    osalEvent m_event;
    osalThreadHandle *m_thread;
    os_boolean m_stop_thread;
};
