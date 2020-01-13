/**

  @file    frank.c
  @brief   Frank controller using static IO device configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "frank.h"

/* The devicedir is here for testing only, take away.
 */
#include "devicedir.h"

iocRoot ioapp_root;
static FrankMain *frank_main;

/* IO device/network configuration.
 */
iocNodeConf ioapp_device_conf;

/* Forward referred static functions.
 */
static void info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,
    void *context);


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
    osPersistentParams persistentprm;
    iocDeviceId *device_id;
    // iocConnectionConfig *connconf;
    osalSecurityConfig *security;
    iocNetworkInterfaces *nics;
    const os_char *device_name = "frank";

    /* Initialize communication root and dymanic structure data root objects.
     * This demo uses dynamic signal configuration.
     */
    ioc_initialize_root(&ioapp_root);

    /* Initialize persistent storage and load device/network configuration and device/user
       account congiguration (persistent storage is typically either file system or
       micro-controller's flash). Defaults are set in network-defaults.json and
       in account-defaults.json.
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.device_name = device_name;
    os_persistent_initialze(&persistentprm);
    ioc_load_node_config(&ioapp_device_conf, ioapp_network_defaults, sizeof(ioapp_network_defaults));
    device_id = ioc_get_device_id(&ioapp_device_conf);
    // connconf = ioc_get_connection_conf(&ioapp_device_conf);

    ioc_set_iodevice_id(&ioapp_root, device_name, device_id->device_nr, device_id->network_name);
    ioc_initialize_dynamic_root(&ioapp_root);

    ioc_enable_user_authentication(&ioapp_root, ioc_authenticate, OS_NULL);

    /* Create frank main object
     */
    frank_main = new FrankMain(device_name, device_id->device_nr, device_id->network_name,
        device_id->publish);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(&ioapp_root, root_callback, OS_NULL);

    /* Setup network interface configuration and initialize transport library. This is
       partyly ignored if network interfaces are managed by operating system
       (Linux/Windows,etc),
     */
    nics = ioc_get_nics(&ioapp_device_conf);
    security = ioc_get_security_conf(&ioapp_device_conf);
    osal_tls_initialize(nics->nic, nics->n_nics, security);
    osal_serial_initialize();

    /* Ready to go, start listening for clients.
     */
    frank_main->listen_for_clients();
    // frank_main->connect_to_device();

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Loop function to be called repeatedly.

  The osal_loop() function doesn't maintains testing console to print out memory structures,
  but otherwise doesn's do anything.

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    osalStatus s;

    frank_main->run();

    os_sleep(50);
    s = io_device_console(&ioapp_root);

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
    ioc_set_root_callback(&ioapp_root, OS_NULL, OS_NULL);
    delete frank_main;

    ioc_release_root(&ioapp_root);
    osal_tls_shutdown();
    osal_serial_shutdown();
}


/**
****************************************************************************************************

  @brief Callback function to add dynamic device information.

  The info_callback() function is called when device information data is received from connection
  or when connection status changes.

  @param   mblk Pointer to the memory block object.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void info_callback(
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
  added or removed.

  @param   root Pointer to the root object.
  @param   event Either IOC_NEW_NETWORK, IOC_NEW_DEVICE or IOC_NETWORK_DISCONNECTED.
  @param   dnetwork Pointer to dynamic network object which has just been connected or is
           about to be removed.
  @param   mblk Pointer to memory block structure, OS_NULL if not available for the event.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void root_callback(
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
        /* Process "new dynamic memory block" callback.
         */
        case IOC_NEW_MEMORY_BLOCK:
            mblk_name = mblk->mblk_name;

            ioc_setup_handle(&handle, root, mblk);
            if (!os_strcmp(mblk_name, "info"))
            {
                ioc_add_callback(&handle, info_callback, OS_NULL);
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
            frank_main->launch_app(dnetwork->network_name);
            break;

        case IOC_NEW_DEVICE:
            osal_trace2_str("IOC_NEW_DEVICE ", mblk->device_name);
            break;

        case IOC_NETWORK_DISCONNECTED:
            osal_trace2("IOC_NETWORK_DISCONNECTED");
            break;

        default:
            break;
    }
}
