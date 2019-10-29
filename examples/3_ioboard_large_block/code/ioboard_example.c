/**

  @file    ioboard_example.c
  @brief   IO board example 3_ioboard_large_block.
  @author  Pekka Lehtikoski
  @version 1.1
  @date    11.7.2019

  3_ioboard_large_block example is to test IO board communication performance with large
  block transfers. I use it with wireshark to make sure that TCP_NODELAY/TCP_CORK options
  provice desired TCP block size and transfer timing.

  Example features:
  - No multithreading - single thread model used.
  - No dynamic memory allocation - static memory pool ioboard_pool used.
  - IO board connects to control computer through TCP socket - control computer listens for
    connections.
  - Data transfer synchronized precisely by ioc_receive() and ioc_send() calls - no
    "prm.auto_synchronization = OS_TRUE" -> IOC_AUTO_SYNC flags not set.
  - Relatively large 10k memory blocks and input memory block ioboard_fc is changed as quickly
    as computer can change it.
  - Unnanamed device, device name is empty string and device number is 0.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#include <stdlib.h> /* for rand() */

/* How this IO device and the control computer connect together. One of IOBOARD_CTRL_LISTEN_SOCKET,
   IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_LISTEN_SERIAL or IOBOARD_CTRL_CONNECT_SERIAL.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SOCKET

/* Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE or OSAL_TLS_IFACE defines.
 */
#define IOBOARD_STEAM_IFACE OSAL_SOCKET_IFACE

/* Maximum number of connections. Basically we need a single connection between IO board
   and control computer. We may want to allow two connections to listen for TCP socket
   for extra debugging connection. There are also other special cases when we need
   to have more than one connection.
 */
#define IOBOARD_MAX_CONNECTIONS (IOBOARD_CTRL_CON == IOBOARD_CTRL_LISTEN_SOCKET ? 2 : 1)

/* IO device's data memory blocks sizes in bytes. "TC" is abbreviation for "to controller"
   and sets size for ioboard_tc "IN" memory block. Similarly "FC" stands for "from controller"
   and ioboard_fc "OUT" memory block.
   Notice that minimum IO memory blocks size is sizeof(osalStaticMemBlock), this limit is
   imposed by static memory pool memory allocation.
 */
#define IOBOARD_TC_BLOCK_SZ 10000
#define IOBOARD_FC_BLOCK_SZ 10000

/* Allocate static memory pool for the IO board. We can do this even if we would be running
   on system with dynamic memory allocation, which is useful for testing micro-controller
   software in PC computer.
 */
static os_uchar
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        IOBOARD_TC_BLOCK_SZ, IOBOARD_FC_BLOCK_SZ)];


/**
****************************************************************************************************

  @brief IO board example.

  Send a lot to test data to evaluate communication trough-output.

  @return  None.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    ioboardParams prm;

    /* Initialize the socket library.
     */
    osal_socket_initialize(OS_NULL, 0);

    /* Set up parameters for the IO board. To connect multiple devices,
       either device number or name must differ.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = IOBOARD_STEAM_IFACE;
    prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.socket_con_str = "127.0.0.1"; /**************** SET IP ADDRESS HERE ***************/
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.send_block_sz = IOBOARD_TC_BLOCK_SZ;
    prm.receive_block_sz = IOBOARD_FC_BLOCK_SZ;
    prm.auto_synchronization = OS_FALSE;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);

    /* Start communication.
     */
    ioboard_start_communication(&prm);

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);
    return 0;
}


/**
****************************************************************************************************

  @brief Loop function to be called repeatedly.

  The osal_loop() function...
  IO board main loop, repeat forever (this example has no terminate condition).

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    int
        i,
        j,
        k;

    /* Keep the communication alive. The IO board uses one thread model, thus
       we need to call this function repeatedly.
     */
    ioc_run(&ioboard_communication);

    /* Received data fame up to date.
     */
    ioc_receive(&ioboard_fc);

    /* Write lot of random stuff to simulate vast number of inputs changing
       very quickly.
     */
    k = rand();
    for (i = 0; i<IOBOARD_TC_BLOCK_SZ/2; i++)
    {
        j = rand() % IOBOARD_TC_BLOCK_SZ;
        ioc_set16(&ioboard_tc, j, k);
        k += 7;
    }

    /* Send changes trough communication.
     */
    ioc_send(&ioboard_tc);

    return OSAL_SUCCESS;
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
    ioboard_end_communication();
    osal_socket_shutdown();
}
