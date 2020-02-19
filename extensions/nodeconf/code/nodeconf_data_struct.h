/**

  @file    nodeconf_data_struct.h
  @brief   Data structures, defines and functions for accessing network node configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Maximum number of connection points
 */
#define IOC_MAX_NCONF_CONNECTIONS 3


typedef struct iocDeviceId
{
    const os_char *device_name;
    os_uint device_nr;
    const os_char *network_name;

    /* User name is often device name and serial number, but can be something else
       for GUI client, etc, which discover's its device number automatically.
     */
    const os_char *user_name;
    const os_char *password;

    /* List of IO networks (user accounts) published by this device.
       Comma is used as separator.
     */
    const os_char *publish;

    /* Custom parmeters.
     */
    const os_char *cust1;
    const os_char *cust2;
}
iocDeviceId;

/* Structure for passing information about all network interfaces
 */
typedef struct osalNetworkInterfaces
{
    osalNetworkInterface *nic;
    os_int n_nics;
}
iocNetworkInterfaces;

/* Structure for passing information about all WiFi networks
 */
typedef struct osalWifiNetworks
{
    osalWifiNetwork *wifi;
    os_int n_wifi;
}
osalWifiNetworks;

/** Structure for passing information about all network interfaces
 */
typedef struct iocOneConnectionConf
{
    /** IP address and optional TCP port, serial port and parameters, etc.
     */
    const char *parameters;

    /** Which transport to use: socket, TLS, serial or blue tooth.
     */
    iocTransportEnum transport;

    /** Bit flags for connection: IOC_CLOUD_CONNECTION and IOC_CONNECT_UP bits,
        same bits as for iocConnection and iocEndPoint.
     */
    os_short flags;

    /** This connection point listens for incoming connections and
        doesn't actively connect.
     */
    os_boolean listen;
}
iocOneConnectionConf;


/** Structure for passing information about all connection points
 */
typedef struct iocConnectionConfig
{
    iocOneConnectionConf *connection;
    os_int n_connections;
}
iocConnectionConfig;


/** Node configuration state structure.
 */
typedef struct iocNodeConf
{
    iocDeviceId device_id;

    /** Array of network interfaces.
     */
    osalNetworkInterface nic[OSAL_MAX_NRO_NICS];

    /** Structure to map network interfaces in array together.
     */
    iocNetworkInterfaces nics;

    /** Array of wifi networks (in JSON).
     */
    osalWifiNetwork wifi[OSAL_MAX_NRO_WIFI_NETWORKS];

    /** Wifi can alternatively set in persistent block OS_PBNR_WIFI.
     */
    osalWifiPersistent wifi_pbnr_wifi;

    /** Structure for passing information about all WiFi networks
     */
    osalWifiNetworks wifis;

    /** Security configuration, user name, password, trusted parties, certificates.
     */
    osalSecurityConfig security_conf;

    /** Array of connections points.
     */
    iocOneConnectionConf connection[IOC_MAX_NCONF_CONNECTIONS];

    /** Structure to map connection points together.
     */
    iocConnectionConfig connections;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    /** Dynamically allocated buffer for loaded persistent configuration.
        OS_NULL if buffer is not needed.
     */
    os_char *allocated_buf;
    os_memsz allocated_sz;
#endif
}
iocNodeConf;

/* Get device identification and custom parameters.
 */
iocDeviceId *ioc_get_device_id(
    iocNodeConf *node);

/* Get network interface configuration.
 */
iocNetworkInterfaces *ioc_get_nics(
    iocNodeConf *node);

/* Get wifi network interface configuration.
 */
osalWifiNetworks *ioc_get_wifis(
    iocNodeConf *node);

/* Get network interface configuration.
 */
osalSecurityConfig *ioc_get_security_conf(
    iocNodeConf *node);

/* Get connection configuration.
 */
iocConnectionConfig *ioc_get_connection_conf(
    iocNodeConf *node);
