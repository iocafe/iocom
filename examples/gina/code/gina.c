/**

  @file    gina.c
  @brief   Gina IO board example featuring  IoT device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    1.11.2019

  IOBOARD_CTRL_CON define selects how this IO device connects to control computer. One of
  IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_CONNECT_TLS or IOBOARD_CTRL_CONNECT_SERIAL.

  GINA_SERIAL_PORT: Serial port can be selected using Windows style using "COM1",
  "COM2"... These are mapped to hardware/operating system in device specific
  manner. On Linux port names like "ttyS30,baud=115200" or "ttyUSB0" can be also used.

  IOBOARD_MAX_CONNECTIONS sets maximum number of connections. IO board needs one connection.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
/* Select socket, TLS or serial communication before including gina.h.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SOCKET
#include "gina.h"

/* The devicedir is here for testing only, take away.
 */
#include "devicedir.h"

/* Transport parameters.
 */
#define GINA_IP_ADDRESS "192.168.1.220"
#define GINA_SERIAL_PORT "COM3,baud=115200"

/* Maximum number of sockets, etc.
 */
#define IOBOARD_MAX_CONNECTIONS 1

/* Use static memory pool
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        GINA_EXP_MBLK_SZ, GINA_IMP_MBLK_SZ)
        + IOBOARD_POOL_DEVICE_INFO(IOBOARD_MAX_CONNECTIONS)];


/**
****************************************************************************************************

  @brief Set up the communication.

  Initialize transport stream and set interface
  @return  OSAL_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    ioboardParams prm;
    const osalStreamInterface *iface;

    /* Setup IO pins.
     */
    pins_setup(&pins_hdr, 0);

    /* Initialize the transport, socket, TLS, serial, etc..
     */
    osal_tls_initialize(OS_NULL, 0, OS_NULL);
    osal_serial_initialize();

    /* Get stream interface by IOBOARD_CTRL_CON define.
     */
    iface = IOBOARD_IFACE;

    /* Set up parameters for the IO board.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = iface;
    prm.device_name = IOBOARD_DEVICE_NAME;
    prm.device_nr = 1;
    prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.socket_con_str = GINA_IP_ADDRESS;
    prm.serial_con_str = GINA_SERIAL_PORT;
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.send_block_sz = GINA_EXP_MBLK_SZ;
    prm.receive_block_sz = GINA_IMP_MBLK_SZ;
    prm.auto_synchronization = OS_FALSE;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);
    prm.device_signal_hdr = &gina_hdr;

    prm.device_info = gina_config;
    prm.device_info_sz = sizeof(gina_config);

    /* Start communication.
     */
    ioboard_start_communication(&prm);

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ioboard_import, ioboard_communication_callback, OS_NULL);

    /* Connect PINS library to IOCOM library
     */
    pins_connect_iocom_library(&pins_hdr);

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Loop function to be called repeatedly.

  The osal_loop() function maintains communication, reads IO pins (reading forwards input states
  to communication) and runs the IO device functionality.

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    /* Keep the communication alive. If data is received from communication, the
       ioboard_communication_callback() will be called.
     */
    ioc_run(&ioboard_communication);

    /* Move data data synchronously to incomong memory block.
     */
    ioc_receive(&ioboard_import);

    /* Read all input pins from hardware into global pins structures. Reading will forward
       input states to communication.
     */
    pins_read_all(&pins_hdr, PINS_DEFAULT);

    /* Run the IO device functionality.
     */

    /* The devicedir call is here for testing only, take away.
     */
    io_device_console(&ioboard_communication);


    /* Move data synchronously from outgoing memory block.
     */
    ioc_send(&ioboard_export);

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

  @brief Callback function when data has been received from communication.

  The ioboard_communication_callback function reacts to data from communication. Here we treat
  memory block as set of communication signals, and mostly just forward these to IO.

  @param   handle Memory block handle.
  @param   start_addr First changed memory block address.
  @param   end_addr Last changed memory block address.
  @param   flags Last changed memory block address.
  @param   context Callback context, not used by "gina" example.
  @return  None.

****************************************************************************************************
*/
void ioboard_communication_callback(
    struct iocHandle *handle,
    int start_addr,
    int end_addr,
    os_ushort flags,
    void *context)
{
    os_char sb;

    /* '#ifdef' is used to compile code in only if 7-segment display is configured
       for the hardware.
     */
#ifdef PINS_SEGMENT7_GROUP
    os_char buf[GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ];
    const Pin *pin;
    os_short i;

    /* Process 7 segment display. Since this is transferred as boolean array, the
       forward_signal_change_to_io_pins() doesn't know to handle this. Thus, read
       boolean array from communication signal, and write it to IO pins.
     */
    if (ioc_is_my_address(&gina.imp.seven_segment, start_addr, end_addr))
    {
        sb = ioc_gets_array(&gina.imp.seven_segment, buf, GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ);
        if (sb & OSAL_STATE_CONNECTED)
        {
            osal_console_write("7 segment data received\n");
            for (i = GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ - 1, pin = pins_segment7_group;
                 i >= 0 && pin;
                 i--, pin = pin->next) /* For now we need to loop backwards, fix this */
            {
                pin_set(pin, buf[i]);
            }
        }
        else
        {
            // WE DO NOT COME HERE. SHOULD WE INVALIDATE WHOLE MAP ON DISCONNECT?
            osal_console_write("7 segment data DISCONNECTED\n");
        }
    }
#endif

    /* Call pins library extension to forward communication signal changed to IO pins.
     */
    forward_signal_change_to_io_pins(handle, start_addr, end_addr, flags);
}
