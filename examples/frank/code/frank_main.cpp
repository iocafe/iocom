/**

  @file    frank_main.h
  @brief   Controller example with static IO defice configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.12.2011

  Copyright 2012 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "frank.h"


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
FrankMain::FrankMain()
{
    FrankApplication *app;
    /* Lauch tour 'frank' applications, one for pekkanet, one for markkunet and two for surfnet.
     */
    m_nro_apps = 0;

    app = new FrankApplication();
    app->start("pekkanet", 1);
    m_app[m_nro_apps++] = app;

    /* app = new FrankTestApplication();
    app->start("markkunet", 1);
    m_app[m_nro_apps++] = app;

    app = new FrankTestApplication();
    app->start("surfnet", 1);
    m_app[m_nro_apps++] = app;

    app = new FrankTestApplication();
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
FrankMain::~FrankMain()
{
    os_int i;

    /* Finish with 'frank' applications.
     */
    for (i = 0; i < m_nro_apps; i++)
    {
        delete m_app[i];
    }
}


osalStatus FrankMain::listen_for_clients()
{
    iocEndPoint *ep = OS_NULL;
    iocEndPointParams epprm;

    const osalStreamInterface *iface = OSAL_SOCKET_IFACE;

    ep = ioc_initialize_end_point(OS_NULL, &frank_root);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = iface;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS; /* Notice IOC_DYNAMIC_MBLKS */
    epprm.parameters = ":" IOC_DEFAULT_SOCKET_PORT_STR;
    ioc_listen(ep, &epprm);

    os_sleep(100);
    return OSAL_SUCCESS;
}
