/**

  @file    gina.c
  @brief   Gina IO board example featuring  IoT device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  IOBOARD_CTRL_CON define selects how this IO device connects to control computer. One of
  IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_CONNECT_TLS or IOBOARD_CTRL_CONNECT_SERIAL.

  GINA_SERIAL_PORT: Serial port can be selected using Windows style using "COM1",
  "COM2"... These are mapped to hardware/operating system in device specific
  manner. On Linux port names like "ttyS30,baud=115200" or "ttyUSB0" can be also used.

  IOBOARD_MAX_CONNECTIONS sets maximum number of connections. IO board needs one connection.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
/* Select socket, TLS or serial communication before including gina.h.
 */
// #define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SOCKET
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_TLS
#include "gina.h"

/* Enable wifi selection by blue tooth (0 or 1) ?.
 */
#define GINA_USE_SELECTWIFI 1
#if GINA_USE_SELECTWIFI
#include "selectwifi.h"
#endif

/* The devicedir is here for testing only, take away.
 */
#include "devicedir.h"

/* IO device configuration.
 */
iocNodeConf ioapp_device_conf;

/* Maximum number of sockets, etc.
 */
#define IOBOARD_MAX_CONNECTIONS 1

/* Use static memory pool
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        GINA_EXP_MBLK_SZ, GINA_IMP_MBLK_SZ)
        + IOBOARD_POOL_DEVICE_INFO(IOBOARD_MAX_CONNECTIONS)
        + IOBOARD_POOL_IMP_EXP_CONF(IOBOARD_MAX_CONNECTIONS,
            GINA_CONF_EXP_MBLK_SZ, GINA_CONF_IMP_MBLK_SZ)];

/* Streamer for transferring IO device configuration and flash program. The streamer is used
   to transfer a stream using buffer within memory block. This static structure selects which
   signals are used for straming data between the controller and IO device.
 */
static iocStreamerParams ioc_ctrl_stream_params = IOBOARD_DEFAULT_CTRL_STREAM(gina,
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
    iocDeviceId *device_id;
    iocConnectionConfig *connconf;
    ioboardParams prm;
    const osalStreamInterface *iface;
    osPersistentParams persistentprm;

    /* Initialize persistent storage (typically flash is running in micro-controller)
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.device_name = IOBOARD_DEVICE_NAME;
    os_persistent_initialze(&persistentprm);

    /* Setup IO pins.
     */
    pins_setup(&pins_hdr, 0);

    /* disableCore0WDT();
    disableCore1WDT();
    disableLoopWDT(); */

    /* Load device configuration from peristant storage, or if not available use
       defaults compiled in this code (config/include/<hw>/<device_name>-network-defaults.c, etc).
     */
    ioc_load_node_config(&ioapp_device_conf, ioapp_network_defaults, sizeof(ioapp_network_defaults));
    device_id = ioc_get_device_id(&ioapp_device_conf);
    connconf = ioc_get_connection_conf(&ioapp_device_conf);

    /* Setup network interface configuration for micro-controller environment and initialize
       transport library. This is partyly ignored if network interfaces are managed by operating
       system (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&ioapp_device_conf);
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    security = ioc_get_security_conf(&ioapp_device_conf);
    osal_tls_initialize(nics->nic, nics->n_nics, security);
#else
    osal_socket_initialize(nics->nic, nics->n_nics);
#endif
    osal_serial_initialize();

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
    prm.send_block_sz = GINA_EXP_MBLK_SZ;
    prm.receive_block_sz = GINA_IMP_MBLK_SZ;
    prm.auto_synchronization = OS_FALSE;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);
    prm.device_signal_hdr = &gina_hdr;
    prm.device_info = ioapp_signal_config;
    prm.device_info_sz = sizeof(ioapp_signal_config);
    prm.conf_send_block_sz = GINA_CONF_EXP_MBLK_SZ;
    prm.conf_receive_block_sz = GINA_CONF_IMP_MBLK_SZ;

    /* Start communication.
     */
    ioboard_start_communication(&prm);

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ioboard_imp, ioboard_communication_callback, OS_NULL);

    /* Connect PINS library to IOCOM library
     */
    pins_connect_iocom_library(&pins_hdr);

    /* Make sure that control stream state is clear even after soft reboot.
     */
    ioc_init_control_stream(&ioc_ctrl_state, &ioc_ctrl_stream_params);

    /* Enable wifi selection by blue tooth.
     */
#if GINA_USE_SELECTWIFI
    ioc_initialize_selectwifi(OS_NULL);
#endif

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
    /* Keep the communication alive. If data is received from communication, the
       ioboard_communication_callback() will be called. Move data data synchronously
       to incomong memory block.
     */
// #if OSAL_MULTITHREAD_SUPPORT == 0
    ioc_run(&ioboard_communication);
// #endif
    ioc_receive(&ioboard_imp);
    ioc_receive(&ioboard_conf_imp);
    ioc_run_control_stream(&ioc_ctrl_state, &ioc_ctrl_stream_params);
#if 1
    /* Read all input pins from hardware into global pins structures. Reading will forward
       input states to communication.
     */
//     pins_read_all(&pins_hdr, PINS_DEFAULT);

    /* Run the IO device functionality.
     */
    // static os_float f[5] = {1, 2, 3, 4, 5};
    static os_timer ti;
    static os_int i = 0;
    os_char /* buf[32], */ state_bits;
    os_long l;
    static os_long prev_l = -1;

    if (i++ == 0) os_get_timer(&ti);

    if (os_elapsed(&ti, 200))
    {
        os_get_timer(&ti);

        l = ioc_gets_int(&gina.conf_imp.frd_select, &state_bits, IOC_SIGNAL_DEFAULT);
        if (l != prev_l)
        {
            osal_debug_error_int("~HERE CHANGE", l);
            osal_debug_error_int(" state ", state_bits);
            prev_l = l;
        }

        // ioc_sets_str(&gina.exp.teststr, "pekka");

        /* f[2] = i++;
        ioc_sets_array(&gina.exp.testfloat, f, 5);

        ioc_sets_str(&gina.exp.teststr, "pekka");
        ioc_gets_str(&gina.imp.strtodevice, buf, sizeof(buf));
        */
    }
#endif
    /* The devicedir call is here for testing only, take away.
     */
    io_device_console(&ioboard_communication);

    /* Move data synchronously from outgoing memory block.
     */
    ioc_send(&ioboard_exp);
    ioc_send(&ioboard_conf_exp);

    return OSAL_SUCCESS;
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
#if GINA_USE_SELECTWIFI
    ioc_release_selectwifi();
#endif

    ioboard_end_communication();
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    osal_tls_shutdown();
#else
    osal_socket_shutdown();
#endif
    osal_serial_shutdown();

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
#undef PINS_SEGMENT7_GROUP

    /* '#ifdef' is used to compile code in only if 7-segment display is configured
       for the hardware.
     */
#ifdef PINS_SEGMENT7_GROUP
    os_char buf[GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ];
    const Pin *pin;
    os_short i;
#endif
    if (flags & IOC_MBLK_CALLBACK_RECEIVE)
    {
#ifdef PINS_SEGMENT7_GROUP
        /* Process 7 segment display. Since this is transferred as boolean array, the
           forward_signal_change_to_io_pins() doesn't know to handle this. Thus, read
           boolean array from communication signal, and write it to IO pins.
         */
        if (ioc_is_my_address(&gina.imp.seven_segment, start_addr, end_addr))
        {
            sb = ioc_gets_array(&gina.imp.seven_segment, buf, GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ);
            if (sb & OSAL_STATE_CONNECTED)
            {
                osal_console_write("7 segment data received\n");
                for (i = GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ - 1, pin = pins_segment7_group;
                     i >= 0 && pin;
                     i--, pin = pin->next) /* For now we need to loop backwards, fix this */
                {
                    pin_set(pin, buf[i]);
                }
            }
            else
            {
                // WE DO NOT COME HERE. SHOULD WE INVALIDATE WHOLE MAP ON DISCONNECT?
                osal_console_write("7 segment data DISCONNECTED\n");
            }
        }
#endif

        /* Call pins library extension to forward communication signal changed to IO pins.
         */
        forward_signal_change_to_io_pins(handle, start_addr, end_addr, flags);
    }
}
