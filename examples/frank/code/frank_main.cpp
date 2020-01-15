/**

  @file    frank_main.h
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
#include "frank.h"


/**
****************************************************************************************************
  Construct application.

  @param  publish List of IO device networks (user accounts) to be published.

****************************************************************************************************
*/
FrankMain::FrankMain(
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name,
    const os_char *publish)
{
    iocBServerParams prm;
    os_int i;

    /* Clear member variables.
     */
    for (i = 0; i < MAX_APPS; i++)
        m_app[i] = OS_NULL;

    /* Initialize signal structure for this device.
     */
    frank_init_signal_struct(&m_signals);

    os_memclear(&prm, sizeof(prm));
    prm.device_name = device_name;
    prm.device_nr = device_nr;
    prm.network_name = network_name;
    prm.signals_exp_hdr = &m_signals.exp.hdr;
    prm.signals_imp_hdr = &m_signals.imp.hdr;
    prm.signals_conf_exp_hdr = &m_signals.conf_exp.hdr;
    prm.signals_conf_imp_hdr = &m_signals.conf_imp.hdr;
    prm.signal_config = ioapp_signal_config;
    prm.signal_config_sz = sizeof(ioapp_signal_config);
    prm.network_defaults = ioapp_network_defaults;
    prm.network_defaults_sz = sizeof(ioapp_network_defaults);
    ioc_initialize_bserver(&m_bmain, &ioapp_root, &prm);

    /* Call basic server implementation macro to set up control stream.
     */
    IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(m_bmain, m_signals)

    /* Publish IO networks hosted by frank, such as "iocafenet" or "asteroidnet"
     */
    ioc_publish_bserver_networks(&m_bmain, publish);
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

    ioc_release_bserver(&m_bmain);
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

    const osalStreamInterface *iface = OSAL_TLS_IFACE;

    ep = ioc_initialize_end_point(OS_NULL, &ioapp_root);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = iface;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS; /* Notice IOC_DYNAMIC_MBLKS */
//    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS|IOC_BIDIRECTIONAL_MBLKS;
    ioc_listen(ep, &epprm);

    os_sleep(100);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************
  Or start thread which connects to client.
****************************************************************************************************
*/
osalStatus FrankMain::connect_to_device()
{
    iocConnection *con = OS_NULL;
    iocConnectionParams conprm;

    const osalStreamInterface *iface = OSAL_TLS_IFACE;

    con = ioc_initialize_connection(OS_NULL, &ioapp_root);
    os_memclear(&conprm, sizeof(conprm));

    conprm.iface = iface;
    conprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS;
    // conprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS|IOC_BIDIRECTIONAL_MBLKS;
    conprm.parameters = "127.0.0.1";
    ioc_connect(con, &conprm);

    os_sleep(100);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************
  Keep the control stream alive.
****************************************************************************************************
*/
void FrankMain::run()
{
    os_int i;

    /* Call basic server implementation to maintain control streams.
     */
    ioc_run_bserver_main(&m_bmain);

    /* Run applications.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (m_app[i])
        {
            m_app[i]->run();
        }
    }
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
        if (m_app[i])
        {
            if (!os_strcmp(network_name, m_app[i]->network_name()))
                return;
        }
    }

    /* Launc app.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (m_app[i] == OS_NULL)
        {
            m_app[i] = new FrankApplication(network_name);
            return;
        }
    }

    osal_debug_error("Too many franks");
}


