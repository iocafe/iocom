/**

  @file    ioboard_example.c
  @brief   IO board example 4_ioboard_test.
  @author  Pekka Lehtikoski
  @version 1.2
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
#define IOCOM_IOBOARD
#include "iocom.h"

/* How this IO device and the control computer connect together. One of IOBOARD_CTRL_LISTEN_SOCKET,
   IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_LISTEN_SERIAL, IOBOARD_CTRL_LISTEN_TLS.
   IOBOARD_CTRL_CONNECT_TLS or IOBOARD_CTRL_CONNECT_SERIAL.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_LISTEN_SOCKET

/* Modify connection parameters here: These apply to different communication types
   Define EXAMPLE_TCP_SOCKET_PORT sets unsecured TCP socket port number
   to listen.
   Define EXAMPLE_TLS_SOCKET_PORT sets secured TCP socket port number
   to listen.
   Defines EXAMPLE_TLS_SERVER_CERT and EXAMPLE_TLS_SERVER_KEY set path
   to cerver certificate and key files.
   Define EXAMPLE_SERIAL_PORT: Serial port can be selected using Windows
   style using "COM1", "COM2"... These are mapped to hardware/operating system in device specific
   manner. On Linux port names like "ttyS30,baud=115200" or "ttyUSB0" can be also used.
 */
#define EXAMPLE_IP_ADDRESS "192.168.1.220"
#define EXAMPLE_TCP_SOCKET_PORT IOC_DEFAULT_SOCKET_PORT_STR
#define EXAMPLE_TLS_SOCKET_PORT IOC_DEFAULT_TLS_PORT_STR
#define EXAMPLE_TLS_SERVER_CERT "/coderoot/eosal/extensions/tls/keys-and-certs/myhome.crt"
#define EXAMPLE_TLS_SERVER_KEY "/coderoot/eosal/extensions/tls/keys-and-certs/secret/myhome.key"
#define EXAMPLE_SERIAL_PORT "COM3,baud=115200"

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
    os_int
        prev_nro_connections,
        prev_drop_count,
        prev_command;
}
MyAppContext;

/* Application context. This needs to exist as long as application runs.
 */
static MyAppContext ioboard_app_context;

#define N_LEDS 8

/* Here I create signal structures from C code by hand. Code to create these can
   be also generated from XML by script.
 */
static iocSignal my_tc_count = {20, 1, OS_SHORT, &ioboard_exp};
static os_short my_signal_count;
static os_timer my_signal_timer;

static iocSignal my_fc_7_segments = {0, N_LEDS, OS_BOOLEAN, &ioboard_imp};


/* Static function prototypes.
 */
static void ioboard_fc_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

/* static void ioboard_show_communication_status(
    MyAppContext *acontext); */


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
    osalWifiNetwork wifi[2];
    const osalStreamInterface *iface;

    /* Setup network interface configuration for micro-controller environment. This is ignored
       if network interfaces are managed by operating system (Linux/Windows,etc), or if we are
       connecting trough wired Ethernet. If only one subnet, set wifi_net_name_1.
     */
    os_memclear(&wifi, sizeof(osalWifiNetwork));
    wifi[0].wifi_net_name = "julian";
    wifi[0].wifi_net_password = "mysecret";
    wifi[1].wifi_net_name = "bean24";
    wifi[1].wifi_net_password = "mysecret";

    /* Initialize the underlying transport library. Never call boath osal_socket_initialize()
       and osal_tls_initialize(). These use the same underlying library.
       Set up iface to point correct transport interface and set parameters to configure it.
       Set also flags for communication protocol.
     */
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_SOCKET
  #if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    static osalSecurityConfig tlsprm = {EXAMPLE_TLS_SERVER_CERT, EXAMPLE_TLS_SERVER_KEY};
    osal_tls_initialize(OS_NULL, 0, wifi, 2, &tlsprm);
    iface = OSAL_TLS_IFACE;
  #else
    osal_socket_initialize(OS_NULL, 0, wifi, 2);
    iface = OSAL_SOCKET_IFACE;
  #endif
#else
    osal_serial_initialize();
    iface = OSAL_SERIAL_IFACE;
#endif

    /** Clear global variables.
     */
    os_memclear(&ioboard_app_context, sizeof(ioboard_app_context));
    ioboard_app_context.prev_command = 0x10000;

    /* Set up parameters for the IO board. This is necessary since
       we are using static memory pool.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = iface;
    prm.ctrl_type = IOBOARD_CTRL_CON;
//    prm.device_name = "ulle";
//    prm.device_nr = 1;
    //prm.network_name = "iocafenet";
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    prm.socket_con_str = EXAMPLE_IP_ADDRESS ":" EXAMPLE_TLS_SOCKET_PORT;
#else
    prm.socket_con_str = EXAMPLE_IP_ADDRESS ":" EXAMPLE_TCP_SOCKET_PORT;
#endif
    prm.serial_con_str = EXAMPLE_SERIAL_PORT;
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.exp_mblk_sz = IOBOARD_EXPORT_MBLK_SZ;
    prm.imp_mblk_sz = IOBOARD_IMPORT_MBLK_SZ;
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
    ioc_add_callback(&ioboard_imp, ioboard_fc_callback, OS_NULL);

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(&ioboard_app_context);
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
    os_int
        command;

    MyAppContext
        *acontext = (MyAppContext*)app_context;

    /* Keep the communication alive. The IO board uses one thread model, thus
       we need to call this function repeatedly.
     */
    ioc_run(&ioboard_root);

    /* If we receive a "command" as 16 bit value in address 2. The command could start
       some operation of IO board. The command is eached back in address 2 to allow
       controller to know that command has been regognized.
     */
    /* OBSOLETE, NEEDS REWRITING
     * command = ioc_getp_short(&ioboard_imp, 2);
    if (command != acontext->prev_command) {
        if (command == 1) {
            osal_console_write("Command 1, working on it.\n");
        }
        acontext->prev_command = command;
        ioc_setp_short(&ioboard_exp, 2, command);
    } */

    /* Send periodic signal to controller.
     */
    if (os_has_elapsed(&my_signal_timer, 2000))
    {
        os_get_timer(&my_signal_timer);
        ioc_set_ext(&my_tc_count, ++my_signal_count, OSAL_STATE_CONNECTED);
    }

    /* ioboard_show_communication_status(acontext); */

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
}


/**
****************************************************************************************************

  @brief Callback function when some communication data has changed.

  The ioboard_fc_callback function...

  @return  None.

****************************************************************************************************
*/
static void ioboard_fc_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    os_int i;
    os_char buf[N_LEDS], sb;

    if (ioc_is_my_address(&my_fc_7_segments, start_addr, end_addr))
    {
        sb = ioc_get_array(&my_fc_7_segments, buf, N_LEDS);
        if (sb & OSAL_STATE_CONNECTED)
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


/**
****************************************************************************************************

  @brief Show connection status.

  Every time a socket connects or disconnects to this "IO board", this function prints number
  of connected sockets and how many times a socket has been dropped (global count).

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  None.

****************************************************************************************************
*/
/*
 * OBSOLETE, NEEDS REWRITING
static void ioboard_show_communication_status(
    MyAppContext *acontext)
{
    os_int
        nro_connections,
        drop_count;

    os_char
        nbuf[32];

    nro_connections = ioc_gets_short(&ioboard_imp, IOC_NRO_CONNECTED_STREAMS);
    drop_count = ioc_getp_int(&ioboard_imp, IOC_CONNECTION_DROP_COUNT);
    if (nro_connections != acontext->prev_nro_connections ||
        drop_count != acontext->prev_drop_count)
    {
        osal_console_write("nro connections = ");
        osal_int_to_str(nbuf, sizeof(nbuf), nro_connections);
        osal_console_write(nbuf);
        osal_console_write(", drop count = ");
        osal_int_to_str(nbuf, sizeof(nbuf), drop_count);
        osal_console_write(nbuf);
        osal_console_write("\n");

        acontext->prev_nro_connections = nro_connections;
        acontext->prev_drop_count = drop_count;
    }
}
*/
