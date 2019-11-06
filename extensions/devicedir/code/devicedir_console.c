/**

  @file    devicedir_console.c
  @brief   Console window for CLI to give software test commands and see the output.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.11.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/devicedir/devicedir.h"
#if OS_CONTROL_CONSOLE_SUPPORT

static void iocom_state_list(
    iocRoot *root,
    os_char select)
{
    osalStream stream;
    os_char *p;
    os_memsz n;

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
            devicedir_memory_blocks(root, stream, 0);
            break;
    }

    osal_stream_write(stream, "\0", 1, &n, OSAL_STREAM_DEFAULT);
    p = osal_stream_buffer_content(stream, &n);

    osal_console_write(p);

    osal_stream_close(stream);
}


osalStatus io_device_console(
    iocRoot *root)
{
    os_ulong c;

    c = osal_console_read();

    switch (c)
    {
        case OSAL_CONSOLE_ESC:
            osal_global->exit_process = OS_TRUE;
            return OSAL_END_OF_FILE;

        case OSAL_CONSOLE_ENTER:
        case '?':
        case 'h':
        case 'H':
            osal_console_write("\nc=connections, e=end points, m=memory blocks\n");
            break;

        case 'c':
        case 'C':
            iocom_state_list(root, 'c');
            break;

        case 'm':
        case 'M':
            iocom_state_list(root, 'm');
            break;

        case 'e':
        case 'E':
            iocom_state_list(root, 'e');
            break;

        default:
            break;
    }

    return OSAL_SUCCESS;
}

#endif
