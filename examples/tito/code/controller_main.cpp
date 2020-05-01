/**

  @file    controller_main.c
  @brief   Program entry point, Tito IO controller set up.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  Code here is general program setup code. It initializes iocom library to be used as automation
  device controller. This example code uses eosal functions everywhere, including the program
  entry point osal_main(). If you use iocom library from an existing program, just call library
  iocom functions from C or C++ code and ignore "framework style" code here.

  The Tito conroller example here uses static IO device configuration. This means that
  communication signal map from IO board JSON files, etc, is compiled into Tito's code ->
  run time matching IO signal at IO device and in Tito is by address and type, not by signal name.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "controller_main.h"

/* The devicedir is here for testing only, take away.
 */
#include "devicedir.h"

iocRoot iocom_root;
static ApplicationRoot *app_root;

/* IO device/network configuration.
 */
static iocNodeConf app_device_conf;

/* IO console state (for development/testing)
 */
IO_DEVICE_CONSOLE(ioconsole);

/* Light house state structure. The lighthouse sends periodic UDP broadcards
   to so that this service can be detected in network.
 */
static LighthouseServer lighthouse;


/**
****************************************************************************************************

  @brief The controller program entry point.

  Initialize IOCOM and start the IO controller application.

  @oaran   argc Number of command line arguments (PC only)
  @oaran   argv Array of command line argument pointers (PC only)
  @return  OSAL_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    const os_char *device_name = "tito";
    osPersistentParams persistentprm;
    iocConnectionConfig *connconf;
    osalSecurityConfig *security;
    iocNetworkInterfaces *nics;
    osalWifiNetworks *wifis;
    iocDeviceId *device_id;
    iocLighthouseInfo lighthouse_info;

    /* Setup error handling. Here we select to keep track of network state. We could also
       set application specific error handler callback by calling osal_set_error_handler().
     */
    osal_initialize_net_state();

    /* Initialize persistent storage
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.device_name = device_name;
    os_persistent_initialze(&persistentprm);

    /* Initialize communication root object.
     */
    ioc_initialize_root(&iocom_root);

    /* If we are using devicedir for development testing, initialize.
     */
    io_initialize_device_console(&ioconsole, &iocom_root);

    /* Setup IO pins.
     */
#if PINS_LIBRARY
    pins_setup(&pins_hdr, PINS_DEFAULT);
#endif

    /* Load device/network configuration and device/user account congiguration
       (persistent storage is typically either file system or micro-controller's flash).
       Defaults are set in network-defaults.json and in account-defaults.json.
     */
    ioc_load_node_config(&app_device_conf, ioapp_network_defaults,
        sizeof(ioapp_network_defaults), IOC_LOAD_PBNR_WIFI);
    device_id = ioc_get_device_id(&app_device_conf);
    ioc_set_iodevice_id(&iocom_root, device_name, device_id->device_nr,
        device_id->password, device_id->network_name);

    /* Get service TCP port number and transport (IOC_TLS_SOCKET or IOC_TCP_SOCKET).
     */
    connconf = ioc_get_connection_conf(&app_device_conf);
    ioc_get_lighthouse_info(connconf, &lighthouse_info);

    /* Setup network interface configuration and initialize transport library. This is
       partyly ignored if network interfaces are managed by operating system
       (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&app_device_conf);
    wifis = ioc_get_wifis(&app_device_conf);
    security = ioc_get_security_conf(&app_device_conf);
    osal_tls_initialize(nics->nic, nics->n_nics, wifis->wifi, wifis->n_wifi, security);
    osal_serial_initialize();

#if PINS_LIBRARY
    /* Set callback to detect received data and connection status changes.
     */
    // ioc_add_callback(&ioboard_imp, ioboard_communication_callback, OS_NULL);

    /* Connect PINS library to IOCOM library
     */
    pins_connect_iocom_library(&pins_hdr);
#endif

     /* Connect to network.
     */
    ioc_connect_node(&iocom_root, connconf, IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD);

    /* Initialize light house. Sends periodic UDP broadcards to so that this service
       can be detected in network.
     */
    ioc_initialize_lighthouse_server(&lighthouse, device_id->publish, &lighthouse_info, OS_NULL);

    /* Create tito main object and start listening for clients.
     */
    app_root = new ApplicationRoot(device_name, device_id->device_nr, device_id->network_name,
        device_id->publish);

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
    osalStatus s;

    /* The call is here for development/testing.
     */
    s = io_run_device_console(&ioconsole);
    if (s) return s;

    /* Run light house (send periodic UDP broadcasts so that this service can be detected)
     */
    ioc_run_lighthouse_server(&lighthouse);

    ioc_run(&iocom_root);
    s = app_root->run();
    ioc_run(&iocom_root);

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
    /* Finished with lighthouse.
     */
    ioc_release_lighthouse_server(&lighthouse);

    delete app_root;

    ioc_release_root(&iocom_root);
    osal_tls_shutdown();
    osal_serial_shutdown();
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
  @param   context Callback context, not used by "tito" example.
  @return  None.

****************************************************************************************************
*/
#if 0
void ioboard_communication_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    /* Call pins library extension to forward communication signal changes to IO pins.
     */
    forward_signal_change_to_io_pins(handle, start_addr, end_addr, flags);
}
#endif
