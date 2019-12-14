/**

  @file    nodeconf_data_struct.h
  @brief   Data structures, defines and functions for accessing network node configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    14.12.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Maximum number of connection points
 */
#define IOC_MAX_CONNECT_POINTS 3

/* Structure for passing information about all network interfaces
 */
typedef struct osalNetworkInterfaces
{
    osalNetworkInterface2 *nic;
    os_int n_nics;
}
iocNetworkInterfaces;

/* Transport types.
 */
typedef enum iocTransportEnum
{
    IOC_TCP_SOCKET,
    IOC_TLS_SOCKET,
    IOC_SERIAL_PORT,
    IOC_BLUETOOTH
}
iocTransportEnum;


/** Structure for passing information about all network interfaces
 */
typedef struct iocConnectionConfig
{
    /** IP address and optional TCP port, serial port and parameters, etc.
     */
    const char *parameters;

    /** Which transport to use: socket, TLS, serial or blue tooth.
     */
    iocTransportEnum transport;

    /** This is connection downwards in network hierarchy
     */
    os_boolean downward;

    /** This connection point listens for incoming connections and
        doesn't actively connect.
     */
    os_boolean listen;
}
iocConnectPoint;


/** Structure for passing information about all connection points
 */
typedef struct iocConnectPoints
{
    iocConnectPoint *connect_point;
    os_int n_connect_points;
}
iocConnectPoints;

typedef struct iocDeviceId
{
    const os_char *device_name;
    os_int device_nr;
    const os_char *network_name;
    const os_char *password;
}
iocDeviceId;


/** Node configuration state structure.
 */
typedef struct iocNodeConf
{
    iocDeviceId device_id;

    /* Array of network interfaces.
     */
    osalNetworkInterface2 nic[OSAL_MAX_NRO_NICS];

    /* Strtucture to map network interfaces in array together.
     */
    iocNetworkInterfaces nics;

    /* Security configuration, user name, password, trusted parties, certificates.
     */
    osalSecurityConfig security_conf;

    /* Array of connection points.
     */
    iocConnectPoint cpoint[IOC_MAX_CONNECT_POINTS];

    /* Strtucture to map connection points together.
     */
    iocConnectPoints cpoints;
}
iocNodeConf;


/* Applications usually include custom configuration related how the board is used, etc.
 * Instead of trying to imagine and list all possibilities, unfixed customization
 * parameters can be added.
*/
/* void nodeconf_get_custom_conf(
    iocNodeConf *node,
    osalCusomNodeConf *conf); */

/* Get network interface configuration.
 */
iocNetworkInterfaces *ioc_get_nics(
    iocNodeConf *node);

/* Get network interface configuration.
 */
osalSecurityConfig *ioc_get_security_conf(
    iocNodeConf *node);

/* Get connection configuration.
 */
iocConnectPoints *ioc_get_connect_conf(
    iocNodeConf *node);



/* void nodeconf_verify_server(
    iocNodeConf *node,
    os_char *node_name,
    os_char *network_name); */

/* Server side client authentication elsewhere, not in nodeconf
 */
