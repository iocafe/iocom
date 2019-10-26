/**

  @file    iocontroller.c
  @brief   IO controller example "iocontroller-listen".
  @author  Pekka Lehtikoski
  @version 1.0
  @date    27.6.2019

  iocontroller-listen example demonstrates a controller which listens TCP socket port for
  connections. The example relies on multithreading (IOC_CREATE_THREAD flag) and dynamic
  memory allocation, so it intented to run on windows/linux, not on a microcontroller.

   Example features:
  - Controller listens for TCP socket connections
  - It doesn't have any information about device, but memory blocks within controller
    are dynamically allocated (IOC_DYNAMIC_MBLKS flag).
  - The controller application receives information about new memory block as
    root_callback() function calls.
  - Used multithreading and dynamic memory allocation - needs RTOS, etc in micro-controller.
  - IO board connects to control computer through TCP socket - control computer listens for
    connections.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

/* Forward referred static functions.
 */
static void root_callback(
    iocRoot *root,
    iocConnection *con,
    iocHandle *mblk_handle,
    iocRootCallbackEvent event,
    void *context);

static void info_callback(
    iocHandle *mblk_handle,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context);


/**
****************************************************************************************************

  @brief IO controller example "iocontroller-listen".

  The osal_main() function listens for socket connections and dynamically creates memory blocks
  according to information received from the device.

  @return  None.

****************************************************************************************************
*/
os_int osal_main(
    os_int argc,
    os_char *argv[])
{
    iocRoot root;
    iocEndPoint *ep;
    iocEndPointParams epprm;

    /* Initialize the socket library and root structure.
     */
    osal_socket_initialize(OS_NULL, 0);
    ioc_initialize_root(&root);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(&root, root_callback, OS_NULL);

    /* Listen to socket port.
     */
    ep = ioc_initialize_end_point(OS_NULL, &root);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = OSAL_SOCKET_IFACE;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS;
    ioc_listen(ep, &epprm);

    /* Do something else.
     */
    while (OS_TRUE)
    {
        os_sleep(100);
    }

    /* End IO board communication, clean up and finsh with the socket library.
     */
    ioc_release_root(&root);
    osal_socket_shutdown();
    return 0;
}


/**
****************************************************************************************************

  @brief Callback from iocom root object.

  The root_callback() function is used to detect new dynamically allocated memory blocks.
  @return  None.

****************************************************************************************************
*/
static void root_callback(
    iocRoot *root,
    iocConnection *con,
    iocHandle *mblk_handle,
    iocRootCallbackEvent event,
    void *context)
{
    os_char text[128], mblk_name[IOC_NAME_SZ];

    switch (event)
    {
        /* Process "new dynamic memory block" callback.
         */
        case IOC_NEW_DYNAMIC_MBLK:
            ioc_memory_block_get_string_param(mblk_handle, IOC_MBLK_NAME, mblk_name, sizeof(mblk_name));

            os_strncpy(text, "Memory block ", sizeof(text));
            os_strncat(text, mblk_name, sizeof(text));
            os_strncat(text, " dynamically allocated\n", sizeof(text));
            osal_console_write(text);

            if (!os_strcmp(mblk_name, "INFO"))
            {
                ioc_add_callback(mblk_handle, info_callback, OS_NULL);
                ioc_memory_block_set_int_param(mblk_handle, IOC_MBLK_AUTO_SYNC_FLAG, OS_TRUE);
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

  @brief Callback function to print device info.

  The info_callback() function is called when device information data is received from connection
  or when connection status changes.

  @param   mblk_handle Memory block handle.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void info_callback(
    iocHandle *mblk_handle,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context)
{
    os_char buf[128];

    /* If actual data received (not connection status change), print the device information.
     */
    if (end_addr >= 0)
    {
        ioc_getstring(mblk_handle, 0, buf, sizeof(buf));
        osal_console_write(buf);
        osal_console_write("\n");
    }
}
