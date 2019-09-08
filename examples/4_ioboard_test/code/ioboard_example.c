/**

  @file    ioboard_example.c
  @brief   IO board example 4_ioboard_test.
  @author  Pekka Lehtikoski
  @version 1.1
  @date    11.7.2019

  The 4_ioboard_test example demonstrates basic IO board with network communication.
  Implementation doesn't use dynamic memory allocation or multithreading, thus it should run on
  any platform.

  Example features:
  - Testing connection status.
  - No multithreading - single thread model used.
  - No dynamic memory allocation - static memory pool ioboard_pool used.
  - Data transfer synchronized automatically "prm.auto_synchronization = OS_TRUE" when data
    is read or written - ioc_receive() and ioc_send() calls not needed.
  - Unnamed IO device with device number 0.
  - IO board listens for TCP socket connection from control computer.
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
   This can be overridden in build.
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
#define IOBOARD_TC_BLOCK_SZ 256
#define IOBOARD_FC_BLOCK_SZ 256

/* Allocate static memory pool for the IO board. We can do this even if we would be running
   on system with dynamic memory allocation, which is useful for testing micro-controller
   software in PC computer.
 */
static os_uchar
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS, 
		IOBOARD_TC_BLOCK_SZ, IOBOARD_FC_BLOCK_SZ)];

static int
    prev_nro_connections,
    prev_drop_count;

/* Static function prototypes.
 */
static void ioboard_show_communication_status(void);

#define N_LEDS 8

static void ioboard_callback(
    struct iocMemoryBlock *mblk,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context)
{
    int s, e, i, n;
    os_uchar buf[N_LEDS];

    /* Get connection status changes.
     */
    if (end_addr >= 0 && start_addr < N_LEDS)
    {
        s = start_addr;
        e = end_addr;
        if (s < 0) s = 0;
        if (e >= N_LEDS) e = N_LEDS - 1;
        n = e - s + 1;

        ioc_read(mblk, s, buf, n);
        for (i = 0; i<n; i++)
        {
          // digitalWrite(leds[s + i], buf[i] ? HIGH : LOW);
        }
    }
}

/**
****************************************************************************************************

  @brief IO board example 2.

  The very basic IO board functionality.

  @return  None.

****************************************************************************************************
*/
os_int osal_main(
    os_int argc,
    os_char *argv[])
{
    os_int
        command,
        prev_command;

    ioboardParams
        prm;

    /* Initialize the socket and serial port libraries.
     */
    osal_socket_initialize();
    osal_serial_initialize();

    /** Clear global variables.
     */
    prev_nro_connections = 0;
    prev_drop_count = 0;

    /* Set up parameters for the IO board. This is necessary since
       we are using static memory pool.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = IOBOARD_STEAM_IFACE;
    prm.ctrl_type = IOBOARD_CTRL_CON;
//    prm.socket_con_str = "127.0.0.1:" IOC_DEFAULT_SOCKET_PORT_STR;
//    prm.socket_con_str = "192.168.1.221:" IOC_DEFAULT_SOCKET_PORT_STR;
    prm.socket_con_str = "192.168.1.220:55555";
    prm.socket_con_str = "45.26.154.177:55555";
    //prm.serial_con_str = "COM5,baud=115200";
    //prm.serial_con_str = "ttyS31,baud=115200";
    prm.serial_con_str = "COM3,baud=115200";
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.send_block_sz = IOBOARD_TC_BLOCK_SZ;
    prm.receive_block_sz = IOBOARD_FC_BLOCK_SZ;
    prm.auto_synchronization = OS_TRUE;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);

    /* Start communication.
     */
    ioboard_start_communication(&prm);
#if IOBOARD_CTRL_CON == IOBOARD_CTRL_LISTEN_SOCKET
    osal_console_write("Listening TCP port ");
    osal_console_write(prm.socket_con_str);
    osal_console_write("\n");
#endif
#if IOBOARD_CTRL_CON == IOBOARD_CTRL_LISTEN_SERIAL
    osal_console_write("Listening serial port ");
    osal_console_write(prm.serial_con_str);
    osal_console_write("\n");
#endif

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ioboard_fc, ioboard_callback, OS_NULL);

    /* IO board main loop, repeat forever (this example has no terminate condition).
     */
    prev_command = 0x10000;
    while (!osal_console_read())
    {
        /* Keep the communication alive. The IO board uses one thread model, thus
           we need to call this function repeatedly.
         */
        ioc_run(&ioboard_communication);

        /* If we receive a "command" as 16 bit value in address 2. The command could start 
           some operation of IO board. The command is eached back in address 2 to allow 
           controller to know that command has been regognized.
         */
        command = ioc_get16(&ioboard_fc, 2);
// command = 0;
        if (command != prev_command) {
            if (command == 1) {
                osal_console_write("Command 1, working on it.\n");
            }
            prev_command = command;
            ioc_set16(&ioboard_tc, 2, command);
        }

        ioboard_show_communication_status();
    }

    /* End IO board communication, clean up and finsh with the socket and serial port libraries.
       On real IO device we may not need to take care about this, since these are often shut down
       only by turning or power or by microcontroller reset.
     */

    ioboard_end_communication();
    osal_socket_shutdown();
    osal_serial_shutdown();
    return 0;
}


/**
****************************************************************************************************

  @brief Show connection status.

  Every time a socket connects or disconnects to this "IO board", this function prints number
  of connected sockets and how many times a socket has been dropped (global count).

  @return  None.

****************************************************************************************************
*/
static void ioboard_show_communication_status(void)
{
    int
        nro_connections,
        drop_count;

    char
        nbuf[32];

    nro_connections = ioc_get16(&ioboard_fc, IOC_NRO_CONNECTED_STREAMS);
    drop_count = ioc_get32(&ioboard_fc, IOC_CONNECTION_DROP_COUNT);
    if (nro_connections != prev_nro_connections ||
        drop_count != prev_drop_count)
    {
        osal_console_write("nro connections = ");
        osal_int_to_string(nbuf, sizeof(nbuf), nro_connections);
        osal_console_write(nbuf);
        osal_console_write(", drop count = ");
        osal_int_to_string(nbuf, sizeof(nbuf), drop_count);
        osal_console_write(nbuf);
        osal_console_write("\n");

        prev_nro_connections = nro_connections;
        prev_drop_count = drop_count;
    }
}
