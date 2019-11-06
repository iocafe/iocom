/**

  @file    devicedir_connections.c
  @brief   List connections.
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


/**
****************************************************************************************************

  @brief List connections of this node.

  The devicedir_connections() function lists connections.

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write connection list JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_connections(
    iocRoot *root,
    osalStream list,
    os_short flags)
{
    iocConnection *con;
    os_char *iface_name;
    os_short cflags;
    os_boolean isfirst;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    osal_stream_write_str(list, "{\"con\": [\n", 0);

    /* Synchronize.
     */
    ioc_lock(root);

    for (con = root->con.first;
         con;
         con = con->link.next)
    {
        cflags = con->flags;

        if (con->iface == OSAL_SOCKET_IFACE)
        {
            iface_name = (cflags & IOC_SOCKET) ? "socket" : "socket MISMATCH";
        }
#if OSAL_TLS_SUPPORT
        else if (con->iface == OSAL_TLS_IFACE)
        {
            iface_name = (cflags & IOC_SOCKET) ? "tls" : "tls MISMATCH";
        }
#endif
#if OSAL_SERIAL_SUPPORT
        else if (con->iface == OSAL_SERIAL_IFACE)
        {
            iface_name = (cflags & IOC_SOCKET) ? "serial MISMATCH" : "serial";
        }
#endif
#if OSAL_BLUETOOTH_SUPPORT
        else if (con->iface == OSAL_BLUETOOTH_IFACE)
        {
            iface_name = (cflags & IOC_SOCKET) ? "bluetooth MISMATCH" : "bluetooth";
        }
#endif
        else
        {
            iface_name = "unknown";
        }

        osal_stream_write_str(list, "{", 0);
        devicedir_append_str_param(list, "iface", iface_name, OS_TRUE);
        devicedir_append_str_param(list, "param", con->parameters, OS_FALSE);

        osal_stream_write_str(list, ", \"flags\":\"", 0);
        isfirst = OS_TRUE;
        if (cflags & IOC_DYNAMIC_MBLKS) devicedir_append_flag(list, "dynamic", &isfirst);
        if (cflags & IOC_LISTENER) devicedir_append_flag(list, "listener", &isfirst);
        if (cflags & IOC_CREATE_THREAD) devicedir_append_flag(list, "thread", &isfirst);
        if (cflags & IOC_CLOSE_CONNECTION_ON_ERROR) devicedir_append_flag(list, "closeonerr", &isfirst);
        osal_stream_write_str(list, "\"", 0);

        osal_stream_write_str(list, "}", 0);
        if (con->link.next)
        {
            osal_stream_write_str(list, ",", 0);
        }
        osal_stream_write_str(list, "\n", 0);
    }

    /* End synchronization.
     */
    ioc_unlock(root);

    osal_stream_write_str(list, "]}\n", 0);
}