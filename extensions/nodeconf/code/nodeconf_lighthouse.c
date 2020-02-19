/**

  @file    nodeconf_lighthouse.c
  @brief   Get listening socket port number and transport.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "nodeconf.h"


/**
****************************************************************************************************

  @brief Get listening socket port number and transport.

  Get basic information about listening server socket for light house, etc, protocol.

  @param   connconf Connection configuration (from persistent storate or JSON defaults).
  @param   port_nr Pointer to integer where to store listened TCP port number.
  @param   transport Pointer where to set transport, either IOC_TLS_SOCKET or IOC_TCP_SOCKET.
  @return  If successfull, the function returns OSAL_SUCCESS. If end point information not
           available, the function returns OSAL_STATUS_FAILED.

****************************************************************************************************
*/
osalStatus ioc_get_lighthouse_info(
    iocConnectionConfig *connconf,
    os_int *port_nr,
    iocTransportEnum *transport)
{
    iocOneConnectionConf *c;
    const os_char *p, *e;
    os_int i, nr;

    for (i = 0; i < connconf->n_connections; i++)
    {
        c = connconf->connection + i;

        /* Get transport interface */
        if (c->transport != IOC_TLS_SOCKET && c->transport != IOC_TCP_SOCKET)
        {
            continue;
        }

        if (c->listen)
        {
            p = c->parameters;
            e = os_strchr((os_char*)p, ':');
            if (e) p = e + 1;
            nr = (os_int)osal_str_to_int(p, OS_NULL);
            if (nr == 0)
            {
                nr = c->transport == IOC_TCP_SOCKET
                    ? IOC_DEFAULT_SOCKET_PORT : IOC_DEFAULT_TLS_PORT;
            }

            *transport = c->transport;
            *port_nr = nr;
            return OSAL_SUCCESS;
        }
    }

    return OSAL_STATUS_FAILED;
}

