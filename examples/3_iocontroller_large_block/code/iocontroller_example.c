/**

  @file    iocontroller.c
  @brief   IO controller example "3_iocontroller_large_block".
  @author  Pekka Lehtikoski
  @version 1.0
  @date    29.6.2019

  The 3_iocontroller_large_block is controller part to run with 3_ioboard_large_block example
  to test IO board performance with large block transfers.
  I use it with wireshark to make sure that TCP_NODELAY/TCP_CORK oprions provice desired
  TCP block size and transfer timing.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

static os_long callback_count;

/* Forward referred static functions.
 */
static void iocontroller_callback(
    struct iocMemoryBlock *mblk,
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
    iocMemoryBlockParams blockprm;
    iocMemoryBlock *inputs, *outputs;
    os_char text[128], nbuf[32];

    const int
        input_block_sz = 10000,
        output_block_sz = 10000;

    /* Initialize the socket library and root structure.
     */
    osal_socket_initialize(OS_NULL, 0);
    ioc_initialize_root(&root);

    /* Create memory blocks for inputs and outputs.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.mblk_nr = IOC_INPUT_MBLK;
    blockprm.nbytes = input_block_sz;
    blockprm.flags = IOC_TARGET|IOC_AUTO_RECEIVE|IOC_ALLOW_RESIZE;
    inputs = ioc_initialize_memory_block(OS_NULL, &root, &blockprm);
    blockprm.mblk_nr = IOC_OUTPUT_MBLK;
    blockprm.nbytes = output_block_sz;
    blockprm.flags = IOC_SOURCE|IOC_AUTO_SEND|IOC_ALLOW_RESIZE;
    outputs = ioc_initialize_memory_block(OS_NULL, &root, &blockprm);

    /* Set callback to count received data packages.
     */
    ioc_add_callback(inputs, iocontroller_callback, OS_NULL);

    /* Listen to socket port.
     */
    ep = ioc_initialize_end_point(OS_NULL, &root);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = OSAL_SOCKET_IFACE;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD;
    ioc_listen(ep, &epprm);

    /* Do something else.
     */
    while (OS_TRUE)
    {
        os_sleep(1000);

        os_strncpy(text, "callback count: ", sizeof(text));
        osal_int_to_string(nbuf, sizeof(nbuf), callback_count);
        os_strncat(text, nbuf, sizeof(text));
        os_strncat(text, "\n", sizeof(text));
        osal_console_write(text);
    }

    /* End IO board communication, clean up and finsh with the socket library.
     */
    ioc_release_root(&root);
    osal_socket_shutdown();
    return 0;
}


/**
****************************************************************************************************

  @brief Callback function.

  The iocontroller_callback() function is called when changed data is received from connection
  or when connection status changes.

  No heavy processing or printing data should be placed in callback. The callback should return
  quickly. The reason is that the communication must be able to process all data it receives,
  and delays here will cause connection buffers to fill up, which at wors could cause time shift
  like delay in communication.

  @param   mblk Pointer to the memory block object.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void iocontroller_callback(
    struct iocMemoryBlock *mblk,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context)
{
    callback_count++;
}
