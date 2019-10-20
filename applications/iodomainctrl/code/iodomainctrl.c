/**

  @file    iocom/applications/iodomainctrl/iodomainctrl.c
  @brief   Socket server example.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.9.2019

  Basic IO domain controller application.

  Copyright 2012 - 2019 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Force tracing on for this source file.
 */
#undef OSAL_TRACE
#define OSAL_TRACE 3

#include "iodomainctrl.h"

/* Which communications we do listen to?
 */
os_boolean iodomain_listen_tls = OS_TRUE;
os_boolean iodomain_listen_plain_tcp = OS_FALSE;
os_boolean iodomain_listen_serial = OS_FALSE;

/* Communication ports.
   iodomain_tls_port: Secured TCP socket port number to listen for TLS connections.
   iodomain_tcp_port: Unsecured TCP socket port number to listen.
   iodomain_serial_prm: Serial port can be selected using Windows
   style using "COM1", "COM2"... These are mapped to hardware/operating system in device specific
   manner. On Linux port names like "ttyS30,baud=115200" or "ttyUSB0" can be also used.
 */
os_char iodomain_tls_port[IODOMAIN_PORT_SZ] = ":" IOC_DEFAULT_TLS_PORT_STR;
os_char iodomain_tcp_port[IODOMAIN_PORT_SZ] = ":" IOC_DEFAULT_SOCKET_PORT_STR;
os_char iodomain_serial_prm[IODOMAIN_SERIAL_PRM_SZ] = "COM3,baud=115200";

/* TLS specific.
   iodomain_server_cert: Path to server certificate file.
   iodomain_server_key: Path to server key file.
 */
os_char iodomain_server_cert[IODOMAIN_PATH_SZ]
    = "/coderoot/eosal/extensions/tls/ssl-test-keys-and-certs/alice.crt";
os_char iodomain_server_key[IODOMAIN_PATH_SZ]
    = "/coderoot/eosal/extensions/tls/ssl-test-keys-and-certs/alice.key";


/* IO domain class structure. Holds state of the IO domain.
 */
static iodomainClass iodomain;

/* Handles for listening connected stream, and for the stream listening for connections.
 */
//static osalStream stream, mystream;


/* Forward referred static functions.
 */
static osalStatus iodomainctrl_parse_command_line(
    os_int argc,
    os_char *argv[]);



/**
****************************************************************************************************

  @brief Process entry point.

  The osal_main() function is OS independent entry point.

  IO domain controller main function.

  @param   argc Number of command line arguments.
  @param   argv Array of string pointers, one for each command line argument. UTF8 encoded.

  @return  None.

****************************************************************************************************
*/
os_int osal_main(
    os_int argc,
    os_char *argv[])
{
    osalTLSParam tlsprm;
    iodomainParams prm;

    /* Parse command line arguments.
     */
    if (iodomainctrl_parse_command_line(argc, argv) != OSAL_SUCCESS)
    {
        return 1;
    }

    /* Initialize the underlying transport libraries.
     */
    if (iodomain_listen_tls)
    {
        os_memclear(&tlsprm, sizeof(tlsprm));
        tlsprm.certfile = iodomain_server_cert;
        tlsprm.keyfile = iodomain_server_key;
        osal_tls_initialize(&tlsprm);
    }
    else if (iodomain_listen_plain_tcp)
    {
        osal_socket_initialize();
    }

    if (iodomain_listen_serial)
    {
        osal_serial_initialize();
    }

    /* All microcontroller do not clear memory at soft reboot.
     */
    os_memclear(&iodomain, sizeof(iodomain));
    os_memclear(&prm, sizeof(prm));
//    stream = OS_NULL;

    /* Set up the domain controller.
     */
    iodomain_start(&iodomain, &prm);


    /* When emulating micro-controller on PC, run loop. Does nothing on real micro-controller.
     */
    osal_simulated_loop(OS_NULL);

    return 0;
}


/**
****************************************************************************************************

  @brief Loop function to be called repeatedly.

  The osal_loop() function:
  - Accepts incoming TCP/TLS socket connection. 
  - If we have a connection:
  -- Reads data received from socket and prints it to console. 
  -- Check for user key pressess and writes those to socket.

  Note: See note for serial communication, same applies here.

  @param   app_context Void pointer, reserved to pass context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    /* Some socket library implementations need this, for DHCP, etc.
     */
    osal_socket_maintain();

    os_sleep(500);

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Finish with communication.

  The osal_main_cleanup() function closes listening socket port and connected socket port, then
  closes underlying stream library. Notice that the osal_stream_close() function does close does 
  nothing if it is called with NULL argument.

  @param   app_context Void pointer, reserved to pass context structure, etc.
  @return  None.

****************************************************************************************************
*/
void osal_main_cleanup(
    void *app_context)
{
    /* We are finished with the IO domain.
     */
    iodomain_stop(&iodomain);

    /* Shut down underlying transports.
     */
    if (iodomain_listen_tls)
    {
        osal_tls_shutdown();
    }
    else if (iodomain_listen_plain_tcp)
    {
        osal_socket_shutdown();
    }

    if (iodomain_listen_serial)
    {
        osal_serial_shutdown();
    }
}


/**
****************************************************************************************************

  @brief Parse command line.

  The iodomainctrl_parse_command_line() function parses command line arguments into global
  variables.

  The function initializes used stream library and either opens a serial port or creates
  listening TCP/TLS socket.

  @param   argc Number of command line arguments.
  @param   argv Array of string pointers, one for each command line argument. UTF8 encoded.

  @return  OSAL_SUCCESS if all is fine. Other values indicate such an error in arguments that
           the application cannot be started.

****************************************************************************************************
*/
static osalStatus iodomainctrl_parse_command_line(
    os_int argc,
    os_char *argv[])
{


    return OSAL_SUCCESS;
}