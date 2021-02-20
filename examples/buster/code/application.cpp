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
#include <math.h>

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

    /* Call base class to set up the application.
     */
    init_application_basics("buster", &aprm);

    /* Initialize signal structure for this device.
     */
    m_signals = &buster;
    buster_init_signal_struct(m_signals);

    /* Add memory blocks for camera
     */
    m_camera1.add_mblks(m_device_id->device_name, m_device_id->device_nr,
        m_device_id->network_name,
        "dexp", &m_signals->dexp.hdr, BUSTER_DEXP_MBLK_SZ,
        "dimp", &m_signals->dimp.hdr, BUSTER_DIMP_MBLK_SZ, &m_root);

    /* Setup IO server
     */
    IOC_SETUP_BSERVER_PARAMS(sprm, buster, m_device_id->device_name, m_device_id->device_nr,
        m_device_id->network_name, ioapp_signals_config, ioapp_network_defaults)
    ioc_initialize_ioserver(&m_bmain, &m_root, &sprm);
    IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(m_bmain, buster)

    /* Call communication_callback_1() for "imp" memory block, when data is received, etc..
     */
    enable_communication_callback_1(&m_bmain.imp);

    /* ioc_add_callback(&m_bmain.imp, iocom_application_communication_callback, this); */


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


    /* Enable user authentication. Basic server pointer (m_bmain) is set as context, this
     * is needed to pass notifications (like "new device", or "wrong password") to server
     * status signals.
     */
    ioc_enable_user_authentication(&m_root, ioc_authorize, &m_bmain);

    /* Setup and start cameras
     */
#if PINS_CAMERA
    m_camera1.setup_camera(&PINS_CAMERA_IFACE, &m_signals->camera, &pins.cameras.camera, &m_root);
#endif

#if IOCOM_USE_MORSE
    initialize_morse_code(&m_morse, IOCOM_MORSEPPIN, OS_NULL,
        MORSE_HANDLE_NET_STATE_NOTIFICATIONS);
#endif

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


    /* Publish IO networks hosted by frank, such as "cafenet" or "asteroidnet"
     */
    ioc_publish_bserver_networks(&m_bmain, m_device_id->publish);

    m_minion1_def = m_minion1.inititalize(m_device_id->network_name, 1);

    /* Setup and start cameras
     */
#if PINS_CAMERA
    m_camera1.turn_camera_on_or_off((os_boolean)ioc_get(&buster.exp.on));
    m_camera1.start_thread(); /* Use if running camera in separate thread */
#endif

    os_get_timer(&m_analogs_timer);
    m_gamecontroller_timer = m_analogs_timer;
    m_gamecontroller_alive = 0;

    m_test_seq1.start(this);
}

void Application::stop()
{
    /* Cameras need to be closed explicitely so that that they are not running
       after memory is released...
     */
#if PINS_CAMERA
    m_camera1.close();
#endif

    m_test_seq1.stop();
    application_cleanup();
}

osalStatus Application::run(os_timer *ti)
{
    ioc_receive_all(&m_root);
    // ioc_run_brick_receive(&m_minion1.m_camera_buffer);

    /* Read analog inputs periodically from hardware into global pins structures.
       Reading will forward input states to communication.
     */
    if (os_has_elapsed_since(&m_analogs_timer, ti, 200))
    {
        pins_read_group(pins_analogs_group);
        m_analogs_timer = *ti;
    }
    // pins_read_all(&pins_hdr, PINS_DEFAULT);

    /* Call basic server implementation to maintain control streams. */
    ioc_run_bserver(&m_bmain, ti);

    run_appplication_basics(ti);

#if IOCOM_USE_MORSE
    /* Keep the morse code LED alive. These indicates boot issues, etc, to user.
     */
    blink_morse_code(&m_morse, ti);
#endif

    steering(ti);

    /* Check for tasks, like saving parameters, changes in network node configuration and
       keep resource monitor signals alive.
     */
    ioc_autosave_buster_parameters(m_signals);
    dinfo_run_node_conf(&m_dinfo_nc, ti);
    dinfo_run_resource_monitor(&m_dinfo_rm, ti);

    ioc_send_all(&m_root);
    os_timeslice();
    return OSAL_SUCCESS;
}


void Application::steering(
    os_timer *ti)
{
    os_double speed, steering, a, center_x, ar, al, l_dir, r_dir, sl, sr;
    os_char state_bits;
    os_ushort alive;
    os_int l_forward, r_forward;
    const os_double
        coeff = 2.0 * 3.1415 / 360, /* We use randians and degrees */
        b_wheel_x = 5.5 * 2.54,
        f_wheel_x = 4.6 * 2.54,
        f_wheel_y = (7 + 1/2) * 2.54;

    alive = (os_ushort)ioc_get_ext(&m_signals->imp.gc_alive, &state_bits, IOC_SIGNAL_DEFAULT);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0) goto halt_motors;
    if (alive == m_gamecontroller_alive) {
        if (os_has_elapsed_since(&m_gamecontroller_timer, ti, 800)) goto halt_motors;
    }
    else if (alive) {
        m_gamecontroller_timer = *ti;
        m_gamecontroller_alive = alive;
    }

    /* "steering" input from -90 degrees (left) to 90 degrees (right) is for direction. 0 = straight forward.
       "speed" is movement speed from -100% (backwards) to 100% (forward).
     */
    speed = 0.01 * ioc_get_ext(&m_signals->imp.gc_LY, &state_bits, IOC_SIGNAL_DEFAULT);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0) goto halt_motors;
    steering = 0.009 * ioc_get_ext(&m_signals->imp.gc_LX, &state_bits, IOC_SIGNAL_DEFAULT);
    if ((state_bits & OSAL_STATE_CONNECTED) == 0) goto halt_motors;
    if (speed < -100) speed = -100;
    if (speed > 100) speed = 100;
    if (steering < -90) steering = -90;
    if (steering > 90) steering = 90;

    /* Calculate center_x on back wheel axis as rotation center.
     */
    a = coeff * steering;
    if (fabs(a) < 0.001) center_x = 1000000.0;
    else center_x = f_wheel_y / tan(a);

    /* Calculate wheel directins in radians and convert to degrees
     */
    ar = atan2(f_wheel_y, (center_x - f_wheel_x));
    al = atan2(f_wheel_y, (center_x + f_wheel_x));
    r_dir = ar/coeff;
    l_dir = al/coeff;
    set_angle_to_range(&r_dir);
    set_angle_to_range(&l_dir);

    /* Calculate motor speeds, sl is left motor speed and sr right motor. Positive values forward and negative back.
     */
    if (center_x >= b_wheel_x) {
        sl = speed;
        sr = sl * (center_x - b_wheel_x) /  (center_x + b_wheel_x);
    }
    else if (center_x <= -b_wheel_x) {
        sr = speed;
        sl = sr * (-center_x - b_wheel_x) /  (-center_x + b_wheel_x);
    }
    else if (center_x > 0) {
        sl = speed * (center_x + b_wheel_x) / (2 * b_wheel_x);
        sr = -sl * (b_wheel_x - center_x) /(b_wheel_x + center_x);
    }
    else {
        sr = speed * (-center_x + b_wheel_x) / (2 * b_wheel_x);
        sl = -sr * (b_wheel_x + center_x) /(b_wheel_x - center_x);
    }

/* osal_trace_int("~HERE steering", steering);
osal_trace_int("~HERE l_dir ", l_dir);
osal_trace_int("HERE r_dir ", r_dir); */

    if (sl < 0) { sl = -sl; l_forward = 0; } else { l_forward = 1; }
    if (sr < 0) { sr = -sr; r_forward = 0; } else { r_forward = 1; }
    pin_set(&pins.outputs.left_dir, l_forward);
    pin_set(&pins.outputs.right_dir, r_forward);
    pin_set_scaled(&pins.pwm.left_motor, sl, PIN_FORWARD_TO_IOCOM);
    pin_set_scaled(&pins.pwm.right_motor, sr, PIN_FORWARD_TO_IOCOM);
    pin_set_scaled(&pins.pwm.left_wheel, l_dir, PIN_FORWARD_TO_IOCOM);
    pin_set_scaled(&pins.pwm.right_wheel, r_dir, PIN_FORWARD_TO_IOCOM);
    return;

halt_motors:
    pin_set_ext(&pins.pwm.left_motor, 0, PIN_FORWARD_TO_IOCOM);
    pin_set_ext(&pins.pwm.right_motor, 0, PIN_FORWARD_TO_IOCOM);
}

void Application::set_angle_to_range(
    os_double *d)
{
    while (*d > 90.0) *d -= 180.0;
    while (*d < -90.0) *d += 180.0;
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
  @return  None.

****************************************************************************************************
*/
void Application::communication_callback_1(
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
                    m_camera1.m_camera_on_or_off = OS_TRUE;
                }
#endif
            }

#if IOCOM_USE_MORSE==2
            if (sig == &buster.imp.set_hlight_lvl) {
                morse.steady_hdlight_level[0] = ioc_get(sig);
            }
            if (sig == &buster.imp.set_hlight_blink) {
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
