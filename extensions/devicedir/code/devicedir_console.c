/**

  @file    devicedir_console.c
  @brief   Console window for CLI to give software test commands and see the output.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"
#if OS_CONTROL_CONSOLE_SUPPORT

static osalStatus io_console_line_edit(
    ioDeviceConsole *console,
    os_uint c);

static void iocom_state_list(
    ioDeviceConsole *console,
    os_char select);


void io_initialize_device_console(
    ioDeviceConsole *console,
    iocRoot *root)
{
    os_memclear(console, sizeof(ioDeviceConsole));
    console->root = root;
}

// @return OSAL_NOTHING_TO_DO if nothing was done OSAL_SUCCESS if keypress was processed.
osalStatus io_run_device_console(
    ioDeviceConsole *console)
{
    os_uint c;
    osalStatus s;

    c = osal_console_read();
    if (c == 0) return OSAL_SUCCESS; // OSAL_NOTHING_TO_DO;

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
            osal_global->exit_process = OS_TRUE;
            return OSAL_END_OF_FILE;

        case OSAL_CONSOLE_ENTER:
        case '?':
        case 'h':
        case 'H':
            osal_console_write("\nc=connections, e=end points, m=memory blocks, i=info, d=dynamic, q=quiet, t=talkative, s=set, x=exit\n");
            break;

        case 'c':
        case 'C':
            iocom_state_list(console, 'c');
            break;

        case 'e':
        case 'E':
            iocom_state_list(console, 'e');
            break;

        case 'm':
        case 'M':
            iocom_state_list(console, (os_char)c);
            break;

        case 'd':
        case 'D':
            iocom_state_list(console, 'd');
            break;

        case 'i':
        case 'I':
            iocom_state_list(console, 'i');
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

  @param   console Pointer to console structure.
  @param   ch Character forresponding to key press.
  @return  OSAL_SUCCESS to continue editing or OSAL_COMPLETED to dinish with line editing.

****************************************************************************************************
*/
static osalStatus io_console_line_edit(
    ioDeviceConsole *console,
    os_uint c)
{
    os_char echobuf[2];

    switch (c)
    {
        case OSAL_CONSOLE_ESC:
            return OSAL_COMPLETED;

        case OSAL_CONSOLE_ENTER:
            osal_console_write("\n*** ");
            osal_console_write(console->line_buf);
            osal_console_write(" ***\n");

            return OSAL_COMPLETED;

        case OSAL_CONSOLE_BACKSPACE:
            if (console->pos > 0)
            {
                console->line_buf[--(console->pos)] = '\0';
                osal_console_write("\b");
            }
            break;

        default:
            if (console->pos < OS_CONSOLE_LINE_BUF_SZ-1)
            {
                console->line_buf[console->pos++] = (os_char)c;
                echobuf[0] = (os_char)c;
                echobuf[1] = '\0';
                osal_console_write(echobuf);
            }
            break;
    }

    return OSAL_SUCCESS;
}


static void iocom_state_list(
    ioDeviceConsole *console,
    os_char select)
{
    iocRoot *root;
    osalStream stream;
    os_char *p;
    os_memsz n;

    root = console->root;
    stream = osal_stream_buffer_open(OS_NULL, 0, OS_NULL, 0);

    switch (select)
    {
        case 'c':
            osal_console_write("\n*** connections ***\n");
            devicedir_connections(root, stream, 0);
            break;

        case 'e':
            osal_console_write("\n*** end points ***\n");
            devicedir_end_points(root, stream, 0);
            break;

        case 'm':
            osal_console_write("\n*** memory blocks ***\n");
            devicedir_memory_blocks(root, stream, OS_NULL, 0);
            break;

        case 'M':
            osal_console_write("\n*** memory blocks ***\n");
            devicedir_memory_blocks(root, stream, OS_NULL, IOC_DEVDIR_BUFFERS|IOC_DEVDIR_DATA);
            break;

        case 'i':
            devicedir_info(root, stream, 0);
            break;

        case 'd':
#if IOC_DYNAMIC_MBLK_CODE
            osal_console_write("\n*** dynamic signal info ***\n");
            devicedir_dynamic_signals(root, stream, OS_NULL, 0);
#else
            osal_console_write("\nDynamic signal support not included in build\n");
#endif
            break;
    }

    osal_stream_write(stream, "\0", 1, &n, OSAL_STREAM_DEFAULT);
    p = osal_stream_buffer_content(stream, &n);

    osal_console_write(p);

    osal_stream_close(stream, OSAL_STREAM_DEFAULT);
}

#endif

