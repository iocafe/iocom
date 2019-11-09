/**

  @file    tito_test_sequence1.h
  @brief   Some example sequence as own thread.
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
class TitoTestSequence1 : public TitoSequence
{
public:
    /* Constructor and virtual destructor.
     */
    TitoTestSequence1();
    virtual ~TitoTestSequence1();

    virtual void start(TitoTestApplication *app);
    virtual void stop();
    virtual void run();
};