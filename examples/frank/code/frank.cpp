/**

  @file    frank.c
  @brief   Frank controller using static IO device configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.11.2019

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

iocRoot frank_root;
static FrankMain *frank_main;


static void root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocHandle *handle,
    iocRootCallbackEvent event,
    void *context);

static void info_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void network_callback(
    struct iocDynamicRoot *droot,
    struct iocDynamicNetwork *dnetwork,
    iocDynamicNetworkEvent event,
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
    /* Initialize the transport, socket, TLS, serial, etc..
     */
    osal_tls_initialize(OS_NULL, 0, OS_NULL);
    osal_serial_initialize();

    /* Initialize communication root and dymanic structure data root objects.
     * This demo uses dynamic signal configuration.
     */
    ioc_initialize_root(&frank_root);
    frank_root.droot = ioc_initialize_dynamic_root();

    /* Create frank main object
     */
    frank_main = new FrankMain;

    /* Set callback function to receive information about created or removed dynamic IO networks.
     */
    ioc_set_dnetwork_callback(frank_root.droot, network_callback, OS_NULL);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(&frank_root, root_callback, OS_NULL);

    /* Ready to go, start listening for clients.
     */
    frank_main->listen_for_clients();

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
    os_sleep(100);
    return io_device_console(&frank_root);
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
    ioc_set_dnetwork_callback(frank_root.droot, OS_NULL, OS_NULL);
    ioc_set_root_callback(&frank_root, OS_NULL, OS_NULL);
    delete frank_main;

    ioc_release_root(&frank_root);
    osal_tls_shutdown();
    osal_serial_shutdown();
}


/**
****************************************************************************************************

  @brief Callback from iocom root object.

  The root_callback() function is used to detect new dynamically allocated memory blocks.

  @param   root Root object.
  @param   con Connection.
  @param   handle Memory block handle.
  @param   event Why the callback?
  @param   context Not used.
  @return  None.

****************************************************************************************************
*/
static void root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocHandle *handle,
    iocRootCallbackEvent event,
    void *context)
{
    os_char text[128], mblk_name[IOC_NAME_SZ];

    switch (event)
    {
        /* Process "new dynamic memory block" callback.
         */
        case IOC_NEW_DYNAMIC_MBLK:
            ioc_memory_block_get_string_param(handle, IOC_MBLK_NAME,
                mblk_name, sizeof(mblk_name));

            os_strncpy(text, "Memory block ", sizeof(text));
            os_strncat(text, mblk_name, sizeof(text));
            os_strncat(text, " dynamically allocated\n", sizeof(text));
            osal_console_write(text);

            if (!os_strcmp(mblk_name, "info"))
            {
                ioc_add_callback(handle, info_callback, OS_NULL);
            }
            break;

        /* Ignore unknown callbacks. More callback events may be introduced in future.
         */
        default:
            break;
    }
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
        ioc_add_dynamic_info(frank_root.droot, handle);
    }
}


/**
****************************************************************************************************

  @brief Callback when dynamic IO network has been connected or disconnected.

  The info_callback() function is called when device information data is received from connection
  or when connection status changes.

  @param   droot Pointer to the dynamic configuration root object.
  @param   dnetwork Pointer to dynamic network object which has just been connected or is
           about to be removed.
  @param   event Either IOC_NEW_DYNAMIC_NETWORK or IOC_DYNAMIC_NETWORK_REMOVED.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void network_callback(
    struct iocDynamicRoot *droot,
    struct iocDynamicNetwork *dnetwork,
    iocDynamicNetworkEvent event,
    void *context)
{
    switch (event)
    {
        case IOC_NEW_DYNAMIC_NETWORK:
            osal_debug_error("IOC_NEW_DYNAMIC_NETWORK");
            frank_main->launch_app(dnetwork->network_name);
            break;

        case IOC_DYNAMIC_NETWORK_REMOVED:
            osal_debug_error("IOC_DYNAMIC_NETWORK_REMOVED");
            break;
    }
}
