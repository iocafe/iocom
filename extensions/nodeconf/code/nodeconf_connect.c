/**

  @file    nodeconf_connect.c
  @brief   Create connections and end points by node configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "nodeconf.h"


/**
****************************************************************************************************

  @brief Connect to cloud server.

  If the IO controller (like this 'frank' example) runs in local network, a cloud server
  can be used to pass connections from remote devices or other nodes. This function
  connects this IO controller to cloud server application like 'claudia'.

  @param   root Pointer to iocom root object.
  @param   connconf Connection configuration (from persistent storate or JSON defaults).
  @param   additional_flags Bits IOC_DYNAMIC_MBLKS or IOC_CREATE_THREAD can be listed,
           set zero for no additional flags.
  @return  If successful, the function returns OSAL_SUCCESS. If failed, even partially,
           the function returns error code.

****************************************************************************************************
*/
osalStatus ioc_connect_node(
    iocRoot *root,
    iocConnectionConfig *connconf,
    os_short additional_flags)
{
    iocEndPoint *ep = OS_NULL;
    iocEndPointParams epprm;
    iocConnection *con = OS_NULL;
    iocConnectionParams conprm;
    iocOneConnectionConf *c;
    const osalStreamInterface *iface;
    os_int i;
    osalStatus ss = OSAL_SUCCESS, s;
    os_short flags;

    for (i = 0; i < connconf->n_connections; i++)
    {
        c = connconf->connection + i;

        /* Get transport interface */
        switch (c->transport)
        {
            case IOC_DEFAULT_TRANSPORT:
            case IOC_TLS_SOCKET:  iface = OSAL_TLS_IFACE; flags = IOC_SOCKET; break;
            case IOC_TCP_SOCKET:  iface = OSAL_SOCKET_IFACE; flags = IOC_SOCKET; break;
            case IOC_SERIAL_PORT: iface = OSAL_SERIAL_IFACE; flags = IOC_SERIAL; break;
            case IOC_BLUETOOTH:   iface = OSAL_BLUETOOTH_IFACE; flags = IOC_SERIAL; break;
            default: iface = OS_NULL; flags = 0; break;
        }
        if (iface == OS_NULL) continue;

        flags |= additional_flags | c->flags;

        if (c->listen && (flags & IOC_SOCKET))
        {
            ep = ioc_initialize_end_point(OS_NULL, root);
            os_memclear(&epprm, sizeof(epprm));
            epprm.iface = iface;
            epprm.flags = flags;
            epprm.parameters = c->parameters;
            s = ioc_listen(ep, &epprm);
            if (s) ss = s;
        }
        else
        {
            /* If listening end of serial connection?
             */
            if (c->listen) flags |= IOC_LISTENER;

            con = ioc_initialize_connection(OS_NULL, root);
            os_memclear(&conprm, sizeof(conprm));
            conprm.iface = iface;
            conprm.flags = flags;
            conprm.parameters = c->parameters;
            s = ioc_connect(con, &conprm);
            if (s) ss = s;
        }
    }

    return ss;
}

