/**

  @file    iocontroller.c
  @brief   IO controller example 4_iocontroller_test.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    7.8.2018

  The 4_iocontroller_test example controls 7 segment LED display. It connects to the IO board
  through TCP socket or serial port.

  This example assumes one memory block for inputs and the other for outputs. It uses both
  supports dynamic memory allocation and multithreading, thus this example cannot be used in
  most microcontrollers.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
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
#define MY_TRANSPORT EXAMPLE_USE_TLS_SOCKET

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
#define EXAMPLE_IP_ADDRESS "192.168.1.220"
#define EXAMPLE_TCP_SOCKET_PORT IOC_DEFAULT_SOCKET_PORT_STR
#define EXAMPLE_TLS_SOCKET_PORT IOC_DEFAULT_TLS_PORT_STR
#define EXAMPLE_TLS_SERVER_CERT "/coderoot/eosal/extensions/tls/ssl-test-keys-and-certs/alice.crt"
#define EXAMPLE_TLS_SERVER_KEY "/coderoot/eosal/extensions/tls/ssl-test-keys-and-certs/alice.key"
#define EXAMPLE_SERIAL_PORT "COM3,baud=115200"

/* List of connection roles. Either listen for or connect socket.
 */
#define EXAMPLE_LISTEN 0
#define EXAMPLE_CONNECT 1

/* Select connect role here
 */
#define MY_ROLE EXAMPLE_LISTEN


typedef struct
{
    volatile int
        count;

    volatile int
        start_addr,
        end_addr;
}
ioControllerCallbackData;

typedef struct
{
    volatile int
        nro_connections,
        drop_count;

    iocRoot
        *root;

    iocMemoryBlock
        *inputs,
        *outputs;

    ioControllerCallbackData
        callbackdata;
}
ioControllerContext;



/* Forward referred static functions.
 */
static void iocontroller_callback(
    struct iocMemoryBlock *mblk,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context);

static void iocontroller_long_processing(
    ioControllerContext *c);

static void iocontroller_7_segment(
    struct iocMemoryBlock *mblk,
    int x);

static void iocontroller_8_spinner(
    struct iocMemoryBlock *mblk,
    int x);


/**
****************************************************************************************************

  @brief IO controller example.

  The osal_main() function connects two memory blocks, inputs and outputs, to IO board.

  @return  None.

****************************************************************************************************
*/
os_int osal_main(
    os_int argc,
    os_char *argv[])
{
    iocRoot root;
    ioControllerContext c;
    iocMemoryBlockParams blockprm;
    const osalStreamInterface *iface;
    os_char *c_parameters, *l_parameters;

#if MY_ROLE==EXAMPLE_CONNECT
    iocConnection *con = 0;
    iocConnectionParams conprm;
#else
    iocEndPoint *ep = 0;
    iocEndPointParams epprm;
#endif

    const os_int
        input_block_sz = 1000,
        output_block_sz = 1000;

    os_int
        countdown = 10,
        spinner = -1,
        count = 0,
        slow = 1,
        flags;

    /* Initialize the underlying transport library. Never call boath osal_socket_initialize()
       and osal_tls_initialize(). These use the same underlying library.
       Set up iface to point correct transport interface and set parameters to configure it.
       Set also flags for communication protocol.
     */
#if MY_TRANSPORT==EXAMPLE_USE_TCP_SOCKET
    osal_socket_initialize(OS_NULL, 0);
    iface = OSAL_SOCKET_IFACE;
    c_parameters = EXAMPLE_IP_ADDRESS ":" EXAMPLE_TCP_SOCKET_PORT;
    l_parameters = ":" EXAMPLE_TCP_SOCKET_PORT;
    flags = IOC_SOCKET|IOC_CREATE_THREAD;
#endif

#if MY_TRANSPORT==EXAMPLE_USE_TLS_SOCKET
    static osalTLSParam prm = {EXAMPLE_TLS_SERVER_CERT, EXAMPLE_TLS_SERVER_KEY};
    osal_tls_initialize(OS_NULL, 0, &prm);
    iface = OSAL_TLS_IFACE;
    c_parameters = EXAMPLE_IP_ADDRESS ":" EXAMPLE_TLS_SOCKET_PORT;
    l_parameters = ":" EXAMPLE_TLS_SOCKET_PORT;
    flags = IOC_SOCKET|IOC_CREATE_THREAD;
#endif

#if MY_TRANSPORT==EXAMPLE_USE_SERIAL_PORT
    osal_serial_initialize();
    iface = OSAL_SERIAL_IFACE;
    c_parameters = EXAMPLE_SERIAL_PORT;
    l_parameters = EXAMPLE_SERIAL_PORT;
    flags = IOC_SERIAL | IOC_CREATE_THREAD;
#endif

    ioc_initialize_root(&root);
    os_memclear(&c, sizeof(c));
    c.root = &root;

    os_memclear(&blockprm, sizeof(blockprm));

    blockprm.mblk_nr = IOC_INPUT_MBLK;
    blockprm.nbytes = input_block_sz;
    blockprm.flags = IOC_TARGET|IOC_AUTO_RECEIVE|IOC_ALLOW_RESIZE;
    c.inputs = ioc_initialize_memory_block(OS_NULL, &root, &blockprm);

    blockprm.mblk_nr = IOC_OUTPUT_MBLK;
    blockprm.nbytes = output_block_sz;
    blockprm.flags = IOC_SOURCE|IOC_AUTO_SEND|IOC_ALLOW_RESIZE;
    c.outputs = ioc_initialize_memory_block(OS_NULL, &root, &blockprm);

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(c.inputs, iocontroller_callback, &c);

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

    while (OS_TRUE)
    {
        if (countdown > 0)
        {
            iocontroller_7_segment(c.outputs, --countdown);
            os_sleep(500);
        }
        else
        {
            if (++spinner > 7) spinner = 0;
            iocontroller_8_spinner(c.outputs, spinner);
            os_sleep(slow ? 30 : 1);
            if (count++ > (slow ? 100 : 2800))
            {
                count = 0;
                slow = !slow;
                if (slow) countdown = 10;
            }
        }

        /* Do processing which must be done by this thread.
         */
        iocontroller_long_processing(&c);
    }

    /* End IO board communication, clean up and finsh with the socket and serial port libraries.
     */
    ioc_release_root(&root);

#if MY_TRANSPORT==EXAMPLE_USE_TCP_SOCKET
    osal_socket_shutdown();
#endif
#if MY_TRANSPORT==EXAMPLE_USE_TLS_SOCKET
    osal_tls_shutdown();
#endif
#if MY_TRANSPORT==EXAMPLE_USE_SERIAL_PORT
    osal_serial_shutdown();
#endif
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
    static int
        count,
        command_echo;

    ioControllerContext
        *c;

    c = (ioControllerContext*)context;

    /* Get connection status changes.
     */
    if (end_addr >= IOC_NRO_CONNECTED_STREAMS && start_addr < IOC_NRO_CONNECTED_STREAMS + 2)
    {
        c->nro_connections = ioc_get16(mblk, IOC_NRO_CONNECTED_STREAMS);
    }

    if (end_addr >= IOC_CONNECTION_DROP_COUNT && start_addr < IOC_CONNECTION_DROP_COUNT + 4)
    {
        c->drop_count = ioc_get32(mblk, IOC_CONNECTION_DROP_COUNT);
    }

    /* Echo 2 bytes at address 2 back to IO board address 10. This happens practically
       immediately.
     */
    if (end_addr >= 2 && start_addr < 2 + 2)
    {
        command_echo = ioc_get16(mblk, 2);
        ioc_set16(c->outputs, 11, command_echo);
    }

    /* Set up for longer processing by specific thread.
     */
    c->callbackdata.count = ++count;
    c->callbackdata.start_addr = start_addr;
    c->callbackdata.end_addr = end_addr;
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

    int
        i;

    char
        text[80],
        nbuf[32];

    os_uchar
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
        osal_int_to_string(nbuf, sizeof(nbuf), cd.count);
        os_strncat(text, nbuf, sizeof(text));
        os_strncat(text, ": ", sizeof(text));
        osal_int_to_string(nbuf, sizeof(nbuf), cd.start_addr);
        os_strncat(text, nbuf, sizeof(text));
        os_strncat(text, " - ", sizeof(text));
        osal_int_to_string(nbuf, sizeof(nbuf), cd.end_addr);
        os_strncat(text, nbuf, sizeof(text));
        os_strncat(text, ": ", sizeof(text));

        for (i = cd.start_addr; i <= cd.end_addr; i++)
        {
            if (i > cd.start_addr) os_strncat(text, ", ", sizeof(text));
            ioc_read(c->inputs, i, &u, 1);
            osal_int_to_string(nbuf, sizeof(nbuf), (os_long)((os_uint)u));
            os_strncat(text, nbuf, sizeof(text));
        }

        osal_trace(text);
    }
}

static void iocontroller_7_segment(
    struct iocMemoryBlock *mblk,
    int x)
{
    static os_uchar digits[10][8] = {
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

    ioc_write(mblk, 0, digits[x], 8);
}

static void iocontroller_8_spinner(
    struct iocMemoryBlock *mblk,
    int x)
{
    static os_uchar digits[10][8] = {
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

    ioc_write(mblk, 0, digits[x], 8);
}
