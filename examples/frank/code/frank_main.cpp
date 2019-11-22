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
  Construct application.
****************************************************************************************************
*/
FrankMain::FrankMain()
{
    os_int i;

    for (i = 0; i < MAX_APPS; i++)
        m_app[i] = OS_NULL;
}


/**
****************************************************************************************************
  Application destructor.
****************************************************************************************************
*/
FrankMain::~FrankMain()
{
    os_int i;

    /* Finish with 'frank' applications.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        delete m_app[i];
    }
}


/**
****************************************************************************************************
  Start thread which listens for client connections.
****************************************************************************************************
*/
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


/**
****************************************************************************************************
  Launc a client application.
****************************************************************************************************
*/
void FrankMain::launch_app(
    os_char *network_name)
{
    os_int i;

    /* If app is already running for this network.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (!os_strcmp(network_name, m_app[i]->m_network_name)) return;
    }

    /* Launc app.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (m_app[i] == OS_NULL)
        {
            m_app[i] = new FrankApplication();
            m_app[i]->start("surfnet", 1);
            return;
        }
    }

    osal_debug_error("Too many IO networks");
}
