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
FrankMain::FrankMain(
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name)
{
    ioc_initialize_bserver_main(&m_bmain, &ioapp_root, device_name, device_nr, network_name);

    ioc_initialize_bserver_accounts(&m_baccts, &ioapp_root, network_name);

    os_int i;

    for (i = 0; i < MAX_APPS; i++)
        m_app[i] = OS_NULL;

    setup_mblks();
    inititalize_accounts();
    setup_ctrl_stream();
    setup_accounts_ctrl_stream();
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

    ioc_release_bserver_main(&m_bmain);
    ioc_release_bserver_accounts(&m_baccts);
}


/* Set up memory blocks and signals.
 */
void FrankMain::setup_mblks()
{
    /* Initialize signal structure for this device.
     */
    frank_init_signal_struct(&m_signals);

    /* Call basic server implementation to do the rest of memory
       block signal setup.
     */
    ioc_setup_bserver_mblks(&m_bmain,
        &m_signals.exp.hdr,
        &m_signals.imp.hdr,
        &m_signals.conf_exp.hdr,
        &m_signals.conf_imp.hdr,
        ioapp_signal_config,
        sizeof(ioapp_signal_config),
        ioapp_network_defaults,
        sizeof(ioapp_network_defaults));
}


void FrankMain::inititalize_accounts()
{
    /* Setup signal structure structure for accounts.
     */
    accounts_init_signal_struct(&m_accounts);

    /* Call basic server implementation to do the rest of accounts setup.
     */
    ioc_setup_bserver_accounts(&m_baccts,
        &m_accounts.conf_exp.hdr,
        &m_accounts.conf_imp.hdr,
        ioapp_account_config,
        sizeof(ioapp_account_config),
        ioapp_account_defaults,
        sizeof(ioapp_account_defaults));
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
  Setup control stream parameters to configure this IO network node.
****************************************************************************************************
*/
void FrankMain::setup_ctrl_stream()
{
    /* Call basic server implementation macro to set up control stream.
     */
    IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(m_bmain, m_signals)
}

/**
****************************************************************************************************
  X
****************************************************************************************************
*/
void FrankMain::setup_accounts_ctrl_stream()
{
    /* Call basic server implementation macro to set up control stream.
     */
    IOC_SETUP_BSERVER_ACCOUNTS_STREAM_MACRO(m_baccts, m_accounts)
}


/**
****************************************************************************************************
  Keep the control stream alive.
****************************************************************************************************
*/
void FrankMain::run()
{
    /* Call basic server implementation to maintain control streams.
     */
    ioc_run_bserver_main(&m_bmain);

    ioc_run_bserver_accounts(&m_baccts);
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
            if (!os_strcmp(network_name, m_app[i]->m_network_name)) return;
        }
    }

    /* Launc app.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (m_app[i] == OS_NULL)
        {
            m_app[i] = new FrankApplication();
            m_app[i]->start(network_name, 1);
            return;
        }
    }

    osal_debug_error("Too many IO networks");
}


