/**

  @file    minion.c
  @brief   Minion example.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.7.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
/* Select IOBOARD_CTRL_CONNECT_SOCKET before including minion.h.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SOCKET
#include "minion.h"

/* Gazerbeamm enables wifi configuration by Android phone's flash light */
#if IOCOM_USE_GAZERBEAM
    #include "gazerbeam.h"
    static GazerbeamReceiver gazerbeam;
#endif

#if IOCOM_USE_LIGHTHOUSE
    #include "lighthouse.h"
    static os_boolean lighthouse_on;
    static os_boolean is_ipv6_wildcard;
    static LighthouseClient lighthouse;
#endif

/* Camera state and camera output */
#if PINS_CAMERA
    static pinsCamera pins_camera;
    static iocBrickBuffer video_output;
    /* Camera control parameter has changed, camera on/off */
    static os_boolean camera_on_or_off;
    static os_boolean camera_is_on;
#endif

/* IO console for wifi configuration and development testing over serial port */
IO_DEVICE_CONSOLE(ioconsole);

/* Blink LED - morse code to indicate network status */
#if IOCOM_USE_MORSE
    static MorseCode morse;
#endif

/* Device configuration and information (nc = network configuration, rm resource monitor). */
iocNodeConf ioapp_device_conf;
static dinfoNodeConfState dinfo_nc;
static dinfoResMonState dinfo_rm;

/* Timer for sending */
static os_timer send_timer;

/* IOBOARD_MAX_CONNECTIONS is maximum number of sockets, etc, connections.
   This app uses dynamic memory allocation, so no static pool
 */
#define IOBOARD_MAX_CONNECTIONS 4

/* The iocStreamerParams structure sets which signals are used for transferring IO device
   configuration and flash program.
 */
static iocStreamerParams ioc_ctrl_stream_params = IOBOARD_DEFAULT_CTRL_STREAM(minion,
    ioapp_network_defaults, sizeof(ioapp_network_defaults));

static iocControlStreamState ioc_ctrl_state;

/* Prototypes of forward referred static functions.
 */
static void ioboard_communication_callback(
    struct iocHandle *handle, os_int start_addr, os_int end_addr,
    os_ushort flags, void *context);

#if PINS_CAMERA
    static void ioboard_camera_callback(
        struct pinsPhoto *photo,
        void *context);

    static void ioboard_configure_camera(void);
    static void ioapp_turn_camera_on_or_off(void);
#endif

/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
 */
EOSAL_C_MAIN


/**
****************************************************************************************************

  @brief Set up the communication.

  Sets up network and Initialize transport
  @return  OSAL_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    osalSecurityConfig *security = OS_NULL;
    iocNetworkInterfaces *nics;
    iocWifiNetworks *wifis;
    iocDeviceId *device_id;
    iocConnectionConfig *connconf;
    ioboardParams prm;
    const osalStreamInterface *iface;
    osPersistentParams persistentprm;
    dinfoNodeConfSignals nc_sigs;
    dinfoSystemSpeSignals si_sigs;
    dinfoResMonSignals rm_sigs;
    OSAL_UNUSED(argc);
    OSAL_UNUSED(argv);

    /* Setup error handling. Here we select to keep track of network state. We could also
       set application specific error handler callback by calling osal_set_error_handler().
     */
    osal_initialize_net_state();

    /* Initialize persistent storage (typically flash is running in micro-controller)
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.device_name = IOBOARD_DEVICE_NAME;
    os_persistent_initialze(&persistentprm);

    /* If we are using devicedir for development testing, initialize.
     */
    io_initialize_device_console(&ioconsole, &ioboard_root);

    /* Setup IO pins.
     */
    pins_setup(&pins_hdr, PINS_DEFAULT);

    /* Load device configuration from peristant storage, or if not available use
       defaults compiled in this code (config/include/<hw>/<device_name>-network-defaults.c, etc).
     */
    ioc_load_node_config(&ioapp_device_conf, ioapp_network_defaults,
        sizeof(ioapp_network_defaults), persistentprm.device_name, IOC_LOAD_PBNR_NODE_CONF);
    device_id = ioc_get_device_id(&ioapp_device_conf);
    connconf = ioc_get_connection_conf(&ioapp_device_conf);

    /* Setup network interface configuration for micro-controller environment and initialize
       transport library. This is partyly ignored if network interfaces are managed by operating
       system (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&ioapp_device_conf);
    wifis = ioc_get_wifis(&ioapp_device_conf);
    osal_socket_initialize(nics->nic, nics->n_nics, wifis->wifi, wifis->n_wifi);

    /* Initialize up device information.
     */
    DINFO_SET_COMMON_NET_CONF_SIGNALS_FOR_WIFI(nc_sigs, minion);
    DINFO_SET_COMMON_RESOURCE_MONITOR_SIGNALS(rm_sigs, minion);
    dinfo_initialize_node_conf(&dinfo_nc, &nc_sigs);
    dinfo_initialize_resource_monitor(&dinfo_rm, &rm_sigs);

    /* Get stream interface by IOBOARD_CTRL_CON define.
     */
    iface = IOBOARD_IFACE;

    /* Set up parameters for the IO board.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = iface;
    prm.device_name = IOBOARD_DEVICE_NAME; /* or device_id->device name to allow change */
    prm.device_nr = device_id->device_nr;
    prm.password = device_id->password;
    prm.network_name = device_id->network_name;
    prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.socket_con_str = connconf->connection[0].parameters;
    prm.serial_con_str = prm.socket_con_str;
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.exp_mblk_sz = MINION_EXP_MBLK_SZ;
    prm.imp_mblk_sz = MINION_IMP_MBLK_SZ;
    prm.dexp_mblk_sz = MINION_DEXP_MBLK_SZ;
    prm.dimp_mblk_sz = MINION_DIMP_MBLK_SZ;
    prm.device_info = ioapp_signals_config;
    prm.device_info_sz = sizeof(ioapp_signals_config);
    prm.conf_exp_mblk_sz = MINION_CONF_EXP_MBLK_SZ;
    prm.conf_imp_mblk_sz = MINION_CONF_IMP_MBLK_SZ;
    prm.exp_signal_hdr = &minion.exp.hdr;
    prm.imp_signal_hdr = &minion.imp.hdr;
    prm.dexp_signal_hdr = &minion.dexp.hdr;
    prm.dimp_signal_hdr = &minion.dimp.hdr;
    prm.conf_exp_signal_hdr = &minion.conf_exp.hdr;
    prm.conf_imp_signal_hdr = &minion.conf_imp.hdr;

#if IOCOM_USE_LIGHTHOUSE
    lighthouse_on = ioc_is_lighthouse_used(prm.socket_con_str, &is_ipv6_wildcard);
    if (lighthouse_on) {
        prm.lighthouse = &lighthouse;
        prm.lighthouse_func = ioc_get_lighthouse_connectstr;
    }
#endif

    /* Initialize IOCOM and set up memory blocks for the ioboard.
     */
    ioboard_setup_communication(&prm);

    /* Initialize defaults and try to load camera parameters from persistent storage
       to "exp" memory buffer.
     */
    ioc_initialize_parameters(&candy, OS_PBNR_CUST_A);
    ioc_load_parameters();

    /* Set up device information.
     */
    dinfo_set_node_conf(&dinfo_nc, device_id, connconf, nics, wifis, security);
    DINFO_SET_COMMON_SYSTEM_SPECS_SIGNALS(si_sigs, minion);
    dinfo_set_system_specs(&si_sigs, MINION_HW);

    /* Set callback to pass communcation to pins.
     */
    ioc_add_callback(&ioboard_imp, ioboard_communication_callback, (void*)&minion_hdr);

    /* Connect PINS library to IOCOM library
     */
    pins_connect_iocom_library(&pins_hdr);

    /* Make sure that control stream state is clear even after soft reboot.
     */
    ioc_init_control_stream(&ioc_ctrl_state, &ioc_ctrl_stream_params);

    /* Listen for UDP broadcasts with server address. Select IPv6 is our socket connection
       string starts with '[' (indicates IPv6 address).
     */
#if IOCOM_USE_LIGHTHOUSE
    if (lighthouse_on) {
        ioc_initialize_lighthouse_client(&lighthouse, is_ipv6_wildcard, OS_FALSE, OS_NULL);
    }
#endif

    /* Set up video output stream and the camera
     */
#if PINS_CAMERA
    ioc_initialize_brick_buffer(&video_output, &minion.camera,
        &ioboard_root, 4000, IOC_BRICK_DEVICE);

    pinsCameraParams camera_prm;
    PINS_CAMERA_IFACE.initialize();
    os_memclear(&camera_prm, sizeof(camera_prm));
    camera_prm.camera_pin = &pins.cameras.camera;
    camera_prm.callback_func = ioboard_camera_callback;
    PINS_CAMERA_IFACE.open(&pins_camera, &camera_prm);
    ioboard_configure_camera();
    camera_on_or_off = camera_is_on = OS_FALSE;
    ioapp_turn_camera_on_or_off();
#endif

    /* Initialize library to receive wifi configuration by phototransostor.
     */
#if IOCOM_USE_GAZERBEAM
    initialize_gazerbeam_receiver(&gazerbeam, &pins.inputs.gazerbeam, GAZERBEAM_DEFAULT);
#endif

    /* Setup to blink LED to indicate boot errors, etc.
     */
#if IOCOM_USE_MORSE
    initialize_morse_code(&morse, &pins.outputs.led_morse, &pins.outputs.led_builtin,
        MORSE_HANDLE_NET_STATE_NOTIFICATIONS);
#endif

    /* Start communication.
     */
    ioboard_start_communication(&prm);

    os_get_timer(&send_timer);

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Loop function to be called repeatedly.

  The osal_loop() function maintains communication, reads IO pins, etc and runs the IO device
  functionality.

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as "reboot" on micro-controller or "exit the program" on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    os_timer ti;
    osalStatus s;
    os_int send_freq_ms = 10;
    OSAL_UNUSED(app_context);

    os_get_timer(&ti);

    /* Run light house to detect server in LAN.
     */
#if IOCOM_USE_LIGHTHOUSE
    if (lighthouse_on) {
        ioc_run_lighthouse_client(&lighthouse);
    }
#endif

    /* Get Wifi configuration messages from Android phone flash light -> phototransistor.
     */
#if IOCOM_USE_GAZERBEAM
    gazerbeam_run_configurator(&gazerbeam, GAZERBEAM_DEFAULT);
#endif

    /* Keep the morse code LED alive. These indicates boot issues, etc, to user.
     */
#if IOCOM_USE_MORSE
    blink_morse_code(&morse, &ti);
#endif

    /* Keep the communication alive. Move data data synchronously
       to incoming memory block and keep control stream alive.
     */
    ioc_run(&ioboard_root);
    ioc_receive_all(&ioboard_root);
    ioc_run_control_stream(&ioc_ctrl_state, &ioc_ctrl_stream_params);

#if PINS_CAMERA
    s = ioc_run_brick_send(&video_output);
    if (s == OSAL_SUCCESS) send_freq_ms = 2;
#endif

    /* Read all input pins from hardware into global pins structures. Reading will forward
       input states to communication.
     */
    pins_read_all(&pins_hdr, PINS_DEFAULT);

    /* The call is here for development testing.
     */
    s = io_run_device_console(&ioconsole);

    /* Send changed data synchronously from outgoing memory blocks.
     */
    if (os_timer_hit(&send_timer, &ti, send_freq_ms))
    {
        ioc_send_all(&ioboard_root);
        ioc_run(&ioboard_root);
    }

#if PINS_CAMERA
    if (camera_on_or_off) {
        camera_on_or_off = OS_FALSE;
        ioapp_turn_camera_on_or_off();
    }
#endif

    /* Check for tasks, like saving parameters, changes in network node configuration and
       keep resource monitor signals alive.
     */
    ioc_autosave_parameters();
    dinfo_run_node_conf(&dinfo_nc, &ti);
    dinfo_run_resource_monitor(&dinfo_rm, &ti);

    return s;
}


/**
****************************************************************************************************

  @brief Finished with the application, clean up.

  The osal_main_cleanup() function ends IO board communication, cleans up and finshes with the
  socket and serial port libraries.

  On real IO device we may not need to take care about this, since these are often shut down
  only by turning or power or by microcontroller reset.

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  None.

****************************************************************************************************
*/
void osal_main_cleanup(
    void *app_context)
{
    OSAL_UNUSED(app_context);

#if IOCOM_USE_LIGHTHOUSE
    ioc_release_lighthouse_client(&lighthouse);
#endif

#if MINION_USE_SELECTWIFI
    ioc_release_selectwifi();
#endif

    ioboard_end_communication();
    osal_socket_shutdown();

#if PINS_CAMERA
    PINS_CAMERA_IFACE.close(&pins_camera);
#endif
    pins_shutdown(&pins_hdr);

    ioc_release_node_config(&ioapp_device_conf);
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
static void ioboard_communication_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    const iocSignal *sig, *pin_sig;
    os_int n_signals;
    osalStatus s;
#if PINS_CAMERA
    os_boolean configuration_changed = OS_FALSE;
#endif
    OSAL_UNUSED(context);

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
    dinfo_node_conf_callback(&dinfo_nc, sig, n_signals, flags);

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
                    camera_on_or_off = OS_TRUE;
                }
#endif
            }
        }
#endif
        sig++;
    }

#if PINS_CAMERA
    if (configuration_changed) {
        ioboard_configure_camera();
    }
#endif
}


#if PINS_CAMERA
/**
****************************************************************************************************

  @brief "New frame from camera" callback.

  The ioboard_camera_callback function is called when a camera frame is captured.
  If video transfer buffer is empty and vido output stream is open, the camera data is  moved
  to video outout buffer. Otherwise camera data is dropped.

  @param   photo Pointer to a frame captured by camera.
  @param   context Application context, not used (NULL).
  @return  None.

****************************************************************************************************
*/
static void ioboard_camera_callback(
    struct pinsPhoto *photo,
    void *context)
{
    if (ioc_ready_for_new_brick(&video_output) && ioc_is_brick_connected(&video_output))
    {
        photo->iface->finalize_photo(photo);
        pins_store_photo_as_brick(photo, &video_output, IOC_DEFAULT_COMPRESSION);
    }
}


/**
****************************************************************************************************

  @brief Configure one camera parameter.

  The ioboard_set_camera_prm function sets a camera parameter to camera API wrapper. The
  value to set is taken from a signal in "exp" memory block.

  @param   ix Camera parameter index, like PINS_CAM_BRIGHTNESS.
  @param   sig Pointer to signal in "exp" memory block.
  @return  None.

****************************************************************************************************
*/
static void ioboard_set_camera_prm(
    pinsCameraParamIx ix,
    const iocSignal *sig)
{
    os_long x;
    os_char state_bits;

    x = ioc_get_ext(sig, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
    if (state_bits & OSAL_STATE_CONNECTED) {
        PINS_CAMERA_IFACE.set_parameter(&pins_camera, ix, x);
    }
}


/**
****************************************************************************************************

  @brief Get camera parameter from camera driver.

  The ioboard_get_camera_prm function reads a camera parameter from camera wrapper and
  stores the value in signal in "exp" memory block.

  @param   ix Camera parameter index, like PINS_CAM_BRIGHTNESS.
  @param   sig Pointer to signal in "exp" memory block.
  @return  None.

****************************************************************************************************
*/
static void ioboard_get_camera_prm(
    pinsCameraParamIx ix,
    const iocSignal *sig)
{
    os_long x;
    x = PINS_CAMERA_IFACE.get_parameter(&pins_camera, ix);
    ioc_set(sig, x);
}


/**
****************************************************************************************************

  @brief Configure camera.

  The ioboard_set_camera_parameters function sets all camera parameters from signals in
  "exp" memory block to camera API.
  @return  None.

****************************************************************************************************
*/
static void ioboard_configure_camera(void)
{
#ifdef MINION_EXP_CAM_NR
    ioboard_set_camera_prm(PINS_CAM_NR, &minion.exp.cam_nr);
#endif
#ifdef MINION_EXP_IMG_WIDTH
    ioboard_set_camera_prm(PINS_CAM_IMG_WIDTH, &minion.exp.img_width);
    ioboard_get_camera_prm(PINS_CAM_IMG_WIDTH, &minion.exp.img_width);
    ioboard_get_camera_prm(PINS_CAM_IMG_HEIGHT, &minion.exp.img_height);
#endif
#ifdef MINION_EXP_IMG_HEIGHT
    ioboard_set_camera_prm(PINS_CAM_IMG_HEIGHT, &minion.exp.img_height);
    ioboard_get_camera_prm(PINS_CAM_IMG_WIDTH, &minion.exp.img_width);
    ioboard_get_camera_prm(PINS_CAM_IMG_HEIGHT, &minion.exp.img_height);
#endif
#ifdef MINION_EXP_FRAMERATE
    ioboard_set_camera_prm(PINS_CAM_FRAMERATE, &minion.exp.framerate);
#endif

#ifdef MINION_EXP_BRIGHTNESS
    ioboard_set_camera_prm(PINS_CAM_BRIGHTNESS, &minion.exp.brightness);
#endif
#ifdef MINION_EXP_SATURATION
    ioboard_set_camera_prm(PINS_CAM_SATURATION, &minion.exp.saturation);
#endif
}


/**
****************************************************************************************************

  @brief Turn camera on/off.

  The ioapp_turn_camera_on_or_off function calls pins library to start or stop the camera.
  @return  None.

****************************************************************************************************
*/
static void ioapp_turn_camera_on_or_off(void)
{
    os_boolean turn_on;

    turn_on = (os_boolean)ioc_get(&minion.exp.on);
    if (turn_on != camera_is_on) {
        if (ioc_get(&minion.exp.on)) {
            PINS_CAMERA_IFACE.start(&pins_camera);
        }
        else {
            PINS_CAMERA_IFACE.stop(&pins_camera);
        }
        camera_is_on = turn_on;
    }
}

#endif
