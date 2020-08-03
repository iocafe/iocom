/**

  @file    application.cpp
  @brief   Buster application's main class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "buster.h"


/* static osalStatus app_gina1_photo_received(
    struct iocBrickBuffer *b,
    void *context); */


void Application::start(os_int argc, const os_char *argv[])
{
    iocBServerParams prm;

    /* Initialize IOCOM basics.
     */
    init_application_basics("buster", ioapp_network_defaults, sizeof(ioapp_network_defaults),
        &pins_hdr, argc, argv);

    /* Initialize signal structure for this device.
     */
    buster_init_signal_struct(&m_signals);

    /* Setup IO server
     */
    IOC_SETUP_BSERVER_PARAMS(prm, m_signals, m_device_id->device_name, m_device_id->device_nr,
        m_device_id->network_name, ioapp_signals_config, ioapp_network_defaults)
    ioc_initialize_ioserver(&m_bmain, &m_root, &prm);
    IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(m_bmain, m_signals)

    /* Publish IO networks hosted by frank, such as "iocafenet" or "asteroidnet"
     */
    ioc_publish_bserver_networks(&m_bmain, m_device_id->publish);

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&m_bmain.imp, pins_default_iocom_callback, &m_signals.hdr);

    /* Enable user authentication. Basic server pointer (m_bmain) is set as context, this
     * is needed to pass notifications (like "new device", or "wrong password") to server
     * status signals.
     */
    ioc_enable_user_authentication(&m_root, ioc_authorize, &m_bmain);

    connect_it();

    m_minion1_def = m_minion1.inititalize(m_device_id->network_name, 1);

    // ioc_set_brick_received_callback(&m_gina1.m_camera_buffer, app_gina1_photo_received, this);
    // ioc_brick_set_receive(&m_gina1.m_camera_buffer, OS_TRUE);

    m_test_seq1.start(this);
}

void Application::stop()
{
    m_test_seq1.stop();
    cleanup_app_basics();
}

osalStatus Application::run(os_timer *ti)
{
    ioc_receive_all(&m_root);
    ioc_run_brick_receive(&m_minion1.m_camera_buffer);

    /* Call basic server implementation to maintain control streams. */
    ioc_run_bserver(&m_bmain);

    run_app_library(ti);

    ioc_send_all(&m_root);
    return OSAL_SUCCESS;
}

/* static osalStatus app_gina1_photo_received(
    struct iocBrickBuffer *b,
    void *context)
{
    osal_debug_error("NEW PHOTO");
    return OSAL_SUCCESS;
} */
