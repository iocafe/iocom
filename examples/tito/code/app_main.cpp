/**

  @file    app_main.c
  @brief   Tito controller using static IO device configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "app_main.h"

/* The devicedir is here for testing only, take away.
 */
#include "devicedir.h"

iocRoot app_iocom_root;
static AppRoot *app_root_obj;

/* IO device/network configuration.
 */
static iocNodeConf app_device_conf;

/* Light house state structure. The lighthouse sends periodic UDP broadcards
   to so that this service can be detected in network.
 */
static LighthouseServer lighthouse;



/**
****************************************************************************************************

  @brief Set up the communication.

  Initialize transport stream and set interface
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
    ioc_initialize_root(&app_iocom_root);

    /* Load device/network configuration and device/user account congiguration
       (persistent storage is typically either file system or micro-controller's flash).
       Defaults are set in network-defaults.json and in account-defaults.json.
     */
    ioc_load_node_config(&app_device_conf, ioapp_network_defaults,
        sizeof(ioapp_network_defaults), IOC_LOAD_PBNR_WIFI);
    device_id = ioc_get_device_id(&app_device_conf);
    ioc_set_iodevice_id(&app_iocom_root, device_name, device_id->device_nr,
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

     /* Connect to network.
     */
    ioc_connect_node(&app_iocom_root, connconf, IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD);

    /* Initialize light house. Sends periodic UDP broadcards to so that this service
       can be detected in network.
     */
    ioc_initialize_lighthouse_server(&lighthouse, device_id->publish, &lighthouse_info, OS_NULL);

    /* Create tito main object and start listening for clients.
     */
    app_root_obj = new AppRoot;

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

    /* The devicedir call is here for testing only, take away.
     */
    s = io_device_console(&app_iocom_root);
    if (s) return s;

    /* Run light house (send periodic UDP broadcasts so that this service can be detected)
     */
    ioc_run_lighthouse_server(&lighthouse);

    return app_root_obj->loop();
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

    delete app_root_obj;

    ioc_release_root(&app_iocom_root);
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
    /* '#ifdef' is used to compile code in only if 7-segment display is configured
       for the hardware.
     */
#ifdef PINS_SEGMENT7_GROUP
    os_char buf[TITO_DOWN_SEVEN_SEGMENT_ARRAY_SZ];
    const Pin *pin;
    os_short i;

    /* Process 7 segment display. Since this is transferred as boolean array, the
       forward_signal_change_to_io_pins() doesn't know to handle this. Thus, read
       boolean array from communication signal, and write it to IO pins.
     */
    if (ioc_is_my_address(&tito.down.seven_segment, start_addr, end_addr))
    {
        ioc_gets_array(&tito.down.seven_segment, buf, TITO_DOWN_SEVEN_SEGMENT_ARRAY_SZ);
        if (ioc_is_value_connected(tito.down.seven_segment))
        {
            osal_console_write("7 segment data received\n");
            for (i = TITO_DOWN_SEVEN_SEGMENT_ARRAY_SZ - 1, pin = pins_segment7_group;
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

    /* Call pins library extension to forward communication signal changes to IO pins.
     */
    forward_signal_change_to_io_pins(handle, start_addr, end_addr, flags);
}
#endif
