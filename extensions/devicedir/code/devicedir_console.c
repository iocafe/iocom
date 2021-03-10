/**

  @file    devicedir_console.c
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
#include "devicedir.h"
#if OSAL_CONTROL_CONSOLE_SUPPORT

/* Forward referred static functions.
 */
static osalStatus io_console_line_edit(
    ioDeviceConsole *console,
    os_uint ch);

static void io_console_print_json(
    ioDeviceConsole *console,
    ddSelectJSON select,
    const os_char *iopath,
    os_short flags);


/**
****************************************************************************************************

  @brief Initialize console structure.
  @anchor io_initialize_device_console

  The io_initialize_device_console function sets up console structure given as argument for use.

  @param   console Pointer to console structure to initialize.
  @param   root Pointer to the IOCOM root object.
  @return  None.

****************************************************************************************************
*/
void io_initialize_device_console(
    ioDeviceConsole *console,
    iocRoot *root)
{
    os_memclear(console, sizeof(ioDeviceConsole));
    console->root = root;
}


/**
****************************************************************************************************

  @brief Run console.
  @anchor io_run_device_console

  The io_run_device_console function should be called repeatedly from IO device's main loop, etc.
  The function reads user key pressed (from serial port, etc), processes these and prints
  replies to user commands.

  @param   console Pointer to console structure to initialize.
  @param   root Pointer to the IOCOM root object.
  @return  OSAL_SUCCESS to proceed with operation, OSAL_END_OF_FILE to exit/reboot the application
           or OSAL_COMPLETED to give "go ahead" for application specific function by pressing "g".

****************************************************************************************************
*/
osalStatus io_run_device_console(
    ioDeviceConsole *console)
{
    os_uint c;
    osalStatus s;

    c = osal_console_read();
    if (c == 0) return OSAL_SUCCESS;

    if (console->line_edit)
    {
        s = io_console_line_edit(console, c);
        if (s) {
            console->line_edit = OS_FALSE;
            osal_quiet(console->saved_quied);
            osal_console_write("\n");
        }
    }
    else switch (c)
    {
        case 'x':
        case 'X':
            osal_reboot(0);
            osal_global->exit_process = OS_TRUE;
            return OSAL_END_OF_FILE;

        case OSAL_CONSOLE_ENTER:
        case '?':
        case 'h':
        case 'H':
            osal_console_write("\nc=connections, e=end points, m=memory blocks, i=info, "
                "d=dynamic, q=quiet, t=talkative, s=set, o=show overrides, x=exit/reboot\n");
            osal_console_write("set: ");
            io_console_print_json(console, IO_DD_OVERRIDES, OS_NULL, IOC_HELP_MODE);
            osal_console_write("\n");
            break;

        case 'c':
        case 'C':
            io_console_print_json(console, IO_DD_CONNECTIONS, OS_NULL, 0);
            break;

        case 'e':
        case 'E':
            io_console_print_json(console, IO_DD_END_POINTS, OS_NULL, 0);
            break;

        case 'm':
            io_console_print_json(console, IO_DD_MEMORY_BLOCKS, OS_NULL, 0);
            break;

        case 'M':
            io_console_print_json(console, IO_DD_MEMORY_BLOCKS, OS_NULL,
                IOC_DEVDIR_BUFFERS|IOC_DEVDIR_DATA);
            break;

        case 'd':
        case 'D':
            io_console_print_json(console, IO_DD_DYNAMIC_SIGNALS, OS_NULL, 0);
            break;

        case 'i':
        case 'I':
            io_console_print_json(console, IO_DD_INFO, OS_NULL, 0);
            break;

        case 'o':
            io_console_print_json(console, IO_DD_OVERRIDES, OS_NULL, 0);
            break;

        case 'q': /* Disable debug prints */
            osal_quiet(OS_TRUE);
            osal_console_write("\nquiet mode...\n");
            break;

        case 't': /* Allow debug prints */
            osal_console_write("\ntalkative...\n");
            osal_quiet(OS_FALSE);
            break;

        case 's': /* Start line edit (settings) */
            console->line_edit = OS_TRUE;
            os_memclear(console->line_buf, OS_CONSOLE_LINE_BUF_SZ);
            console->pos = 0;
            console->saved_quied = osal_quiet(OS_TRUE);
            osal_console_write("\n>");
            break;

        case 'g':
        case 'G':
            return OSAL_COMPLETED;
            break;

        default:
            break;
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief User input line editor.
  @anchor io_console_line_edit

  The io_console_line_edit function processess user key presses when console is in line edit mode.
  This is very simple editor, it holds only one line buffer. Character keys pressed are appended
  to buffer, and back space removes last character from buffer. ESC key terminates editing,
  ENTER saves the values.

  @param   console Pointer to console structure.
  @param   ch A character corresponding to the key press.
  @return  OSAL_SUCCESS to continue editing or OSAL_COMPLETED to finish with line editing.

****************************************************************************************************
*/
static osalStatus io_console_line_edit(
    ioDeviceConsole *console,
    os_uint ch)
{
    os_char echobuf[2];

    switch (ch)
    {
        case OSAL_CONSOLE_ESC:
            return OSAL_COMPLETED;

        case OSAL_CONSOLE_ENTER:
            /* If factory reset?
             */
            if (!os_strnicmp(console->line_buf, "reset", -1))
            {
                os_persistent_delete(-1, OSAL_PERSISTENT_DELETE_ALL);
                osal_console_write("\nFactory reset done. Reboot the device with 'x'.\n");
            }

            /* Otherwise save configuration changes.
             */
            else
                {
                if (devicedir_save_config(console->line_buf) == OSAL_SUCCESS) {
                    osal_console_write("\noverride setting(s) saved.\n");
                }
                else {
                    osal_console_write("\nNO CHANGES TO KNOWN PARAMETERS.\n");
                }
            }
            return OSAL_COMPLETED;

        case OSAL_CONSOLE_BACKSPACE:
        case '\b':
            if (console->pos > 0) {
                console->line_buf[--(console->pos)] = '\0';
                osal_console_write("\b \b");
            }
            break;

        default:
            if (console->pos < OS_CONSOLE_LINE_BUF_SZ-1) {
                console->line_buf[console->pos++] = (os_char)ch;
                echobuf[0] = (os_char)ch;
                echobuf[1] = '\0';
                osal_console_write(echobuf);
            }
            break;
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Print selected information to console as JSON text.
  @anchor io_console_print_json

  The io_console_print_json function....

  @param   console Pointer to console structure.
  @param   select Selects which JSON to print.
  @return  None.

****************************************************************************************************
*/
static void io_console_print_json(
    ioDeviceConsole *console,
    ddSelectJSON select,
    const os_char *iopath,
    os_short flags)
{
    const os_char *label;
    osalStream stream;
    os_char *p;
    os_memsz n;

    stream = osal_stream_buffer_open(OS_NULL, 0, OS_NULL, 0);

    devicedir_get_json(console->root, stream, select, iopath, flags, &label);

    osal_console_write("\n*** ");
    osal_console_write(label);
    osal_console_write(" ***\n");

    p = osal_stream_buffer_content(stream, &n);
    osal_console_write(p);

    osal_stream_close(stream, OSAL_STREAM_DEFAULT);
}

#endif

