/**

  @file    app_root.cpp
  @brief   Controller example with static IO defice configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "app_main.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
AppRoot::AppRoot()
{
    AppInstance *app;
    /* Lauch tour 'tito' applications, one for iocafenet, one for markkunet and two for surfnet.
     */
    m_nro_apps = 0;

    app = new AppInstance();
    app->start("iocafenet", 1);
    m_app[m_nro_apps++] = app;

    /* app = new AppInstance();
    app->start("markkunet", 1);
    m_app[m_nro_apps++] = app;

    app = new AppInstance();
    app->start("surfnet", 1);
    m_app[m_nro_apps++] = app;

    app = new AppInstance();
    app->start("surfnet", 2);
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
AppRoot::~AppRoot()
{
    os_int i;

    /* Finish with 'tito' applications.
     */
    for (i = 0; i < m_nro_apps; i++)
    {
        delete m_app[i];
    }
}


osalStatus AppRoot::listen_for_clients()
{
    iocEndPoint *ep = OS_NULL;
    iocEndPointParams epprm;

    const osalStreamInterface *iface = OSAL_TLS_IFACE;

    ep = ioc_initialize_end_point(OS_NULL, &app_iocom);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = iface;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD;
    ioc_listen(ep, &epprm);

    os_sleep(100);
    return OSAL_SUCCESS;
}


osalStatus AppRoot::loop()
{
    os_sleep(100);
    return OSAL_SUCCESS;
}

