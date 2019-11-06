/**

  @file    devicedir_end_points.c
  @brief   List end points.
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

  @brief List end points connections of this node.

  The devicedir_end_points() function lists connections.

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write connection list JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_end_points(
    iocRoot *root,
    osalStream list,
    os_short flags)
{
    iocEndPoint *epoint;
    os_char *iface_name;
    os_short eflags;
    os_boolean isfirst;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    osal_stream_write_str(list, "{\"epoint\": [\n", 0);

    /* Synchronize.
     */
    ioc_lock(root);

    for (epoint = root->epoint.first;
         epoint;
         epoint = epoint->link.next)
    {
        eflags = epoint->flags;
        if (epoint->iface == OSAL_SOCKET_IFACE)
        {
            iface_name = (eflags & IOC_SOCKET) ? "socket" : "socket MISMATCH";
        }
#if OSAL_TLS_SUPPORT
        else if (epoint->iface == OSAL_TLS_IFACE)
        {
            iface_name = (eflags & IOC_SOCKET) ? "tls" : "tls MISMATCH";
        }
#endif
        else
        {
            iface_name = "unknown";
        }

        osal_stream_write_str(list, "{", 0);
        devicedir_append_str_param(list, "iface", iface_name, OS_TRUE);
        devicedir_append_str_param(list, "param", epoint->parameters, OS_FALSE);

        osal_stream_write_str(list, ", \"flags\":\"", 0);
        isfirst = OS_TRUE;
        if (eflags & IOC_DYNAMIC_MBLKS) devicedir_append_flag(list, "dynamic", &isfirst);
        if (eflags & IOC_LISTENER) devicedir_append_flag(list, "listener", &isfirst);
        if (eflags & IOC_CREATE_THREAD) devicedir_append_flag(list, "thread", &isfirst);
        if (eflags & IOC_CLOSE_CONNECTION_ON_ERROR) devicedir_append_flag(list, "closeonerr", &isfirst);
        osal_stream_write_str(list, "\"", 0);

        osal_stream_write_str(list, "}", 0);
        if (epoint->link.next)
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


