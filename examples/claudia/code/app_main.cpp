/**

  @file    app_main.cpp
  @brief   Entry point and IO controller program set up.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  Code here is general program setup code. It initializes iocom library to be used as dynamic
  IO controller. This example code uses eosal functions everywhere, including the program
  entry point osal_main(). If you use iocom library from an existing program, just call library
  iocom functions from C or C++ code and ignore "framework style" code here.

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

/* IOCOM root object for this application
 */
iocRoot iocom_root;

/* Pointer to IO application's root object.
 */
static AppRoot *app_root;

/* IO device/network configuration.
 */
static iocNodeConf app_device_conf;

/* IO console state (for development/testing)
 */
IO_DEVICE_CONSOLE(ioconsole);

/* We may enter idle mode when nothing to do.
 */
#if OSAL_MULTITHREAD_SUPPORT
static os_boolean idle_mode;
static os_timer idle_timer;
#endif


/* Forward referred static functions.
 */
static void app_root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,
    void *context);

/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
 */
EOSAL_C_MAIN


/**
****************************************************************************************************

  @brief Program entry point.

  The osal_main function sets up iocom library for the application and creates the application
  root object.

  @return  OSAL_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    osPersistentParams persistentprm;
    iocDeviceId *device_id;
    osalSecurityConfig *security;
    iocNetworkInterfaces *nics;
    iocWifiNetworks *wifis;
    iocConnectionConfig *connconf;
    const os_char *device_name = "claudia";

    /* Setup error handling. Here we select to keep track of network state. We could also
       set application specific event handler callback by calling osal_set_net_event_handler().
     */
    osal_initialize_net_state();

    /* Initialize persistent storage
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.subdirectory = device_name;
    os_persistent_initialze(&persistentprm);

    /* Initialize communication root and dymanic structure data root objects.
     * This demo uses dynamic signal configuration.
     */
    ioc_initialize_root(&iocom_root, IOC_CREATE_OWN_MUTEX);

    /* If we are using devicedir for development testing, initialize.
     */
    io_initialize_device_console(&ioconsole, &iocom_root);

    /* Load device/network configuration and device/user account congiguration (persistent
       storage is typically either file system or micro-controller's flash). Defaults are
       set in network-defaults.json and in account-defaults.json.
     */
    ioc_load_node_config(&app_device_conf, ioapp_network_defaults,
        sizeof(ioapp_network_defaults), device_name, 0);
    device_id = ioc_get_device_id(&app_device_conf);

    ioc_set_iodevice_id(&iocom_root, device_name, device_id->device_nr,
        device_id->password, device_id->network_name);
    ioc_initialize_dynamic_root(&iocom_root);

    /* Create claudia main object
     */
    app_root = new AppRoot(device_name, device_id->device_nr, device_id->network_name,
        device_id->publish);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(&iocom_root, app_root_callback, OS_NULL);

    /* Setup network interface configuration and initialize transport library. This is
       partyly ignored if network interfaces are managed by operating system
       (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&app_device_conf);
    wifis = ioc_get_wifis(&app_device_conf);
    security = ioc_get_security_conf(&app_device_conf);
    osal_tls_initialize(nics->nic, nics->n_nics, wifis->wifi, wifis->n_wifi, security);
    osal_serial_initialize();

    /* Ready to go, connect to network.
     */
    connconf = ioc_get_connection_conf(&app_device_conf);
    ioc_connect_node(&iocom_root, connconf, IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD);

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Loop function to be called repeatedly.

  The osal_loop() function is called repeatedly to keep the application alive.
  If mutithreading is supported, this implements simple idle mode: Loop is slowed down
  if there is no activity for two seconds. This allows more time for other threads to
  run (not really useful in near real time environment, but useful in server).

  @param   app_context A pointer to pass application context structure, etc. Not used by this
           code example.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    osalStatus s;

    s = app_root->run();
#if OSAL_MULTITHREAD_SUPPORT
    switch (s)
    {
        default:
        case OSAL_SUCCESS:
            os_get_timer(&idle_timer);
            break;

        case OSAL_NOTHING_TO_DO:
            if (idle_mode) os_sleep(50);
            else idle_mode = os_has_elapsed(&idle_timer, 2000);
            break;
    }
#endif

    /* The call is here for development/testing.
     */
    s = io_run_device_console(&ioconsole);
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
    ioc_set_root_callback(&iocom_root, OS_NULL, OS_NULL);
    delete app_root;

    ioc_release_root(&iocom_root);
    osal_tls_shutdown();
    osal_serial_shutdown();
}


/**
****************************************************************************************************

  @brief Callback when dynamic IO network, device, etc has been connected or disconnected.

  The root_callback() function is called when memory block, io device network or io device is
  added or removed, etc.

  @param   root Pointer to the root object.
  @param   event Either IOC_NEW_MEMORY_BLOCK, IOC_MBLK_CONNECTED_AS_SOURCE,
           IOC_MBLK_CONNECTED_AS_TARGET, IOC_MEMORY_BLOCK_DELETED, IOC_NEW_NETWORK,
           IOC_NETWORK_DISCONNECTED, IOC_NEW_DEVICE or IOC_DEVICE_DISCONNECTED.
  @param   dnetwork Pointer to dynamic network object which has just been connected or is
           about to be removed.
  @param   mblk Pointer to memory block structure, OS_NULL if not available for the event.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void app_root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,
    void *context)
{
    switch (event)
    {
        case IOC_NEW_DEVICE:
            osal_trace2_str("IOC_NEW_DEVICE ", mblk->device_name);
            break;

        case IOC_NEW_NETWORK:
            osal_trace2_str("IOC_NEW_NETWORK ", dnetwork->network_name);
            // app_root->launch_app(dnetwork->network_name);     XXXXXXXXXXXXXXXXXXXXXXXXXX
            break;

        default:
            break;
    }
}
