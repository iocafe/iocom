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
iocRoot app_iocom;

/* Pointer to IO application's root object.
 */
static AppRoot *app_root_obj;

/* IO device/network configuration.
 */
static iocNodeConf app_device_conf;


/* We may enter idle mode when nothing to do.
 */
#if OSAL_MULTITHREAD_SUPPORT
static os_boolean idle_mode;
static os_timer idle_timer;
#endif


/* Forward referred static functions.
 */
// static void app_listen_for_clients();

// static void app_connect_cloud(
//    iocConnectionConfig *connconf);

static void app_info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void app_root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,
    void *context);


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
    iocConnectionConfig *connconf;
    osalSecurityConfig *security;
    iocNetworkInterfaces *nics;
    const os_char *device_name = "frank";

    /* Initialize communication root and dymanic structure data root objects.
     * This demo uses dynamic signal configuration.
     */
    ioc_initialize_root(&app_iocom);

    /* Initialize persistent storage and load device/network configuration and device/user
       account congiguration (persistent storage is typically either file system or
       micro-controller's flash). Defaults are set in network-defaults.json and
       in account-defaults.json.
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.device_name = device_name;
    os_persistent_initialze(&persistentprm);
    ioc_load_node_config(&app_device_conf, ioapp_network_defaults, sizeof(ioapp_network_defaults));
    device_id = ioc_get_device_id(&app_device_conf);

    ioc_set_iodevice_id(&app_iocom, device_name, device_id->device_nr,
        device_id->password, device_id->network_name);
    ioc_initialize_dynamic_root(&app_iocom);

    ioc_enable_user_authentication(&app_iocom, ioc_authorize, OS_NULL);

    /* Create frank main object
     */
    app_root_obj = new AppRoot(device_name, device_id->device_nr, device_id->network_name,
        device_id->publish);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(&app_iocom, app_root_callback, OS_NULL);

    /* Setup network interface configuration and initialize transport library. This is
       partyly ignored if network interfaces are managed by operating system
       (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&app_device_conf);
    security = ioc_get_security_conf(&app_device_conf);
    osal_tls_initialize(nics->nic, nics->n_nics, security);
    osal_serial_initialize();

    /* Ready to go, start listening for clients.
     */
    // app_listen_for_clients();

    /* Connect to cloud.
     */
    // connconf = ioc_get_connection_conf(&app_device_conf);
    // app_connect_cloud(connconf);

    /* Connect to network.
     */
    connconf = ioc_get_connection_conf(&app_device_conf);
    ioc_connect_node(&app_iocom, connconf, IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD);

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

    s = app_root_obj->run();
#if OSAL_MULTITHREAD_SUPPORT
    switch (s)
    {
        default:
        case OSAL_SUCCESS:
            os_get_timer(&idle_timer);
            break;

        case OSAL_STATUS_NOTHING_TO_DO:
            if (idle_mode) os_sleep(50);
            else idle_mode = os_elapsed(&idle_timer, 2000);
            break;
    }
#endif

    s = io_device_console(&app_iocom);
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
    ioc_set_root_callback(&app_iocom, OS_NULL, OS_NULL);
    delete app_root_obj;

    ioc_release_root(&app_iocom);
    osal_tls_shutdown();
    osal_serial_shutdown();
}


#if 0
/**
****************************************************************************************************

  @brief Listen for incoming connections.

  The app_listen_for_clients creates iocom end point which listens for incoming connections
  from IO nodes.

  Notice flag IOC_DYNAMIC_MBLKS, this allows to create new memory blocks dynamically bu
  received configuration information.

  @return  None.

****************************************************************************************************
*/
static void app_listen_for_clients()
{
    iocEndPoint *ep = OS_NULL;
    iocEndPointParams epprm;

    const osalStreamInterface *iface = OSAL_TLS_IFACE;

    ep = ioc_initialize_end_point(OS_NULL, &app_iocom);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = iface;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS;
    ioc_listen(ep, &epprm);
}


/**
****************************************************************************************************

  @brief Connect to cloud server.

  If the IO controller (like this 'frank' example) runs in local network, a cloud server
  can be used to pass connections from remote devices or other nodes. This function
  connects this IO controller to cloud server application like 'claudia'.

  @param   connconf Connection configuration (from persistent storate or JSON defaults).
  @return  None.

****************************************************************************************************
*/
static void app_connect_cloud(
    iocConnectionConfig *connconf)
{
    iocConnection *con = OS_NULL;
    iocOneConnectionConf *c;
    iocConnectionParams conprm;
    const osalStreamInterface *iface = OSAL_TLS_IFACE;
    os_int i;

    /* Just take first TLS connect downwards in object hierarchy
     */
    for (i = 0; i < connconf->n_connections; i++)
    {
        c = connconf->connection + i;
        if (c->listen || !c->down || c->transport != IOC_TLS_SOCKET) continue;

        con = ioc_initialize_connection(OS_NULL, &app_iocom);
        os_memclear(&conprm, sizeof(conprm));

        conprm.iface = iface;
        conprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS;
        conprm.parameters = connconf->connection->parameters;
        ioc_connect(con, &conprm);
        break;
    }
}
#endif



/**
****************************************************************************************************

  @brief Callback function to add dynamic device information.

  The app_info_callback() function is called when device information data is received from
  connection or when connection status changes.

  @param   handle Memory block handle.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void app_info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    /* If actual data received (not connection status change).
     */
    if (end_addr >= 0)
    {
        ioc_add_dynamic_info(handle, OS_FALSE);
    }
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
    os_char *mblk_name;
    iocHandle handle;

    switch (event)
    {
        case IOC_NEW_MEMORY_BLOCK:
            mblk_name = mblk->mblk_name;

            ioc_setup_handle(&handle, root, mblk);
            if (!os_strcmp(mblk_name, "info"))
            {
                ioc_add_callback(&handle, app_info_callback, OS_NULL);
            }
            else
            {
                ioc_memory_block_set_int_param(&handle,
                    IOC_MBLK_AUTO_SYNC_FLAG, OS_TRUE);
            }
            ioc_release_handle(&handle);
            break;

        case IOC_NEW_NETWORK:
            osal_trace2_str("IOC_NEW_NETWORK ", dnetwork->network_name);
            app_root_obj->launch_app(dnetwork->network_name);
            break;

        default:
            break;
    }
}
