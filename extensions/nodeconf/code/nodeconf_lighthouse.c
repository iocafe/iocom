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

  Notice: This is typically part of start up. Here we cannot assume that network is initialized, 
  so osal_socket_get_ip_and_port() cannot be used.

  @param   connconf Connection configuration (from persistent storate or JSON defaults).
  @param   info Network information structure to fill for light house. Contains listened TCP port 
           number,  Transport, either IOC_TLS_SOCKET or IOC_TCP_SOCKET and .
  @return  If successfull, the function returns OSAL_SUCCESS. If end point information not
           available, the function returns OSAL_STATUS_FAILED.

****************************************************************************************************
*/
osalStatus ioc_get_lighthouse_info(
    iocConnectionConfig *connconf,
    iocLighthouseInfo *info)
{
    iocOneConnectionConf *c;
    const os_char *p, *e;
    os_int i, nr, n;
    osalStatus s = OSAL_STATUS_FAILED;

    os_memclear(info, sizeof(iocLighthouseInfo));
    n = 0;

    for (i = 0; i < connconf->n_connections; i++)
    {
        c = connconf->connection + i;

        /* Lighthouse deals only with TCP and TLS socket end point information.
         */
        if (c->transport != IOC_TLS_SOCKET && c->transport != IOC_TCP_SOCKET)
        {
            continue;
        }

        if (c->listen)
        {
            p = c->parameters;
            if (p == OS_NULL) continue;

            e = os_strchr((os_char*)p, ']');
            if (e) p = e + 1;
            e = os_strchr((os_char*)p, ':');
            if (e) p = e + 1;
            nr = (os_int)osal_str_to_int(p, OS_NULL);
            if (nr == 0)
            {
                nr = c->transport == IOC_TCP_SOCKET
                    ? IOC_DEFAULT_SOCKET_PORT : IOC_DEFAULT_TLS_PORT;
            }
            info->epoint[n].transport = c->transport;
            info->epoint[n].port_nr = nr;
            info->epoint[n].is_ipv6 = (os_boolean)(c->parameters[0] == '[');
            n++;
            s = OSAL_SUCCESS;
        }
    }

    info->n_epoints = n;
    return s;
}

