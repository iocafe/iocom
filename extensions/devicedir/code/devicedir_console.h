/**

  @file    devicedir_console.h
  @brief   CLI to give software test commands and see the output.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    25.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

#if OS_CONTROL_CONSOLE_SUPPORT

#define OS_CONSOLE_LINE_BUF_SZ 256

/* Flat device console state structure.
 */
typedef struct ioDeviceConsole
{
    iocRoot *root;

    os_char line_buf[OS_CONSOLE_LINE_BUF_SZ];
    os_int pos;

    os_boolean line_edit;
    os_boolean saved_quied;
}
ioDeviceConsole;

/* Initialize device console.
 */
void io_initialize_device_console(
    ioDeviceConsole *console,
    iocRoot *root);

/* Keep the device console alive, call from loop repeatedly.
 */
osalStatus io_run_device_console(
    ioDeviceConsole *console);

/* Macro to declare static ioDevice console structure.
 */
#define IO_DEVICE_CONSOLE(name) ioDeviceConsole name

/* Save wifi configuration from console to persistent storage.
 */
osalStatus devicedir_save_config(
  os_char *line_buf);

#else

#define IO_DEVICE_CONSOLE(name)
#define io_initialize_device_console(c, r)
#define io_run_device_console(r) (OSAL_SUCCESS)

#endif

