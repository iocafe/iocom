/**

  @file    uno.c
  @brief   Arduino Uno IO board as IOCOM device.
  @author  Pekka Lehtikoski, Markku Nissinen
  @version 1.0
  @date    22.1.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Select serial communication before including uno.h.
 */
#define IOBOARD_CTRL_CON IOBOARD_CTRL_CONNECT_SERIAL
#include "uno.h"

/* Do we want this test code to control IO though PINS API? 
   PINS API is portability wrapper, which here maps to Arduino IO functions. PINS API is
   optional, the regular Arduino IO functions can be used as well. Include pins/unoboard
   in platformio.ini to build with pins library, or pins/generic to build without it.
   That effects IOC_PINS_IO_INCLUDED define.
 */
#ifdef IOC_PINS_IO_INCLUDED
    #define IOBOARD_USE_PINS_IO 1
#else
    #define IOBOARD_USE_PINS_IO 0
#endif

/* Maximum number of sockets, etc.
 */
#define IOBOARD_MAX_CONNECTIONS 1

/* Use static memory pool. 
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

  @brief Initialize communication and other stuff.
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

    /* We use quiet mode. Since Arduino UNO has only one serial port, we need it for
       communication. We cannot have any trace, etc. prints to serial port. 
     */
#if OSAL_MINIMALISTIC
    osal_quiet(OS_TRUE);
#endif

#if IOBOARD_USE_PINS_IO
    /* Setup IO pins.
     */
     pins_setup(&pins_hdr, PINS_DEFAULT);
#endif     

    osal_serial_initialize();

    /* Set up parameters for the IO board.
     */
    os_memclear(&prm, sizeof(prm));
    prm.iface = IOBOARD_IFACE;
    prm.device_name = IOBOARD_DEVICE_NAME; /* or device_id->device name to allow change */
    prm.device_nr = 1;
    prm.network_name = "cafenet";
    prm.ctrl_type = IOBOARD_CTRL_CON;
    prm.serial_con_str = "ttyS30";
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

    /* Set callback to detect received data and connection status changes.
     */
    ioc_add_callback(&ioboard_imp, ioboard_callback, OS_NULL);

#if IOBOARD_USE_PINS_IO
    /* Connect PINS library to IOCOM library
     */
    pins_connect_iocom_library(&pins_hdr);
#endif

    /* When emulating micro-controller on PC, run loop. Just save context pointer on
       real micro-controller.
     */
    osal_simulated_loop(OS_NULL);

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Loop function, called repeatedly.

  The osal_loop() function maintains communication, reads IO pins and runs the IO device 
  functionality.

  @param   app_context Void pointer, to pass application context structure, etc.
  @return  The function returns OSAL_SUCCESS to continue running. Other return values are
           to be interprened as reboot on micro-controller or quit the program on PC computer.

****************************************************************************************************
*/
osalStatus osal_loop(
    void *app_context)
{
    os_timer ti;
    static os_timer start_t = 0;
    static os_char state = 0;
    os_int timeout_ms;
    OSAL_UNUSED(app_context);

    /* Keep the communication alive. If data is received from communication, the
       ioboard_callback() will be called. Move data data synchronously
       to incomong memory block.
     */
    os_get_timer(&ti);
    ioc_run(&ioboard_root);
    ioc_receive(&ioboard_imp);

#if IOBOARD_USE_PINS_IO
    /* Read all input pins from hardware into global pins structures. Reading will forward
       input states to communication.
     */
    pins_read_all(&pins_hdr, PINS_DEFAULT);
#endif

    /* Get inputs we are using.
     */
    int LeftTurn = ioc_get(&uno.imp.LeftTurn);
    int RightTurn = ioc_get(&uno.imp.RightTurn);
    int StraightForward = ioc_get(&uno.imp.StraightForward);
    int ForwardBackward = ioc_get(&uno.imp.ForwardBackward);

    /* Modify state.
     */
    timeout_ms = 1000;
    if (LeftTurn) timeout_ms = 200;
    if (RightTurn) timeout_ms = 80;
    if (StraightForward) timeout_ms = 30;
    if (ForwardBackward) timeout_ms = 5;

    if (os_has_elapsed_since(&start_t, &ti, timeout_ms)) {
        if (++state > 3) state = 0;
        start_t = ti;
    }

    /* Set outputs.
     */
    ioc_set(&uno.exp.LEFT, state == 0);
    ioc_set(&uno.exp.RIGHT, state == 1);
    ioc_set(&uno.exp.FORWARD, state == 2);
    ioc_set(&uno.exp.BACKWARD, state == 3);

    /* Send changed data to iocom.
     */
    ioc_send(&ioboard_exp);
    ioc_run(&ioboard_root);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Finished with the application, clean up.

  The osal_main_cleanup() function ends IO board communication, cleans up and finshes with the
  socket and serial port libraries. On real IO device we may not need to take care about this, 
  since these are often shut down only by turning or power or by microcontroller reset.

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

#if IOBOARD_USE_PINS_IO
    if (flags & IOC_MBLK_CALLBACK_RECEIVE)
    {
        /* Call pins library extension to forward communication signal changes to IO pins.
         */
        forward_signal_change_to_io_pins(handle, start_addr, end_addr, &uno_hdr, flags);
    }
#else
    OSAL_UNUSED(handle);
    OSAL_UNUSED(start_addr);
    OSAL_UNUSED(end_addr);
    OSAL_UNUSED(flags);
#endif    
}
