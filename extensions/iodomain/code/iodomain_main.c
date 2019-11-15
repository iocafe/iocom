/**

  @file    iocdomain_main.c
  @brief   IO domain controller library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.9.2019

  The domain controller library listens for connections from IO devices and other IO comain
  controllers. Once an IO device connects to domain, memory maps for the device are created.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iodomain/iodomain.h"


// IODOMAIN BINDS DATA. OF TWO ITEMS DO HAVE SAME GLOBAL NAME THEY ARE BIUND TOGETHER???


/* Forward referred static functions.
 */
static void root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocHandle *handle,
    iocRootCallbackEvent event,
    void *context);

static void info_callback(
    struct iocHandle *mblk,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);


/**
****************************************************************************************************

  @brief Initialize the IO domain data structure.

  The iodomain_initialize() initializes empty IO domain data structure.

  @param   iodomain Pointer to IO domain class structure to initialize.
  @return  None.

****************************************************************************************************
*/
void iodomain_initialize(
    iodomainClass *iodomain)
{
    os_memclear(iodomain, sizeof(iodomainClass));

    /* Initialize the root structure.
     */
    ioc_initialize_root(&iodomain->root);
}


/**
****************************************************************************************************

  @brief Finished with IO domain. Clean up.

  The iodomain_shutdown() function releases all resources allocated for the IO domain.
  @return  None.

****************************************************************************************************
*/
void iodomain_shutdown(
    iodomainClass *iodomain)
{
    /* End IO board communication, clean up and finsh with the socket library.
     */
    ioc_release_root(&iodomain->root);
}


/**
****************************************************************************************************

  @brief Start the IO domain.

  The iodomain_start() starts the listening for incoming connections from the IO devices according
  to node configuration information.

  @param   iodomain Pointer to IO domain class structure.
  @param   node Pointer to network topology configuration for the node.
  @return  None.

****************************************************************************************************
*/
void iodomain_start(
    iodomainClass *iodomain,
    iotopologyNode *node)
{
    iocEndPoint *ep;
    iocEndPointParams epprm;

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    ioc_set_root_callback(&iodomain->root, root_callback, iodomain);

    /* Listen to socket port.
     */
    ep = ioc_initialize_end_point(OS_NULL, &iodomain->root);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = OSAL_SOCKET_IFACE;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS;
    ioc_listen(ep, &epprm);
}


/**
****************************************************************************************************

  @brief Callback from iocom root object.

  The root_callback() function is used to detect new dynamically allocated memory blocks.

  @param   handle Memory block handle.
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
            ioc_memory_block_get_string_param(handle, IOC_MBLK_NAME, mblk_name, sizeof(mblk_name));

            os_strncpy(text, "Memory block ", sizeof(text));
            os_strncat(text, mblk_name, sizeof(text));
            os_strncat(text, " dynamically allocated\n", sizeof(text));
            osal_console_write(text);

            if (!os_strcmp(mblk_name, "info"))
            {
                ioc_add_callback(handle, info_callback, OS_NULL);
                ioc_memory_block_set_int_param(handle, IOC_MBLK_AUTO_SYNC_FLAG, OS_TRUE);
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
    os_char buf[128];

    /* If actual data received (not connection status change), print the device information.
     */
    if (end_addr >= 0)
    {
        ioc_getp_str(handle, 0, buf, sizeof(buf));
        osal_console_write(buf);
        osal_console_write("\n");
    }
}
