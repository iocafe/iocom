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
#include "iodomain.h"


IODOMAIN BINDS DATA. OF TWO ITEMS DO HAVE SAME GLOBAL NAME THEY ARE BIUND TOGETHER???


/* Forward referred static functions.
 */
static void root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocMemoryBlock *mblk,
    iocRootCallbackEvent event,
    void *context);

static void info_callback(
    struct iocMemoryBlock *mblk,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context);


/**
****************************************************************************************************

  @brief Set up IO domain.

  The iodomain_setup() startus the IO domain listening for TLS socket connections.

  The iofomain function listens for socket connections and dynamically creates memory blocks
  according to information received from the device.

  @return  None.

****************************************************************************************************
*/
void iodomain_setup(
    iodomainParams *prm)
{
    iocRoot root;
    iocEndPoint *ep;
    iocEndPointParams epprm;

    /* Initialize the socket library and root structure.
     */
    osal_socket_initialize();
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
}


/**
****************************************************************************************************

  @brief Callback from iocom root object.

  The root_callback() function is used to detect new dynamically allocated memory blocks.
  @return  None.

****************************************************************************************************
*/
static void root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocMemoryBlock *mblk,
    iocRootCallbackEvent event,
    void *context)
{
    os_char text[128], mblk_name[IOC_NAME_SZ];

    switch (event)
    {
        /* Process "new dynamic memory block" callback.
         */
        case IOC_NEW_DYNAMIC_MBLK:
             ioc_get_memory_block_param(mblk, IOC_MBLK_NAME, mblk_name, sizeof(mblk_name));

            os_strncpy(text, "Memory block ", sizeof(text));
            os_strncat(text, mblk_name, sizeof(text));
            os_strncat(text, " dynamically allocated\n", sizeof(text));
            osal_console_write(text);

            if (!os_strcmp(mblk_name, "INFO"))
            {
                ioc_add_callback(mblk, info_callback, OS_NULL);
                ioc_set_flag(mblk, IOC_AUTO_RECEIVE, OS_TRUE);
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
    struct iocMemoryBlock *mblk,
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
        ioc_getstring(mblk, 0, buf, sizeof(buf));
        osal_console_write(buf);
        osal_console_write("\n");
    }
}
