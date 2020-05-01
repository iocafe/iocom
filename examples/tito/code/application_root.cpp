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


static osalStatus app_gina1_photo_received(
    struct iocBrickBuffer *b,
    void *context);


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
ApplicationRoot::ApplicationRoot(
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name,
    const os_char *publish)
{
    iocBServerParams prm;

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

    start(network_name, device_nr);
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
ApplicationRoot::~ApplicationRoot()
{
    stop();
}


void ApplicationRoot::start(const os_char *network_name, os_uint device_nr)
{
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);

    m_gina1_def = m_gina1.inititalize(m_network_name, 1);
    m_gina2_def = m_gina2.inititalize(m_network_name, 2);

    ioc_set_brick_received_callback(&m_gina1.m_camera_buffer, app_gina1_photo_received, this);
    ioc_brick_set_receive(&m_gina1.m_camera_buffer, OS_TRUE);

    m_test_seq1.start(this);
}

void ApplicationRoot::stop()
{
    m_test_seq1.stop();
}

osalStatus ApplicationRoot::run()
{
    ioc_single_thread_run(&iocom_root);
    ioc_receive_all(&iocom_root);
    ioc_run_brick_receive(&m_gina1.m_camera_buffer);

    /* Call basic server implementation to maintain control streams.
     */
    ioc_run_bserver(&m_bmain);

#if OSAL_MULTITHREAD_SUPPORT == 0
    m_test_seq1.run();
#endif

    ioc_send_all(&iocom_root);
    ioc_single_thread_run(&iocom_root);
    return OSAL_SUCCESS;
}

static osalStatus app_gina1_photo_received(
    struct iocBrickBuffer *b,
    void *context)
{
    return OSAL_SUCCESS;
}
