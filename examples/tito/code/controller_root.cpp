/**

  @file    controller_root.cpp
  @brief   Root class for Tito application.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  There can be only one instance of the root object.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "controller_main.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
ControllerRoot::ControllerRoot()
{
    AppInstance *app;

    /* Lauch tour 'tito' applications, one for iocafenet, two for asteroidnet.
     */
    m_nro_apps = 0;
    app = new AppInstance();
    app->start("iocafenet", 1);
    m_app[m_nro_apps++] = app;

    /* app = new AppInstance();
    app->start("asteroidnet", 1);
    m_app[m_nro_apps++] = app;

    app = new AppInstance();
    app->start("asteroidnet", 2);
    m_app[m_nro_apps++] = app; */

    osal_debug_assert(m_nro_apps <= MAX_APPS);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
ControllerRoot::~ControllerRoot()
{
    os_int i;

    /* Finish with 'tito' applications.
     */
    for (i = 0; i < m_nro_apps; i++)
    {
        delete m_app[i];
    }
}

osalStatus ControllerRoot::loop()
{
    os_int i;

    for (i = 0; i<m_nro_apps; i++) {
        m_app[i]->run();
    }

    return OSAL_SUCCESS;
}
