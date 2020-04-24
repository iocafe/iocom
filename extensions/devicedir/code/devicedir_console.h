/**

  @file    devicedir_conf.h
  @brief   List IO networks, devices, memory blocks and IO signals.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

#if OS_CONTROL_CONSOLE_SUPPORT

typedef struct ioDeviceConsole
{
    iocRoot *root;

}
ioDeviceConsole;


void io_initialize_device_console(
    ioDeviceConsole *console,
    iocRoot *root);

osalStatus io_run_device_console(
    ioDeviceConsole *console);

#define IO_DEVICE_CONSOLE(name) ioDeviceConsole name

#else

#define IO_DEVICE_CONSOLE(name) ioDeviceConsole name
#define io_initialize_device_console(c, r)
#define io_run_device_console(r) (OSAL_SUCCESS)

#endif

