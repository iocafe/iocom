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

#if CANDY_USE_GAZERBEAM
#include "gazerbeam.h"
static GazerbeamReceiver gazerbeam;
#endif

#if CANDY_USE_LIGHTHOUSE
#include "lighthouse.h"
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

/* Use static memory pool
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        CANDY_EXP_MBLK_SZ, CANDY_IMP_MBLK_SZ)
        + IOBOARD_POOL_DEVICE_INFO(IOBOARD_MAX_CONNECTIONS)
        + IOBOARD_POOL_IMP_EXP_CONF(IOBOARD_MAX_CONNECTIONS,
            CANDY_CONF_EXP_MBLK_SZ, CANDY_CONF_IMP_MBLK_SZ)];

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

    /* If we are using devicedir for development testing, initialize.
     */
    io_initialize_device_console(&ioconsole, &ioboard_root);

    /* Initialize persistent storage (typically flash is running in micro-controller)
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.device_name = IOBOARD_DEVICE_NAME;
    os_persistent_initialze(&persistentprm);

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
    prm.serial_con_str = connconf->connection[0].parameters;
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.send_block_sz = CANDY_EXP_MBLK_SZ;
    prm.receive_block_sz = CANDY_IMP_MBLK_SZ;
    prm.auto_synchronization = OS_FALSE;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);
    prm.device_signal_hdr = &candy_hdr;
    prm.device_info = ioapp_signal_config;
    prm.device_info_sz = sizeof(ioapp_signal_config);
    prm.conf_send_block_sz = CANDY_CONF_EXP_MBLK_SZ;
    prm.conf_receive_block_sz = CANDY_CONF_IMP_MBLK_SZ;
#if CANDY_USE_LIGHTHOUSE
    prm.lighthouse = &lighthouse;
    prm.lighthouse_func = ioc_get_lighthouse_connectstr;
#endif

    /* Start communication.
     */
    ioboard_start_communication(&prm);

    /* Set callback to pass communcation to pins.
     */
    ioc_add_callback(&ioboard_imp, pins_default_iocom_callback, OS_NULL);

    /* Connect PINS library to IOCOM library
     */
    pins_connect_iocom_library(&pins_hdr);

    /* Make sure that control stream state is clear even after soft reboot.
     */
    ioc_init_control_stream(&ioc_ctrl_state, &ioc_ctrl_stream_params);

    /* Listen for UDP broadcasts with server address. Select IPv6 is our socket connection
       string starts with '[' (indicates IPv6 address).
     */
#if CANDY_USE_LIGHTHOUSE
    ioc_initialize_lighthouse_client(&lighthouse, prm.socket_con_str[0] == '[', OS_NULL);
#endif

    /* Initialize library to receive wifi configuration by phototransostor.
     */
#if CANDY_USE_GAZERBEAM
    initialize_gazerbeam_receiver(&gazerbeam, &pins.inputs.gazerbeam, GAZERBEAM_DEFAULT);
#endif

    /* Set up video output stream and the camera
     */
#if PINS_CAMERA
    ioc_initialize_brick_buffer(&video_output, &candy.camera, &ioboard_root, 0, IOC_BRICK_DEVICE);

    pinsCameraParams camera_prm;
    PINS_CAMERA_IFACE.initialize();
    os_memclear(&camera_prm, sizeof(camera_prm));
    camera_prm.camera_nr = 0;
    camera_prm.camera_pin = &pins.cameras.camera;
    camera_prm.callback_func = ioboard_camera_callback;
    PINS_CAMERA_IFACE.open(&pins_camera, &camera_prm);
    PINS_CAMERA_IFACE.start(&pins_camera);
#endif

    /* Setup to blink LED to indicate boot errors, etc.
     */
    initialize_morse_code(&morse, &pins.outputs.led_morse,
        MORSE_HANDLE_NET_STATE_NOTIFICATIONS);

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);
    os_get_timer(&send_timer);

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

    os_get_timer(&ti);

    /* Run light house.
     */
#if CANDY_USE_LIGHTHOUSE
    ioc_run_lighthouse_client(&lighthouse);
#endif

    /* Get Wifi configuration messages from Android phone flash light -> phototransistor.
     */
#if CANDY_USE_GAZERBEAM
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
    ioc_run_brick_send(&video_output);
#endif

    /* Read all input pins from hardware into global pins structures. Reading will forward
       input states to communication.
     */
    pins_read_all(&pins_hdr, PINS_DEFAULT);

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
    if (os_timer_hit(&send_timer, &ti, 10))
    {
        ioc_send_all(&ioboard_root);
        ioc_run(&ioboard_root);
    }

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
#if CANDY_USE_LIGHTHOUSE
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

    ioc_release_node_config(&ioapp_device_conf);
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
    if (ioc_is_brick_empty(&video_output) && ioc_is_brick_connected(&video_output))
    {
        pins_store_photo_as_brick(photo, &video_output, IOC_UNCOMPRESSED_BRICK);
    }
}
#endif
