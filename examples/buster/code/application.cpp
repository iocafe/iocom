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

/* Global signals. This allows mapping IO pins directly to signals from JSON, but we can have only
   one application instance.
 */
struct buster_t buster;

void Application::start(os_int argc, const os_char *argv[])
{
    AbstractAppParams aprm;
    iocBServerParams sprm;
    dinfoNodeConfSignals nc_sigs;
    dinfoSystemSpeSignals si_sigs;
    dinfoResMonSignals rm_sigs;

    /* Initialize IOCOM basics.
     */
    os_memclear(&aprm, sizeof(aprm));
    aprm.network_defaults = ioapp_network_defaults;
    aprm.network_defaults_sz = sizeof(ioapp_network_defaults);
    aprm.pins_header = &pins_hdr;
    aprm.argc = argc;
    aprm.argv = argv;

    /* Call base class to set yp the application.
     */
    init_application_basics("buster", &aprm);

    /* Initialize signal structure for this device.
     */
    m_signals = &buster;
    buster_init_signal_struct(m_signals);

// xxxxxxxxxx
    /* Initialize up device information.
     */
    DINFO_SET_COMMON_NET_CONF_SIGNALS_FOR_WIFI(nc_sigs, *m_signals);
    DINFO_SET_COMMON_RESOURCE_MONITOR_SIGNALS(rm_sigs, *m_signals);
    dinfo_initialize_node_conf(&m_dinfo_nc, &nc_sigs);
    dinfo_initialize_resource_monitor(&m_dinfo_rm, &rm_sigs);

    /* Initialize defaults and try to load parameters from persistent storage
       to "exp" memory buffer.
     */
    ioc_initialize_buster_parameters(m_signals, OS_PBNR_CUST_A, OS_NULL);
    ioc_load_buster_parameters(m_signals);
// xxxxxxxxxx

    /* Setup IO server
     */
    IOC_SETUP_BSERVER_PARAMS(sprm, buster, m_device_id->device_name, m_device_id->device_nr,
        m_device_id->network_name, ioapp_signals_config, ioapp_network_defaults)
    ioc_initialize_ioserver(&m_bmain, &m_root, &sprm);
    IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(m_bmain, buster)

    /* Call communication_callback() when data is received, etc..
     */
    ioc_add_callback(&m_bmain.imp, iocom_application_communication_callback, this);

    /* Enable user authentication. Basic server pointer (m_bmain) is set as context, this
     * is needed to pass notifications (like "new device", or "wrong password") to server
     * status signals.
     */
    ioc_enable_user_authentication(&m_root, ioc_authorize, &m_bmain);

    /* Call base class application to do much of setup work.
     */
    connect_application();

// xxxxxxxxxx
    /* Set up device information.
     */
    dinfo_set_node_conf(&m_dinfo_nc, m_device_id, m_connconf, m_nics, m_wifis, m_security);
    DINFO_SET_COMMON_SYSTEM_SPECS_SIGNALS(si_sigs, *m_signals);
    dinfo_set_system_specs(&si_sigs, BUSTER_HW);

// xxxxxxxxx


    /* Publish IO networks hosted by frank, such as "iocafenet" or "asteroidnet"
     */
    ioc_publish_bserver_networks(&m_bmain, m_device_id->publish);

    m_minion1_def = m_minion1.inititalize(m_device_id->network_name, 1);

    // ioc_set_brick_received_callback(&m_gina1.m_camera_buffer, app_gina1_photo_received, this);
    // ioc_brick_set_receive(&m_gina1.m_camera_buffer, OS_TRUE);

    m_test_seq1.start(this);
}

void Application::stop()
{
    m_test_seq1.stop();
    application_cleanup();
}

osalStatus Application::run(os_timer *ti)
{
    ioc_receive_all(&m_root);
    // ioc_run_brick_receive(&m_minion1.m_camera_buffer);

    /* Call basic server implementation to maintain control streams. */
    ioc_run_bserver(&m_bmain, ti);

    run_appplication_basics(ti);

    ioc_send_all(&m_root);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Callback function when data has been received from communication.

  The ioboard_communication_callback function reacts to data from communication. Here we treat
  memory block as set of communication signals, and mostly just forward these to IO.

  @param   handle Memory block handle.
  @param   start_addr First changed memory block address.
  @param   end_addr Last changed memory block address.
  @param   flags IOC_MBLK_CALLBACK_WRITE indicates change by local write,
           IOC_MBLK_CALLBACK_RECEIVE change by data received.
  @param   context Callback context, not used by "gina" example.
  @return  None.

****************************************************************************************************
*/
void Application::communication_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags)
{
    const iocSignal *sig, *pin_sig;
    os_int n_signals;
    osalStatus s;
#if PINS_CAMERA
    os_boolean configuration_changed = OS_FALSE;
#endif

// &m_signals->hdr

    /* If this memory block is not written by communication, no need to do anything.
     */
    if ((handle->flags & IOC_MBLK_DOWN) == 0 ||
        (flags & IOC_MBLK_CALLBACK_RECEIVE) == 0)
    {
        return;
    }

    /* Get range of signals that may have changed. Signals are in order by address.
     */
    sig = ioc_get_signal_range(handle, start_addr, end_addr, &n_signals);

    /* Check if this callback causes change in device info
     */
    dinfo_node_conf_callback(&m_dinfo_nc, sig, n_signals, flags);

    while (n_signals-- > 0)
    {
        if (sig->flags & IOC_PIN_PTR) {
            forward_signal_change_to_io_pin(sig, IOC_SIGNAL_DEFAULT);
        }
#if IOC_DEVICE_PARAMETER_SUPPORT
        else if (sig->flags & IOC_PFLAG_IS_PRM) {
            s = ioc_set_parameter_by_signal(sig, &pin_sig);
            if (s == OSAL_COMPLETED) {
                if (pin_sig) {
                    forward_signal_change_to_io_pin(pin_sig, IOC_SIGNAL_NO_TBUF_CHECK);
                }
#if PINS_CAMERA
                if (sig->flags & IOC_PFLAG_IS_PERSISTENT) {
                    configuration_changed = OS_TRUE;
                }
                else {
                    m_camera_on_or_off = OS_TRUE;
                }
#endif
            }

#if IOCOM_USE_MORSE==2
            if (sig == &candy.imp.set_hlight_lvl) {
                morse.steady_hdlight_level[0] = ioc_get(sig);
            }
            if (sig == &candy.imp.set_hlight_blink) {
                morse.blink_level[0] = ioc_get(sig);
            }
#endif

        }
#endif
        sig++;
    }

#if PINS_CAMERA
    if (configuration_changed) {
        // ioboard_configure_camera();
    }
#endif
}



/* static osalStatus app_gina1_photo_received(
    struct iocBrickBuffer *b,
    void *context)
{
    osal_debug_error("NEW PHOTO");
    return OSAL_SUCCESS;
} */
