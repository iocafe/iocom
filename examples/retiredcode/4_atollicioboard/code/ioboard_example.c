/**

  @file    ioboard_example.c
  @brief   IO board example 4_ioboard_test.
  @author  Pekka Lehtikoski
  @version 1.1
  @date    8.1.2020

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

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#define OSAL_INCLUDE_METAL_HEADERS
#include "iocom.h"
#include "gpio.h"

/* How this IO device and the control computer connect together. One of IOBOARD_CTRL_LISTEN_SOCKET,
   IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_LISTEN_SERIAL or IOBOARD_CTRL_CONNECT_SERIAL.
   This can be overridden in build.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_LISTEN_SERIAL

/* Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE or OSAL_TLS_IFACE defines.
 */
#define IOBOARD_STEAM_IFACE OSAL_SERIAL_IFACE

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

typedef struct
{
    GPIO_TypeDef *port;
    os_ushort pin;
}
MyPinDef;

#ifdef STM32L476xx
#define N_LEDS 8
const MyPinDef leds[]
   = {{GPIOB, GPIO_PIN_8},
      {GPIOA, GPIO_PIN_10},
      {GPIOB, GPIO_PIN_3},
      {GPIOB, GPIO_PIN_5},
      {GPIOB, GPIO_PIN_4},
      {GPIOA, GPIO_PIN_8},
      {GPIOA, GPIO_PIN_9},
      {GPIOC, GPIO_PIN_7}};
#endif


static void ioboard_callback(
    struct iocMemoryBlock *mblk,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);


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
    os_int
        command,
        prev_command  = 0x10000,
        state = 0;

    ioboardParams
        prm;

    os_timer
        ti;

    /* Initialize the socket and serial port libraries.
     */
    osal_socket_initialize(OS_NULL, 0);
    osal_serial_initialize();

    /* Set up parameters for the IO board. This is necessary since
       we are using static memory pool.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = IOBOARD_STEAM_IFACE;
    prm.ctrl_type = IOBOARD_CTRL_CON;
    //prm.device_name = "atollic";
    //prm.device_nr = 1;
    //prm.network_name = "iocafenet";
    prm.socket_con_str = "127.0.0.1";
    prm.socket_con_str = "192.168.1.229";
    //prm.serial_con_str = "COM5,baud=115200";
    //prm.serial_con_str = "ttyS31,baud=115200";
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
    ioc_add_callback(&ioboard_imp, ioboard_callback, OS_NULL);

    os_get_timer(&ti);

    /* IO board main loop, repeat forever (this example has no terminate condition).
     */
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
        command = ioc_getp_short(&ioboard_imp, 2);
        if (command != prev_command) {
            if (command == 1) {
                // osal_console_write("Command 1, working on it.\n");
            }
            prev_command = command;
            // ioc_setp_short(&ioboard_exp, 2, command);
        }

        if (os_elapsed(&ti, 100))
        {
            HAL_GPIO_WritePin(leds[3].port, leds[3].pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            state = !state;
            os_get_timer(&ti);
        }
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

  @brief Callback function.

  The iocontroller_callback() function is called when changed data is received from connection
  or when connection status changes. This is used to control 7 segment display LEDs in my
  STM32L476 test.

  No heavy processing or printing data should be placed in callback. The callback should return
  quickly. The reason is that the communication must be able to process all data it receives,
  and delays here will cause connection buffers to fill up, which at worst could cause time shift
  like delay in communication.

  @param   mblk Pointer to the memory block object.
  @param   start_addr Address of first changed byte.
  @param   end_addr Address of the last changed byte.
  @param   flags Reserved  for future.
  @param   context Application specific pointer passed to this callback function.

  @return  None.

****************************************************************************************************
*/
static void ioboard_callback(
    struct iocMemoryBlock *mblk,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    os_int s, e, i, n;
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
          HAL_GPIO_WritePin(leds[s + i].port, leds[s + i].pin, buf[i] ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
    }
}
