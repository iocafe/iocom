/**

  @file    candy.c
  @brief   Candy camera IO example.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.4.2020

  IOBOARD_CTRL_CON define selects how this IO device connects to control computer. One of
  IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_CONNECT_TLS or IOBOARD_CTRL_CONNECT_SERIAL.

  Serial port can be selected using Windows style using "COM1", "COM2"... These are mapped
  to hardware/operating system in device specific manner. On Linux port names like
  "ttyS30,baud=115200" or "ttyUSB0" can be also used.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
/* Select socket, TLS or serial communication before including candy.h.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_TLS
#include "candy.h"

/* The devicedir is here for testing only, take away.
 */
#include "devicedir.h"

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

/* IO device configuration.
 */
iocNodeConf ioapp_device_conf;

/* Camera state and camera output.
 */
#if PINS_CAMERA
    static pinsCamera pins_camera;
    static iocBrickBuffer video_output;
#endif

/* IO console state (for development/testing)
 */
IO_DEVICE_CONSOLE(ioconsole);

/* Blink LED morse code to indicate boot errors.
 */
static MorseCode morse;

/* Maximum number of sockets, etc.
 */
#define IOBOARD_MAX_CONNECTIONS 1

/* Timer for sending
 */
static os_timer send_timer;

/* Camera control parameter has changed, camera on/off */
static os_boolean camera_control_changed;

/* Memory pool. We use fixed size Pool. If the system has dynamic memory allocation, fixes
 * size memory pool will be allocate by initialization. This is preferrable so executable
 * size stays smaller. If we have no dynamic memory allocation, we just allocate a static pool.
 */
#define MY_POOL_SZ \
    (IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS, \
     CANDY_EXP_MBLK_SZ, CANDY_IMP_MBLK_SZ) \
     + IOBOARD_POOL_DEVICE_INFO(IOBOARD_MAX_CONNECTIONS) \
     + IOBOARD_POOL_IMP_EXP_CONF(IOBOARD_MAX_CONNECTIONS, \
        CANDY_CONF_EXP_MBLK_SZ, CANDY_CONF_IMP_MBLK_SZ))

#define ALLOCATE_STATIC_POOL (OSAL_DYNAMIC_MEMORY_ALLOCATION == 0)

#if ALLOCATE_STATIC_POOL
    static os_char ioboard_pool[MY_POOL_SZ];
#endif

/* Streamer for transferring IO device configuration and flash program. The streamer is used
   to transfer a stream using buffer within memory block. This static structure selects which
   signals are used for straming data between the controller and IO device.
 */
static iocStreamerParams ioc_ctrl_stream_params = IOBOARD_DEFAULT_CTRL_STREAM(candy,
    ioapp_network_defaults, sizeof(ioapp_network_defaults));

static iocControlStreamState ioc_ctrl_state;


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
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    osalSecurityConfig *security;
#endif
    iocNetworkInterfaces *nics;
    osalWifiNetworks *wifis;
    iocDeviceId *device_id;
    iocConnectionConfig *connconf;
    ioboardParams prm;
    const osalStreamInterface *iface;
    osPersistentParams persistentprm;

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
        sizeof(ioapp_network_defaults), IOC_LOAD_PBNR_WIFI);
    device_id = ioc_get_device_id(&ioapp_device_conf);
    connconf = ioc_get_connection_conf(&ioapp_device_conf);

    /* Setup network interface configuration for micro-controller environment and initialize
       transport library. This is partyly ignored if network interfaces are managed by operating
       system (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&ioapp_device_conf);
    wifis = ioc_get_wifis(&ioapp_device_conf);
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    security = ioc_get_security_conf(&ioapp_device_conf);
    osal_tls_initialize(nics->nic, nics->n_nics, wifis->wifi, wifis->n_wifi, security);
#else
    osal_socket_initialize(nics->nic, nics->n_nics, wifis->wifi, wifis->n_wifi);
#endif

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
    prm.send_block_sz = CANDY_EXP_MBLK_SZ;
    prm.receive_block_sz = CANDY_IMP_MBLK_SZ;
#if ALLOCATE_STATIC_POOL
    prm.pool = ioboard_pool;
#endif
    prm.pool_sz = MY_POOL_SZ;
    prm.device_info = ioapp_signals_config;
    prm.device_info_sz = sizeof(ioapp_signals_config);
    prm.conf_send_block_sz = CANDY_CONF_EXP_MBLK_SZ;
    prm.conf_receive_block_sz = CANDY_CONF_IMP_MBLK_SZ;
    prm.exp_signal_hdr = &candy.exp.hdr;
    prm.imp_signal_hdr = &candy.imp.hdr;
    prm.conf_exp_signal_hdr = &candy.conf_exp.hdr;
    prm.conf_imp_signal_hdr = &candy.conf_imp.hdr;

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
    ioc_initialize_parameters(OS_PBNR_CUST_A);
    ioc_load_parameters();

    /* Set callback to pass communcation to pins.
     */
    ioc_add_callback(&ioboard_imp, ioboard_communication_callback, (void*)&candy_hdr);

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
        ioc_initialize_lighthouse_client(&lighthouse, is_ipv6_wildcard, OS_NULL);
    }
#endif

    /* Set up video output stream and the camera
     */
#if PINS_CAMERA
osal_debug_error("HERE A1")    ;
    ioc_initialize_brick_buffer(&video_output, &candy.camera,
        &ioboard_root, 4000, IOC_BRICK_DEVICE);

osal_debug_error("HERE A2")    ;
    pinsCameraParams camera_prm;
    PINS_CAMERA_IFACE.initialize();
osal_debug_error("HERE A3")    ;
    os_memclear(&camera_prm, sizeof(camera_prm));
    camera_prm.camera_pin = &pins.cameras.camera;
    camera_prm.callback_func = ioboard_camera_callback;
osal_debug_error("HERE A4")    ;
    PINS_CAMERA_IFACE.open(&pins_camera, &camera_prm);
osal_debug_error("HERE A5")    ;
    ioboard_configure_camera();
osal_debug_error("HERE A6")    ;
    PINS_CAMERA_IFACE.start(&pins_camera);
osal_debug_error("HERE 76")    ;
#endif

    /* Initialize library to receive wifi configuration by phototransostor.
     */
#if IOCOM_USE_GAZERBEAM
    initialize_gazerbeam_receiver(&gazerbeam, &pins.inputs.gazerbeam, GAZERBEAM_DEFAULT);
#endif

osal_debug_error("HERE B1")    ;

    /* Setup to blink LED to indicate boot errors, etc.
     */
    initialize_morse_code(&morse, &pins.outputs.led_morse, &pins.outputs.led_builtin,
        MORSE_HANDLE_NET_STATE_NOTIFICATIONS);

osal_debug_error("HERE B2")    ;

    /* Start communication.
     */
    ioboard_start_communication(&prm);

osal_debug_error("HERE B3")    ;


    os_get_timer(&send_timer);
    camera_control_changed = OS_FALSE;

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Loop function to be called repeatedly.

  The osal_loop() function maintains communication, reads IO pins (reading forwards input states
  to communication) and runs the IO device functionality.

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    os_timer ti;
    osalStatus s;
    os_int send_freq_ms = 10;

    os_get_timer(&ti);

    /* Run light house.
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
    blink_morse_code(&morse, &ti);

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

// static int u;
// ioc_set(&candy.exp.ambient, u++ / 200);

    /* The call is here for development testing.
     */
    s = io_run_device_console(&ioconsole);

    /* Send changed data synchronously from outgoing memory blocks every 100 ms. If we need
       very low latency IO in local network we can have interval like 1 ms, or just call send
       unconditionally.
       If we are not in such hurry, we can save network resources by merging multiple changes.
       to be sent together in TCP package and use value like 100 ms.
       Especially in IoT we may want to minimize number of transferred TCP packets to
       cloud server. In this case it is best to use to two timers and flush ioboard_exp and
       ioboard_conf_exp separately. We could evenu use value like 2000 ms or higher for
       ioboard_exp. For ioboard_conf_exp we need to use relatively short value, like 100 ms
       even then to keep software updates, etc. working. This doesn't generate much
       communication tough, conf_export doesn't change during normal operation.
     */
    if (os_timer_hit(&send_timer, &ti, send_freq_ms))
    {
        ioc_send_all(&ioboard_root);
        ioc_run(&ioboard_root);
    }

#if PINS_CAMERA
    if (camera_control_changed) {
        camera_control_changed = OS_FALSE;
        ioboard_control_camera();
    }
#endif


    ioc_autosave_parameters();

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
#if IOCOM_USE_LIGHTHOUSE
    ioc_release_lighthouse_client(&lighthouse);
#endif

#if CANDY_USE_SELECTWIFI
    ioc_release_selectwifi();
#endif

    ioboard_end_communication();
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    osal_tls_shutdown();
#else
    osal_socket_shutdown();
#endif

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
void ioboard_communication_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
// #if IOC_DEVICE_PARAMETER_SUPPORT
    const iocSignal *sig;
    os_int n_signals;
    osalStatus s;
    os_boolean configuration_changed;

    /* If this memory block is not written by communication, no need to do anything.
     */
    if ((handle->flags & IOC_MBLK_DOWN) == 0 ||
        (flags & IOC_MBLK_CALLBACK_RECEIVE) == 0)
    {
        return;
    }

    configuration_changed = OS_FALSE;
    sig = ioc_get_signal_range(handle, start_addr, end_addr, &n_signals);
    while (n_signals-- > 0)
    {
        if (sig->flags & IOC_PIN_PTR) {
            forward_signal_change_to_io_pin(sig, 0);
        }
        else if (sig->flags & IOC_PFLAG_IS_PRM) {
            s = ioc_set_parameter_by_signal(sig);
            if (s == OSAL_COMPLETED) {
                if (sig->flags & IOC_PFLAG_IS_PERSISTENT) {
                    configuration_changed = OS_TRUE;
                }
                else {
                    camera_control_changed = OS_TRUE;
                }
            }
        }
        sig++;
    }

    if (configuration_changed) {
#if PINS_CAMERA
        ioboard_configure_camera();
#endif
    }
}


#if PINS_CAMERA
/**
****************************************************************************************************

  @brief "New frame from camera" callback.

  The ioboard_camera_callback function is called when a camera frame is captured.
  If video transfer buffer is empty and vido output stream is open, the camera data is  moved
  to video outout buffer. Othervise camera data is dropped.

  @param   photo Pointer to a frame captured by camera.
  @param   context Application context, not used (NULL).
  @return  None.

****************************************************************************************************
*/
void ioboard_camera_callback(
    struct pinsPhoto *photo,
    void *context)
{
    if (ioc_ready_for_new_brick(&video_output) && ioc_is_brick_connected(&video_output))
    {
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
void ioboard_configure_camera(void)
{
#ifdef CANDY_EXP_CAM_NR
    ioboard_set_camera_prm(PINS_CAM_NR, &candy.exp.cam_nr);
#endif
#ifdef CANDY_EXP_IMG_WIDTH
    ioboard_set_camera_prm(PINS_CAM_IMG_WIDTH, &candy.exp.img_width);
    ioboard_get_camera_prm(PINS_CAM_IMG_WIDTH, &candy.exp.img_width);
    ioboard_get_camera_prm(PINS_CAM_IMG_HEIGHT, &candy.exp.img_height);
#endif
#ifdef CANDY_EXP_IMG_HEIGHT
    ioboard_set_camera_prm(PINS_CAM_IMG_HEIGHT, &candy.exp.img_height);
    ioboard_get_camera_prm(PINS_CAM_IMG_WIDTH, &candy.exp.img_width);
    ioboard_get_camera_prm(PINS_CAM_IMG_HEIGHT, &candy.exp.img_height);
#endif
#ifdef CANDY_EXP_FRAMERATE
    ioboard_set_camera_prm(PINS_CAM_FRAMERATE, &candy.exp.framerate);
#endif

#ifdef CANDY_EXP_BRIGHTNESS
    ioboard_set_camera_prm(PINS_CAM_BRIGHTNESS, &candy.exp.brightness);
#endif
#ifdef CANDY_EXP_CONTRAST
    ioboard_set_camera_prm(PINS_CAM_CONTRAST, &candy.exp.contrast);
#endif

#ifdef CANDY_EXP_HUE
    ioboard_set_camera_prm(PINS_CAM_HUE, &candy.exp.hue);
#endif
#ifdef CANDY_EXP_SATURATION
    ioboard_set_camera_prm(PINS_CAM_SATURATION, &candy.exp.saturation);
#endif
#ifdef CANDY_EXP_SHARPNESS
    ioboard_set_camera_prm(PINS_CAM_SHARPNESS, &candy.exp.sharpness);
#endif
#ifdef CANDY_EXP_GAMMA
    ioboard_set_camera_prm(PINS_CAM_GAMMA, &candy.exp.gamma);
#endif
#ifdef CANDY_EXP_COLOR
    ioboard_set_camera_prm(PINS_CAM_COLOR_ENABLE, &candy.exp.color);
#endif
#ifdef CANDY_EXP_WHITE_BAL
    ioboard_set_camera_prm(PINS_CAM_WHITE_BALANCE, &candy.exp.white_bal);
#endif
#ifdef CANDY_EXP_BL_COMP
    ioboard_set_camera_prm(PINS_CAM_BACKLIGHT_COMPENSATION, &candy.exp.bl_comp);
#endif
#ifdef CANDY_EXP_GAIN
    ioboard_set_camera_prm(PINS_CAM_GAIN, &candy.exp.gain);
#endif
#ifdef CANDY_EXP_EXPOSURE
    ioboard_set_camera_prm(PINS_CAM_EXPOSURE, &candy.exp.exposure);
#endif
#ifdef CANDY_EXP_IRIS
    ioboard_set_camera_prm(PINS_CAM_IRIS, &candy.exp.iris);
#endif
#ifdef CANDY_EXP_FOCUS
    ioboard_set_camera_prm(PINS_CAM_FOCUS, &candy.exp.focus);
#endif
}

/* Turn camera on/off.
 */
void ioboard_control_camera(void)
{
    if (ioc_get(&candy.exp.on)) {
        PINS_CAMERA_IFACE.start(&pins_camera);
    }
    else {
        PINS_CAMERA_IFACE.stop(&pins_camera);
    }
}

#endif
