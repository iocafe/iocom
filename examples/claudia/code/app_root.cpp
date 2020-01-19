/**

  @file    app_root.cpp
  @brief   IO application's root class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  The root class starts and runs basic server code from ioserver extension library. This
  provides basic functionality like ability to connect to this application and configure it,
  set up IO networks and user accounts, etc.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "app_main.h"


/**
****************************************************************************************************
  Construct application.

  @param  publish List of IO device networks (user accounts) to be published.

****************************************************************************************************
*/
AppRoot::AppRoot(
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
    claudia_init_signal_struct(&m_signals);

    os_memclear(&prm, sizeof(prm));
    prm.is_cloud_server = OS_TRUE;
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
    prm.account_defaults = ioapp_account_defaults;
    prm.account_defaults_sz = sizeof(ioapp_account_defaults);


    ioc_initialize_bserver(&m_bmain, &app_iocom, &prm);

    /* Call basic server implementation macro to set up control stream.
     */
    IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(m_bmain, m_signals)

    /* Publish IO networks hosted by claudia, such as "iocafenet" or "asteroidnet"
     */
    ioc_publish_bserver_networks(&m_bmain, publish);
}


/**
****************************************************************************************************
  Application destructor.
****************************************************************************************************
*/
AppRoot::~AppRoot()
{
    os_int i;

    /* Finish with 'claudia' applications.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        delete m_app[i];
    }

    ioc_release_bserver(&m_bmain);
}


/**
****************************************************************************************************
  Keep basic server and application instances alive.

  return If working in something, the function returns OSAL_SUCCESS. Return value
         OSAL_STATUS_NOTHING_TO_DO indicates that this thread can be switched to slow
         idle mode as far as the application root knows.
****************************************************************************************************
*/
osalStatus AppRoot::run()
{
    os_int i;
    osalStatus s;

    /* Call basic server implementation to maintain control streams.
     */
    s = ioc_run_bserver(&m_bmain);

    /* Run applications.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (m_app[i])
        {
            if (m_app[i]->run() != OSAL_STATUS_NOTHING_TO_DO)
                s = OSAL_SUCCESS;
        }
    }

    return s;
}


/**
****************************************************************************************************
  Launch an application instance.
****************************************************************************************************
*/
void AppRoot::launch_app(
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
            m_app[i] = new AppInstance(network_name);
            return;
        }
    }

    osal_debug_error("Too many claudias");
}


