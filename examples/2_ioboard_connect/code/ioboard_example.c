/**

  @file    ioboard_example.c
  @brief   IO board example 2.
  @author  Pekka Lehtikoski
  @version 1.2
  @date    11.7.2019

  ioboard2 example demonstrates basic IO board with network communication. This example is
  kept minimalistic. An IO board typically has one memory block for the inputs and the one
  other for the outputs. Since this example doesn't use dynamic memory allocation or
  multithreading, thus it should run on any platform.

  Example features:
  - No multithreading - single thread model used.
  - No dynamic memory allocation - static memory pool ioboard_pool used.
  - Data transfer synchronized automatically "prm.auto_synchronization = OS_TRUE" when data
    is read or written - ioc_receive() and ioc_send() calls not needed.
  - Demonstrates device name, "MYDEV", and device number 1.
  - IO board connects to control computer through TCP socket - control computer listens for
    connections.
  - How this example IO device and control computer connect can be set by IOBOARD_CTRL_CON
    define. Set connection parameters according to used environment in prm.socket_con_str or
    in prm.serial_con_str. See code below.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

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

/* IO device's data transfer memory blocks sizes in bytes. Minimum IO memory block size
   is sizeof(osalStaticMemBlock).
 */
#define IOBOARD_EXPORT_MBLK_SZ 256
#define IOBOARD_IMPORT_MBLK_SZ 256

/* Allocate static memory pool for the IO board. We can do this even if we would be running
   on system with dynamic memory allocation, which is useful for testing micro-controller
   software in PC computer.
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        IOBOARD_EXPORT_MBLK_SZ, IOBOARD_IMPORT_MBLK_SZ)];

static os_int
        prev_command;


/**
****************************************************************************************************

  @brief IO board example 2.

  The very basic IO board functionality.

  @return  None.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    ioboardParams prm;

    /* Initialize the socket and serial communication libraries.
     */
    osal_socket_initialize(OS_NULL, 0);
    osal_serial_initialize();

    /* Set up parameters for the IO board. To connect multiple devices,
       either device number or name must differ.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = IOBOARD_STEAM_IFACE;
    prm.device_name = "MYDEV";
    prm.device_nr = 1;
	prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.socket_con_str = "127.0.0.1";
    prm.serial_con_str = "COM3,baud=115200";
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.send_block_sz = IOBOARD_EXPORT_MBLK_SZ;
    prm.receive_block_sz = IOBOARD_IMPORT_MBLK_SZ;
    prm.auto_synchronization = OS_TRUE;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);

    /* Start communication.
     */
    ioboard_start_communication(&prm);

    /* Clear globals
     */
    prev_command = 0x10000;

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

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    os_int command;

    /* Keep the communication alive. The IO board uses single thread model, thus
       we need to call this function repeatedly.
     */
    ioc_run(&ioboard_communication);

    /* If we receive a "command" as 16 bit value in address 2. The command could start
       some operation of IO board. The command is eached back in address 2 to allow
       controller to know that command has been regognized.
     */
    command = ioc_getp_short(&ioboard_import, 2);
    if (command != prev_command)
    {
        if (command == 1)
        {
            osal_console_write("Command 1, working on it.\n");
        }
        prev_command = command;
        ioc_setp_short(&ioboard_export, 2, command);
    }

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
    osal_serial_shutdown();
}
