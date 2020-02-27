/**

  @file    nodeconf_lighthouse.h
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


#define IOC_LIGHTHOUSE_HAS_TCP_SOCKET 1
#define IOC_LIGHTHOUSE_HAS_TLS_SOCKET 2

/* Information anout one light house end point.
 */
typedef struct iocLighthouseEndPointInfo
{
    /** Transport, either IOC_TLS_SOCKET or IOC_TCP_SOCKET.
     */
    iocTransportEnum transport;

    /** TCP port number listened by server.
     */
    os_int port_nr;

    /** OS_TRUE for IPv6 or OS_FALSE for IPv4.
     */
    os_boolean is_ipv6;
} 
iocLighthouseEndPointInfo;

/* Maximum number of end points to store into info
 */
#define IOC_LIGHTHOUSE_INFO_MAX_END_POINTS 4

/* Information for light house (multicast device discovery) from node configuration.
 */
typedef struct iocLighthouseInfo
{
    /** End point array
     */
    iocLighthouseEndPointInfo epoint[IOC_LIGHTHOUSE_INFO_MAX_END_POINTS];

    /** Number of end points in array.
     */
    os_int n_epoints;
}
iocLighthouseInfo;

/* Get listening socket port number and transport.
 */
osalStatus ioc_get_lighthouse_info(
    iocConnectionConfig *connconf,
    iocLighthouseInfo *info);
