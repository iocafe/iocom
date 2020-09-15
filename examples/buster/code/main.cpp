/**

  @file    main.cpp
  @brief   Program entry point, Buster IO device set up.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Code here is general program setup code. It initializes iocom library to be used as automation
  device controller. This example code uses eosal functions everywhere, including the program
  entry point osal_main(). If you use iocom library from an existing program, just call library
  iocom functions from C or C++ code and ignore "framework style" code here.

  The Buster conroller example here uses static IO device configuration. This means that
  communication signal map from IO board JSON files, etc, is compiled into Buster's code ->
  run time matching IO signal at IO device and in Buster is by address and type, not by signal name.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "buster.h"

/* Buster application object. */
Application app;

/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
 */
EOSAL_C_MAIN


/**
****************************************************************************************************

  @brief The controller program entry point.

  Initialize IOCOM and start the IO controller application.

  - osal_simulated_loop: When emulating micro-controller on PC, run loop. Just save context
    pointer on real micro-controller.

  @oaran   argc Number of command line arguments (PC only)
  @oaran   argv Array of command line argument pointers (PC only)
  @return  OSAL_SUCCESS if all fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    app.start(argc, (const os_char**)argv);
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
    os_timer ti;

static long ulledoo; if (++ulledoo > 10009) {osal_debug_error("ulledoo app\n"); ulledoo = 0;}

    os_get_timer(&ti);
    return app.run(&ti);
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
    app.stop();
}
