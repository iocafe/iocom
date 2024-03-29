/**

  @file    iocontroller.c
  @brief   IO controller example 4_iocontroller_test.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The 4_iocontroller_test example controls 7 segment LED display. It connects to the IO board
  through TCP socket or serial port.

  This example assumes one memory block for exp and the other for imp. It uses both
  supports dynamic memory allocation and multithreading, thus this example cannot be used in
  most microcontrollers.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/* Connection types.
 */
#define EXAMPLE_USE_TCP_SOCKET 0
#define EXAMPLE_USE_TLS_SOCKET 1
#define EXAMPLE_USE_SERIAL_PORT 2

/* Select how to connect: TCP socket, TLS socket (OpenSSL, etc) or serial port.
 * EXAMPLE_USE_TCP_SOCKET, EXAMPLE_USE_TLS_SOCKET or EXAMPLE_USE_SERIAL_PORT
 */
#define MY_TRANSPORT EXAMPLE_USE_TCP_SOCKET

/* Modify connection parameters here: These apply to different communication types
   EXAMPLE_USE_TCP_SOCKET: Define EXAMPLE_TCP_SOCKET_PORT sets unsecured TCP socket port number
   to listen.
   EXAMPLE_USE_TLS_SOCKET: Define EXAMPLE_TLS_SOCKET_PORT sets secured TCP socket port number
   to listen.
   EXAMPLE_USE_TLS_SOCKET: Defines EXAMPLE_TLS_SERVER_CERT and EXAMPLE_TLS_SERVER_KEY set path
   to cerver certificate and key files.
   EXAMPLE_USE_SERIAL_PORT, define EXAMPLE_SERIAL_PORT: Serial port can be selected using Windows
   style using "COM1", "COM2"... These are mapped to hardware/operating system in device specific
   manner. On Linux port names like "ttyS30,baud=115200" or "ttyUSB0" can be also used.
 */
#define EXAMPLE_IP_ADDRESS "192.168.1.119"
#define EXAMPLE_TCP_SOCKET_PORT IOC_DEFAULT_SOCKET_PORT_STR
#define EXAMPLE_TLS_SOCKET_PORT IOC_DEFAULT_TLS_PORT_STR
#define EXAMPLE_TLS_SERVER_CERT "/coderoot/eosal/extensions/tls/keys-and-certs/myhome.crt"
#define EXAMPLE_TLS_SERVER_KEY "/coderoot/eosal/extensions/tls/keys-and-certs/secret/myhome.key"
#define EXAMPLE_SERIAL_PORT "COM3,baud=115200"

/* List of connection roles. Either listen for or connect socket.
 */
#define EXAMPLE_LISTEN 0
#define EXAMPLE_CONNECT 1

/* Select connect role here
 */
#define MY_ROLE EXAMPLE_CONNECT


typedef struct
{
    volatile os_int
        count;

    volatile os_int
        start_addr,
        end_addr;
}
ioControllerCallbackData;

typedef struct
{
    volatile os_int
        nro_connections,
        drop_count;

    iocRoot
        *root;

    iocHandle
        exp,
        imp;

    os_int
        countdown,
        spinner,
        count,
        slow;

    os_short
        my_count_from_ioboard;

    os_char
        my_count_status_bits_from_ioboard;

    ioControllerCallbackData
        callbackdata;
}
ioControllerContext;


static iocRoot root;
static ioControllerContext ctx;

/* Forward referred static functions.
 */
static void iocontroller_callback(
    struct iocHandle *mblk_handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void iocontroller_spin_7_segment_delay(
    ioControllerContext *c);

static void iocontroller_long_processing(
    ioControllerContext *c);

static void iocontroller_7_segment(
    iocHandle *mblk_handle,
    os_int x);

static void iocontroller_8_spinner(
    iocHandle *mblk_handle,
    os_int x);

static void iocontroller_print_changes(
    ioControllerContext *c);


/**
****************************************************************************************************

  @brief IO controller example.

  The osal_main() function connects two memory blocks, exp and imp, to IO board.

  @return  None.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    iocMemoryBlockParams blockprm;
    const osalStreamInterface *iface;

#if MY_ROLE==EXAMPLE_CONNECT
    iocConnection *con = 0;
    iocConnectionParams conprm;
    os_char *c_parameters;
#else
    iocEndPoint *ep = 0;
    iocEndPointParams epprm;
    os_char *l_parameters;
#endif

    osalNetworkInterface nic;

    const os_int
        input_block_sz = 1000,
        output_block_sz = 1000;

    os_int
        flags;

    /* Setup network interface configuration for micro-controller environment. This is ignored
       if network interfaces are managed by operating system (Linux/Windows,etc), or if we are
       connecting trough wired Ethernet. If only one subnet, set wifi_net_name_1.
     */
    os_memclear(&nic, sizeof(osalNetworkInterface));
    os_strncpy(nic.wifi_net_name_1, "julian", OSAL_WIFI_PRM_SZ);
    os_strncpy(nic.wifi_net_password_1, "mysecret", OSAL_WIFI_PRM_SZ);
    os_strncpy(nic.wifi_net_name_2, "bean24", OSAL_WIFI_PRM_SZ);
    os_strncpy(nic.wifi_net_password_2 ,"mysecret", OSAL_WIFI_PRM_SZ);

    /* Initialize the underlying transport library. Never call both osal_socket_initialize()
       and osal_tls_initialize(). These use the same underlying library.
       Set up iface to point correct transport interface and set parameters to configure it.
       Set also flags for communication protocol.
     */
#if MY_TRANSPORT==EXAMPLE_USE_TCP_SOCKET
    osal_socket_initialize(&nic, 1);
    iface = OSAL_SOCKET_IFACE;
#if MY_ROLE==EXAMPLE_CONNECT
    c_parameters = EXAMPLE_IP_ADDRESS ":" EXAMPLE_TCP_SOCKET_PORT;
#else
    l_parameters = ":" EXAMPLE_TCP_SOCKET_PORT;
#endif
    flags = IOC_SOCKET|IOC_CREATE_THREAD;
#endif

#if MY_TRANSPORT==EXAMPLE_USE_TLS_SOCKET
    static osalSecurityConfig prm = {EXAMPLE_TLS_SERVER_CERT, EXAMPLE_TLS_SERVER_KEY};
    osal_tls_initialize(&nic, 1, &prm);
    iface = OSAL_TLS_IFACE;
#if MY_ROLE==EXAMPLE_CONNECT
    c_parameters = EXAMPLE_IP_ADDRESS ":" EXAMPLE_TLS_SOCKET_PORT;
#else
    l_parameters = ":" EXAMPLE_TLS_SOCKET_PORT;
#endif
    flags = IOC_SOCKET|IOC_CREATE_THREAD;
#endif

#if MY_TRANSPORT==EXAMPLE_USE_SERIAL_PORT
    osal_serial_initialize();
    iface = OSAL_SERIAL_IFACE;
#if MY_ROLE==EXAMPLE_CONNECT
    c_parameters = EXAMPLE_SERIAL_PORT;
#else
    l_parameters = EXAMPLE_SERIAL_PORT;
#endif
    flags = IOC_SERIAL | IOC_CREATE_THREAD;
#endif

    ioc_initialize_root(&root, IOC_CREATE_OWN_MUTEX);
    os_memclear(&ctx, sizeof(ctx));
    ctx.root = &root;

    ctx.countdown = 10;
    ctx.spinner = -1;
    ctx.count = 0;
    ctx.slow = 1;

    os_memclear(&blockprm, sizeof(blockprm));

    blockprm.nbytes = input_block_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_ALLOW_RESIZE;
    ioc_initialize_memory_block(&ctx.exp, OS_NULL, &root, &blockprm);

    blockprm.nbytes = output_block_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_ALLOW_RESIZE;
    ioc_initialize_memory_block(&ctx.imp, OS_NULL, &root, &blockprm);

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ctx.exp, iocontroller_callback, &ctx);

#if MY_ROLE==EXAMPLE_CONNECT
    /* Connect to an "IO board".
     */
    con  = ioc_initialize_connection(OS_NULL, &root);
    os_memclear(&conprm, sizeof(conprm));
    conprm.parameters = c_parameters;
    conprm.flags = flags;
    conprm.iface = iface;
    ioc_connect(con, &conprm);
#else
    ep = ioc_initialize_end_point(OS_NULL, &root);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = iface;
    epprm.flags = flags;
    epprm.parameters = l_parameters;
    ioc_listen(ep, &epprm);
#endif

   /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(&ctx);
    return OSAL_SUCCESS;
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
    os_short x;
    os_char state_bits;
    ioControllerContext *c;
    c = (ioControllerContext*)app_context;

    /* Do processing which must be done by this thread.
     */
    // iocontroller_long_processing(c);

    // iocontroller_spin_7_segment_delay(c);

    iocontroller_print_changes(c);

    x = ioc_get_short(&ctx.exp, 20, &state_bits);
    if (state_bits & OSAL_STATE_CONNECTED)
    {
        osal_trace_int("v = ", x);
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
    /* End IO board communication, clean up and finsh with the socket and serial port libraries.
     */
    ioc_release_root(&root);
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
    struct iocHandle *mblk_handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    static os_int
        count,
        command_echo;

    ioControllerContext
        *c;

    c = (ioControllerContext*)context;

    /* Get connection status changes.
     */
    if (end_addr >= IOC_NRO_CONNECTED_STREAMS && start_addr < IOC_NRO_CONNECTED_STREAMS + 2)
    {
        c->nro_connections = ioc_getp_short(mblk_handle, IOC_NRO_CONNECTED_STREAMS);
    }

    if (end_addr >= IOC_CONNECTION_DROP_COUNT && start_addr < IOC_CONNECTION_DROP_COUNT + 4)
    {
        c->drop_count = ioc_getp_int(mblk_handle, IOC_CONNECTION_DROP_COUNT);
    }

    /* Echo 2 bytes at address 2 back to IO board address 10. This happens practically
       immediately.
     */
    if (end_addr >= 2 && start_addr < 2 + 2)
    {
        command_echo = ioc_getp_short(mblk_handle, 2);
        ioc_setp_short(&c->imp, 11, command_echo);
    }

    /* Set up for longer processing by specific thread.
     */
    c->callbackdata.count = ++count;
    c->callbackdata.start_addr = start_addr;
    c->callbackdata.end_addr = end_addr;
}


static void iocontroller_spin_7_segment_delay(
    ioControllerContext *c)
{
    if (c->countdown > 0)
    {
        iocontroller_7_segment(&c->imp, --(c->countdown));
        osal_sleep(500);
    }
    else
    {
        if (++(c->spinner) > 7) c->spinner = 0;
        iocontroller_8_spinner(&c->imp, c->spinner);
        osal_sleep(c->slow ? 30 : 1);
        if (c->count++ > (c->slow ? 100 : 2800))
        {
            c->count = 0;
            c->slow = !c->slow;
            if (c->slow) c->countdown = 10;
        }
    }
}

/**
****************************************************************************************************

  @brief Handle stuff detected in callback.

  The iocontroller_long_processing() function handles changed detected at callback function.
  The code which may take time, or needs to be executed by a specific thread is placed here
  instead of callback. The callback just sets "count" in context structure which is then
  used by this function to detect if there is something to do.

  @param   c Pointer to context structure.
  @return  None.

****************************************************************************************************
*/
static void iocontroller_long_processing(
    ioControllerContext *c)
{
    ioControllerCallbackData
        cd;

    os_int
        i;

    char
        text[80],
        nbuf[32];

    os_char
        u;

    /* Get callback data, copy it to local stack
     */
    ioc_lock(c->root);
    cd = c->callbackdata;
    c->callbackdata.count = 0;
    ioc_unlock(c->root);

    if (cd.count)
    {
        os_strncpy(text, "processing callback ", sizeof(text));
        osal_int_to_str(nbuf, sizeof(nbuf), cd.count);
        os_strncat(text, nbuf, sizeof(text));
        os_strncat(text, ": ", sizeof(text));
        osal_int_to_str(nbuf, sizeof(nbuf), cd.start_addr);
        os_strncat(text, nbuf, sizeof(text));
        os_strncat(text, " - ", sizeof(text));
        osal_int_to_str(nbuf, sizeof(nbuf), cd.end_addr);
        os_strncat(text, nbuf, sizeof(text));
        os_strncat(text, ": ", sizeof(text));

        for (i = cd.start_addr; i <= cd.end_addr; i++)
        {
            if (i > cd.start_addr) os_strncat(text, ", ", sizeof(text));
            ioc_read(&c->exp, i, &u, 1);
            osal_int_to_str(nbuf, sizeof(nbuf), (os_long)((os_uint)u));
            os_strncat(text, nbuf, sizeof(text));
        }

        osal_trace(text);
    }
}

static void iocontroller_7_segment(
    iocHandle *mblk_handle,
    os_int x)
{
    static os_char digits[10][8] = {
    /* A, B, C, P, D, E, F, G */
      {1, 1, 1, 0, 1, 1, 1, 0}, /* 0 */
      {0, 1, 1, 0, 0, 0, 0, 0}, /* 1 */
      {1, 1, 0, 0, 1, 1, 0, 1}, /* 2 */
      {1, 1, 1, 0, 1, 0, 0, 1}, /* 3 */
      {0, 1, 1, 0, 0, 0, 1, 1}, /* 4 */
      {1, 0, 1, 0, 1, 0, 1, 1}, /* 5 */
      {0, 0, 1, 0, 1, 1, 1, 1}, /* 6 */
      {1, 1, 1, 0, 0, 0, 0, 0}, /* 7 */
      {1, 1, 1, 0, 1, 1, 1, 1}, /* 8 */
      {1, 1, 1, 0, 0, 0, 1, 1}  /* 9 */
    };

    ioc_set_boolean_array(mblk_handle, 0, digits[x], 8);
}

static void iocontroller_8_spinner(
    iocHandle *mblk_handle,
    os_int x)
{
    static os_char digits[10][8] = {
    /* A, B, C, P, D, E, F, G */
      {1, 0, 0, 0, 0, 0, 0, 0}, /* 0 */
      {0, 0, 0, 0, 0, 0, 1, 0}, /* 1 */
      {0, 0, 0, 0, 0, 0, 0, 1}, /* 2 */
      {0, 0, 1, 0, 0, 0, 0, 0}, /* 3 */
      {0, 0, 0, 0, 1, 0, 0, 0}, /* 4 */
      {0, 0, 0, 0, 0, 1, 0, 0}, /* 5 */
      {0, 0, 0, 0, 0, 0, 0, 1}, /* 6 */
      {0, 1, 0, 0, 0, 0, 0, 0}, /* 7 */
    };

    ioc_set_boolean_array(mblk_handle, 0, digits[x], 8);
}


static void iocontroller_print_changes(
    ioControllerContext *c)
{
    os_short my_count_from_ioboard;
    os_char my_count_status_bits_from_ioboard, nbuf[OSAL_NBUF_SZ];

    /* Read count from IO board.
     */
    my_count_from_ioboard = ioc_get_short(&c->exp, 20,
        &my_count_status_bits_from_ioboard);

    /* If count or state bits have changed, then print it.
     */
    if (my_count_from_ioboard != c->my_count_from_ioboard ||
        my_count_status_bits_from_ioboard != c->my_count_status_bits_from_ioboard)
    {
        c->my_count_from_ioboard = my_count_from_ioboard;
        c->my_count_status_bits_from_ioboard = my_count_status_bits_from_ioboard;

        osal_int_to_str(nbuf, sizeof(nbuf), my_count_from_ioboard);
        osal_console_write("signal[20] = ");
        osal_console_write(nbuf);

        osal_console_write(my_count_status_bits_from_ioboard & OSAL_STATE_CONNECTED
             ? " CONNECTED" : " DISCONNECTED");

        switch (my_count_status_bits_from_ioboard & OSAL_STATE_ERROR_MASK)
        {
            case OSAL_STATE_YELLOW:
                osal_console_write(" YELLOW");
                break;

            case OSAL_STATE_ORANGE:
                osal_console_write(" ORANGE");
                break;

            case OSAL_STATE_RED:
                osal_console_write(" RED");
                break;

            default:
                break;
        }
        osal_console_write("\n");
    }
}
