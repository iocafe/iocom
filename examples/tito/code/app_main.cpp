/**

  @file    app_main.c
  @brief   Tito controller using static IO device configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "app_main.h"

/* The devicedir is here for testing only, take away.
 */
#include "devicedir.h"

iocRoot app_iocom;
static AppRoot *tito_main;


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
    osalSecurityConfig security_prm;

    os_memclear(&security_prm, sizeof(security_prm));
    security_prm.server_cert_file = "alice.crt";
    security_prm.server_key_file = "alice.key";

    /* Initialize communication root object.
     */
    ioc_initialize_root(&app_iocom);
    ioc_set_iodevice_id(&app_iocom, "tito", IOC_AUTO_DEVICE_NR, "pass", "iocafenet");

    /* Initialize the transport, socket, TLS, serial, etc..
     */
    osal_tls_initialize(OS_NULL, 0, &security_prm);
    osal_serial_initialize();

    /* Create tito main object and start listening for clients.
     */
    tito_main = new AppRoot;
    tito_main->listen_for_clients();

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
    /* The devicedir call is here for testing only, take away.
     */
    io_device_console(&app_iocom);

    return tito_main->loop();
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
    delete tito_main;

    ioc_release_root(&app_iocom);
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
  @param   flags IOC_MBLK_CALLBACK_WRITE indicates change by local write,
           IOC_MBLK_CALLBACK_RECEIVE change by data received.
  @param   context Callback context, not used by "tito" example.
  @return  None.

****************************************************************************************************
*/
#if 0
void ioboard_communication_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    /* '#ifdef' is used to compile code in only if 7-segment display is configured
       for the hardware.
     */
#ifdef PINS_SEGMENT7_GROUP
    os_char buf[TITO_DOWN_SEVEN_SEGMENT_ARRAY_SZ];
    const Pin *pin;
    os_short i;

    /* Process 7 segment display. Since this is transferred as boolean array, the
       forward_signal_change_to_io_pins() doesn't know to handle this. Thus, read
       boolean array from communication signal, and write it to IO pins.
     */
    if (ioc_is_my_address(&tito.down.seven_segment, start_addr, end_addr))
    {
        ioc_gets_array(&tito.down.seven_segment, buf, TITO_DOWN_SEVEN_SEGMENT_ARRAY_SZ);
        if (ioc_is_value_connected(tito.down.seven_segment))
        {
            osal_console_write("7 segment data received\n");
            for (i = TITO_DOWN_SEVEN_SEGMENT_ARRAY_SZ - 1, pin = pins_segment7_group;
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
#endif
