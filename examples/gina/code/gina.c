/**

  @file    gina.c
  @brief   Gina IO board example featuring  IoT device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    1.11.2019

  Selecting transport
  IOBOARD_CTRL_CON define selects how this IO device connects to control computer. One of
  IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_CONNECT_TLS or IOBOARD_CTRL_CONNECT_SERIAL.

  Transport configuration
  Modify connection parameters here: These apply to different communication types
  Define EXAMPLE_TCP_SOCKET_PORT sets unsecured TCP socket port number
   to listen.
   Define EXAMPLE_TLS_SOCKET_PORT sets secured TCP socket port number
   to listen.
   Defines EXAMPLE_TLS_SERVER_CERT and EXAMPLE_TLS_SERVER_KEY set path
   to cerver certificate and key files.
   Define EXAMPLE_SERIAL_PORT: Serial port can be selected using Windows
   style using "COM1", "COM2"... These are mapped to hardware/operating system in device specific
   manner. On Linux port names like "ttyS30,baud=115200" or "ttyUSB0" can be also used.

  Number of connections
  The IOBOARD_MAX_CONNECTIONS sets maximum number of connections. IO board needs one connection.

  IO device's data memory blocks sizes in bytes. "TC" is abbreviation for "to controller"
  and sets size for ioboard_EXPORT "IN" memory block. Similarly "FC" stands for "from controller"
  and ioboard_IMPORT "OUT" memory block.
  Notice that minimum IO memory blocks size is sizeof(osalStaticMemBlock), this limit is
  imposed by static memory pool memory allocation.

  Allocate static memory pool for the IO board. We can do this even if we would be running
   on system with dynamic memory allocation, which is useful for testing micro-controller
   software in PC computer.


  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "gina.h"

/* Select socket, TLS or serial communication.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SOCKET

/* Transport parameters.
 */
#define EXAMPLE_IP_ADDRESS "192.168.1.220"
#define EXAMPLE_TLS_SERVER_CERT "/coderoot/eosal/extensions/tls/ssl-test-keys-and-certs/alice.crt"
#define EXAMPLE_TLS_SERVER_KEY "/coderoot/eosal/extensions/tls/ssl-test-keys-and-certs/alice.key"
#define EXAMPLE_SERIAL_PORT "COM3,baud=115200"

/* Maximum number of sockets, etc.
 */
#define IOBOARD_MAX_CONNECTIONS 1

/* Memory pool
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS, 
        SIGNAL_EXPORT_MBLK_SZ, SIGNAL_IMPORT_MBLK_SZ)];


/**
****************************************************************************************************

  @brief Set up the communication.

  X..
  @return  OSAL_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    ioboardParams prm;
    const osalStreamInterface *iface;

    /* Initialize the transport, socket, TLS, serial, etc..
     */
    osal_tls_initialize(OS_NULL, 0, OS_NULL);
    osal_serial_initialize();

    /* Set up parameters for the IO board. This is necessary since
       we are using static memory pool.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = iface;
    prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.socket_con_str = EXAMPLE_IP_ADDRESS;
    prm.serial_con_str = EXAMPLE_SERIAL_PORT;
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.send_block_sz = SIGNAL_EXPORT_MBLK_SZ;
    prm.receive_block_sz = SIGNAL_IMPORT_MBLK_SZ;
    prm.auto_synchronization = OS_FALSE;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);

    /* Start communication.
     */
    ioboard_start_communication(&prm);

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ioboard_IMPORT, ioboard_communication_callback, OS_NULL);

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
    /* Keep the communication alive. Here we use single thread model, thus we need to call
       this function repeatedly.
     */
    ioc_run(&ioboard_communication);

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
    osal_tls_shutdown();
    osal_serial_shutdown();
}


/**
****************************************************************************************************

  @brief Callback function when some communication data has changed.

  The ioboard_fc_callback function...

  @return  None.

****************************************************************************************************
*/
void ioboard_communication_callback(
    struct iocHandle *mblk,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context)
{
    int i;
#define N_LEDS 8
    os_char buf[N_LEDS];

    if (ioc_is_my_address(&signal_IMPORT.SEVEN_SEGMENT, start_addr, end_addr))
    {
        ioc_gets_array(&signal_IMPORT.SEVEN_SEGMENT, buf, N_LEDS);
        if (ioc_is_value_connected(signal_IMPORT.SEVEN_SEGMENT))
        {
            osal_console_write("7 segment data received\n");
            for (i = 0; i < N_LEDS; i++)
            {
                // digitalWrite(leds[s + i], buf[i] ? HIGH : LOW);
            }
        }
        else
        {
            // WE DO NOT COME HERE. SHOULD WE INVALIDATE WHOLE MAP ON DISCONNECT?
            osal_console_write("7 segment data DISCONNECTED\n");
        }
    }
}
