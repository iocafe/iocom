/**

  @file    uno.c
  @brief   Uno IO board example featuring  IoT device.
  @author  Pekka Lehtikoski, Markku Nissinen
  @version 1.0
  @date    6.4.2020

  IOBOARD_CTRL_CON define selects how this IO device connects to control computer. One of
  IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_CONNECT_TLS or IOBOARD_CTRL_CONNECT_SERIAL.

  Serial port can be selected using Windows style using "COM1", "COM2"... These are mapped
  to hardware/operating system in device specific manner. On Linux port names like
  "ttyS30,baud=115200" or "ttyUSB0" can be also used.

  IOBOARD_MAX_CONNECTIONS sets maximum number of connections. IO board needs one connection.

  Notes:

  - ON MULTITHREADING ENVIRONMENT WITH SELECTS LOOP THREAD CAN WAIT FOR TIMEOUT OR EVENT

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
/* Select socket, TLS or serial communication before including uno.h.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SERIAL
#include "uno.h"

/* Maximum number of sockets, etc.
 */
#define IOBOARD_MAX_CONNECTIONS 1

/* Timer for sending
 */
static os_timer send_timer;

/* Use static memory pool
 */
static os_char
    ioboard_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        UNO_EXP_MBLK_SZ, UNO_IMP_MBLK_SZ)
        + IOBOARD_POOL_DEVICE_INFO(IOBOARD_MAX_CONNECTIONS)];

/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
 */
EOSAL_C_MAIN

/**
****************************************************************************************************

  @brief Set up the communication.

  Sets up network and Initialize transport
  @return  OSAL_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    ioboardParams prm;

    OSAL_UNUSED(argc);
    OSAL_UNUSED(argv);

    osal_quiet(OS_TRUE);

    /* Setup IO pins.
     */
    // pins_setup(&pins_hdr, PINS_DEFAULT);

    osal_serial_initialize();

// osal_sysconsole_write("HEHE 3\n");

    /* Set up parameters for the IO board.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = IOBOARD_IFACE;
    prm.device_name = IOBOARD_DEVICE_NAME; /* or device_id->device name to allow change */
    prm.device_nr = 1;
    prm.network_name = "cafenet";
    prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.serial_con_str = "ttyS30:";
    prm.max_connections = IOBOARD_MAX_CONNECTIONS;
    prm.exp_mblk_sz = UNO_EXP_MBLK_SZ;
    prm.imp_mblk_sz = UNO_IMP_MBLK_SZ;
    prm.pool = ioboard_pool;
    prm.pool_sz = sizeof(ioboard_pool);
    prm.device_info = ioapp_signals_config;
    prm.device_info_sz = sizeof(ioapp_signals_config);

    prm.exp_signal_hdr = &uno.exp.hdr;
    prm.imp_signal_hdr = &uno.imp.hdr;

    /* Start communication.
     */
    ioboard_start_communication(&prm);

// osal_sysconsole_write("HEHE 4\n");

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ioboard_imp, ioboard_callback, OS_NULL);

    /* Connect PINS library to IOCOM library
     */
// THIS GETS LOCKED UP!
//    pins_connect_iocom_library(&pins_hdr);

// osal_sysconsole_write("HEHE 5\n");

    os_get_timer(&send_timer);

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);

// osal_sysconsole_write("HEHE 6\n");

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
    os_timer ti;

    OSAL_UNUSED(app_context);


    os_sleep(300);

    osal_sysconsole_write("HEHE X0\n");

   /* static os_boolean test_toggle; */

    os_get_timer(&ti);


 os_sleep(300);

    /* Keep the communication alive. If data is received from communication, the
       ioboard_callback() will be called. Move data data synchronously
       to incomong memory block.
     */
    ioc_run(&ioboard_root);

// osal_sysconsole_write("HEHE X2\n");

    ioc_receive(&ioboard_imp);

// osal_sysconsole_write("HEHE X\n");

    static int i;
    ioc_set(&uno.exp.LEFT, i++);


#if 0
    /* Read all input pins from hardware into global pins structures. Reading will forward
       input states to communication.
     */
    pins_read_all(&pins_hdr, PINS_DEFAULT);



    int LeftTurn = ioc_get(&uno.imp.LeftTurn);
    int RightTurn = ioc_get(&uno.imp.RightTurn);
    int StraightForward = ioc_get(&uno.imp.StraightForward);
    int ForwardBackward = ioc_get(&uno.imp.ForwardBackward);

    if (StraightForward && ForwardBackward && !LeftTurn && !RightTurn) {
        pin_set(&pins.outputs.LEFT,0);
        pin_set(&pins.outputs.RIGHT,0);
        pin_set(&pins.outputs.FORWARD,1);
        pin_set(&pins.outputs.BACKWARD,0);
    }

    if (StraightForward && !ForwardBackward && !LeftTurn && !RightTurn) {
        pin_set(&pins.outputs.LEFT,0);
        pin_set(&pins.outputs.RIGHT,0);
        pin_set(&pins.outputs.FORWARD,0);
        pin_set(&pins.outputs.BACKWARD,1);
    }

    if (LeftTurn && ForwardBackward && !StraightForward && !RightTurn) {
        pin_set(&pins.outputs.LEFT,1);
        pin_set(&pins.outputs.RIGHT,0);
        pin_set(&pins.outputs.FORWARD,1);
        pin_set(&pins.outputs.BACKWARD,0);
    }

    if (LeftTurn && !ForwardBackward && !StraightForward && !RightTurn) {
        pin_set(&pins.outputs.LEFT,1);
        pin_set(&pins.outputs.RIGHT,0);
        pin_set(&pins.outputs.FORWARD,0);
        pin_set(&pins.outputs.BACKWARD,1);
    }

    if (RightTurn && ForwardBackward && !StraightForward && !LeftTurn) {
        pin_set(&pins.outputs.RIGHT,1);
        pin_set(&pins.outputs.LEFT,0);
        pin_set(&pins.outputs.FORWARD,1);
        pin_set(&pins.outputs.BACKWARD,0);
    }

    if (RightTurn && !ForwardBackward && !StraightForward && !LeftTurn) {
        pin_set(&pins.outputs.RIGHT,1);
        pin_set(&pins.outputs.LEFT,0);
        pin_set(&pins.outputs.FORWARD,0);
        pin_set(&pins.outputs.BACKWARD,1);
    }

    if (!RightTurn && !StraightForward && !LeftTurn) {
        pin_set(&pins.outputs.LEFT,0);
        pin_set(&pins.outputs.RIGHT,0);
        pin_set(&pins.outputs.FORWARD,0);
        pin_set(&pins.outputs.BACKWARD,0);
    }
#endif

    /* Send changed data synchronously from outgoing memory blocks every 50 ms. If we need
       very low latency IO in local network we can have interval like 1 ms, or just call send
       unconditionally.
       If we are not in such hurry, we can save network resources by merging multiple changes.
       to be sent together in TCP package and use value like 100 ms.
       Especially in IoT we may want to minimize number of transferred TCP packets to
       cloud server. In this case it is best to use to two timers and flush ioboard_exp and
       ioboard_conf_exp separately. We could evenu use value like 2000 ms or higher for
       ioboard_exp. For ioboard_conf_exp we need to use relatively short value, like 100 ms
       even then to keep software updates, etc. working. This doesn't generate much
       communication tough, conf_export doesn't change during normal operation.
     */
    // if (os_timer_hit(&send_timer, &ti, 10))
    {
        ioc_send(&ioboard_exp);
        /* ioc_send(&ioboard_conf_exp); */
        ioc_run(&ioboard_root);
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
    OSAL_UNUSED(app_context);

    ioboard_end_communication();
    osal_serial_shutdown();
}


/**
****************************************************************************************************

  @brief Callback function when data has been received from communication.

  The ioboard_callback function reacts to data from communication. Here we treat
  memory block as set of communication signals, and mostly just forward these to IO.

  @param   handle Memory block handle.
  @param   start_addr First changed memory block address.
  @param   end_addr Last changed memory block address.
  @param   flags IOC_MBLK_CALLBACK_WRITE indicates change by local write,
           IOC_MBLK_CALLBACK_RECEIVE change by data received.
  @param   context Callback context, not used by "uno" example.
  @return  None.

****************************************************************************************************
*/
void ioboard_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    OSAL_UNUSED(context);

    if (flags & IOC_MBLK_CALLBACK_RECEIVE)
    {
        /* Call pins library extension to forward communication signal changes to IO pins.
         */
//        forward_signal_change_to_io_pins(handle, start_addr, end_addr, &uno_hdr, flags);
    }
}
