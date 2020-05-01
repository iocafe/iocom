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
ControllerRoot::ControllerRoot(
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name,
    const os_char *publish)
{
    AppInstance *app;
    iocBServerParams prm;

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

    /* Initialize signal structure for this device.
     */
    tito_init_signal_struct(&m_signals);

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
    ioc_initialize_ioserver(&m_bmain, &iocom_root, &prm);

    /* Call basic server implementation macro to set up control stream.
     */
    IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(m_bmain, m_signals)

    /* Publish IO networks hosted by frank, such as "iocafenet" or "asteroidnet"
     */
    ioc_publish_bserver_networks(&m_bmain, publish);

    /* Enable user authentication. Basic server pointer (m_bmain) is set as context, this
     * is needed to pass notifications (like "new device", or "wrong password") to server
     * status signals.
     */
    ioc_enable_user_authentication(&iocom_root, ioc_authorize, &m_bmain);
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

    /* Call basic server implementation to maintain control streams.
     */
    ioc_run_bserver(&m_bmain);

    for (i = 0; i<m_nro_apps; i++) {
        m_app[i]->run();
    }

    return OSAL_SUCCESS;
}
