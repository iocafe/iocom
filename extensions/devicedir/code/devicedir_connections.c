/**

  @file    devicedir_conf.c
  @brief   List IO networks, devices, memory blocks and IO signals.
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
    os_boolean isfirst;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    osal_stream_write_str(list, "{\"connections\": [", 0);

    /* Synchronize.
     */
    ioc_lock(root);


    for (con = root->con.first;
         con;
         con = con->link.next)
    {
        if (con->iface == OSAL_SOCKET_IFACE)
        {
            iface_name = (con->flags & IOC_SOCKET) ? "socket" : "socket MISMATCH";
        }
#if OSAL_TLS_SUPPORT
        else if (con->iface == OSAL_TLS_IFACE)
        {
            iface_name = (con->flags & IOC_SOCKET) ? "tls" : "tls MISMATCH";
        }
#endif
#if OSAL_SERIAL_SUPPORT
        else if (con->iface == OSAL_SERIAL_IFACE)
        {
            iface_name = (con->flags & IOC_SOCKET) ? "serial MISMATCH" : "serial";
        }
#endif
#if OSAL_BLUETOOTH_SUPPORT
        else if (con->iface == OSAL_BLUETOOTH_IFACE)
        {
            iface_name = (con->flags & IOC_SOCKET) ? "bluetooth MISMATCH" : "bluetooth";
        }
#endif
        else
        {
            iface_name = "unknown";
        }

        osal_stream_write_str(list, "{\"iface\":\"", 0);
        osal_stream_write_str(list, iface_name, 0);
        osal_stream_write_str(list, "\"", 0);

        osal_stream_write_str(list, ", \"param\":\"", 0);
        osal_stream_write_str(list, con->parameters, 0);
        osal_stream_write_str(list, "\"", 0);


        osal_stream_write_str(list, ", \"flags\":\"", 0);
        isfirst = OS_TRUE;
        if (con->flags & IOC_DYNAMIC_MBLKS)
        {
            if (!isfirst) osal_stream_write_str(list, ",", 0);
            osal_stream_write_str(list, "dynamic", 0);
            isfirst = OS_FALSE;
        }
        if (con->flags & IOC_LISTENER)
        {
            if (!isfirst) osal_stream_write_str(list, ",", 0);
            osal_stream_write_str(list, "listener", 0);
            isfirst = OS_FALSE;
        }
        if (con->flags & IOC_CREATE_THREAD)
        {
            if (!isfirst) osal_stream_write_str(list, ",", 0);
            osal_stream_write_str(list, "thread", 0);
            isfirst = OS_FALSE;
        }
        if (con->flags & IOC_CLOSE_CONNECTION_ON_ERROR)
        {
            if (!isfirst) osal_stream_write_str(list, ",", 0);
            osal_stream_write_str(list, "closeonerr", 0);
            isfirst = OS_FALSE;
        }
        osal_stream_write_str(list, "\"", 0);

        osal_stream_write_str(list, "}", 0);
        if (con->link.next)
        {
            osal_stream_write_str(list, ",", 0);
        }
    }

    /* End synchronization.
     */
    ioc_unlock(root);

    osal_stream_write_str(list, "]}\n", 0);
}


