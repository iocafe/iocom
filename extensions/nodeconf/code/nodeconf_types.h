/**

  @file    nodeconf.h
  @brief   Data structures, defines and functions for managing network node configuration and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/

#define NODECONF_APP_NAME_SZ 16
#define NODECONF_APP_VERSION_SZ 8
#define NODECONF_VERSION_SZ  8
#define NODECONF_NODE_NAME_SZ 16
#define NODECONF_NETWORK_NAME_SZ 24
#define NODECONF_PASSWORD_SZ 16
#define NODECONF_CONNECTION_PRMSTR_SZ 48

#define NODECONF_IPADDR_SZ 40
#define NODECONF_MAC_SZ 24
#define NODECONF_OPTIONS_SZ 16
#define NODECONF_WIFI_PRM_SZ 16

#define NODECONF_MAX_NICS 2
#define NODECONF_MAX_CONNECTIONS 2
#define NODECONF_MAX_TRUSTED_AUTHORITIES 3


/**
****************************************************************************************************
    A server certificate.
****************************************************************************************************
*/
typedef struct
{
    os_char *data;
    os_memsz *data_sz;
}
nodeconfCertificate;


/**
****************************************************************************************************
    Structure for storing a key.
****************************************************************************************************
*/
typedef struct
{
    os_char *key;
    os_memsz *key_sz;
}
nodeconfKey;


/**
****************************************************************************************************
    A trusted authority who can sign server certificates.
****************************************************************************************************
*/
typedef struct
{
    /** Name of trusted network, for example SMOKECLOUD.
     */
    os_char network_name[NODECONF_NETWORK_NAME_SZ];

    // public key
}
nodeconfTrustedAuthority;


/**
****************************************************************************************************
    An IO device (or controller below) authorized to connect to this one.
****************************************************************************************************
*/
typedef struct nodeconfAuthorization
{
    /** Name of authenticated node, for example GRUMPYBORG.
        If asterix "*", then all node names are accepted.
     */
    os_char node_name[NODECONF_NODE_NAME_SZ];

    /** Name of authenticated IO device network, for example PEKKA.
        If asterix "*", then all node names are accepted.
     */
    os_char network_name[NODECONF_NETWORK_NAME_SZ];

    /** Frag indicating that this is received from higher level controller.
     */
    os_boolean inherited;

    /** Pointer to next authorization in linked list.
     */
    struct nodeconfAuthorization *next;
}
nodeconfAuthorization;


/**
****************************************************************************************************
    Specifies protocol, IP address and port of an IO controller to connect to.
****************************************************************************************************
*/
typedef struct
{
    os_short flags;
    os_char parameters[NODECONF_CONNECTION_PRMSTR_SZ];
}
nodeconfNetworkConnect;


/**
****************************************************************************************************
    Specifies protocol, port and possibly port to listen to.
****************************************************************************************************
*/
typedef struct nodeconfNetworkListen
{
    os_short flags;
    os_char parameters[NODECONF_CONNECTION_PRMSTR_SZ];

    struct nodeconfNetworkListen *next;
}
nodeconfNetworkListen;


/**
****************************************************************************************************
    Network interface setup for micro controllers.
****************************************************************************************************
*/
typedef struct
{
    os_char ip_address[NODECONF_IPADDR_SZ];
    os_char subnet_mask[NODECONF_IPADDR_SZ];
    os_char gateway_address[NODECONF_IPADDR_SZ];
    os_char dns_address[NODECONF_IPADDR_SZ];

    /* Locally administered MAC address ranges safe for testing: x2:xx:xx:xx:xx:xx,
       x6:xx:xx:xx:xx:xx, xA:xx:xx:xx:xx:xx and xE:xx:xx:xx:xx:xx
    */
    os_char mac[NODECONF_MAC_SZ];
    os_char options[NODECONF_OPTIONS_SZ]; /* dhcp, etc */

    os_char wifi_net_name[NODECONF_WIFI_PRM_SZ];
    os_char wifi_net_password[NODECONF_WIFI_PRM_SZ];
}
nodeconfNIC;


/**
****************************************************************************************************
    Basic IO network node configuration for both IO devices and controllers (flat structure)
****************************************************************************************************
*/
typedef struct
{
    /** Version of this structure.
     */
    os_char version[NODECONF_VERSION_SZ];

    /** Network interface configuration. Used only for embedded devices/micro-controllers.
     */
    nodeconfNIC nic[NODECONF_MAX_NICS];

    /** Name of this node, for example GRUMPYBORG.
     */
    os_char node_name[NODECONF_NODE_NAME_SZ];

    /** Name of this IO device network, for example PEKKA. This can be also in two parts,
        like VARKAUS.MIGHTYCORP.
     */
    os_char network_name[NODECONF_NETWORK_NAME_SZ];

    /** Array of IP addressess/ports of IO domain controllers to connect. In simple cases
        there is one connection upwards, but two are reserved for future redundant connection
        support.
     */
    nodeconfNetworkConnect connect[NODECONF_MAX_CONNECTIONS];

    /** Array of trusted authorities (a certificate signed by authority is accepted)
     */
    nodeconfTrustedAuthority trust[NODECONF_MAX_TRUSTED_AUTHORITIES];
}
nodeconfNodeBasics;


/**
****************************************************************************************************
    Extended node configuration for controllers.
****************************************************************************************************
*/
typedef struct
{
    /** Server certificate. Used to identify this controller as legimate
        to IO devices and controllers below it.
     */
    nodeconfCertificate server_cert;

    nodeconfKey public_key;
    nodeconfKey private_key;

    /** Controller only: Linked list of IP protocols/addressess/socket ports to listen.
        There may be more than one, for example if our controller listens for both TLS and serial
        communication.
      */
    nodeconfNetworkListen *listen;

    /** Controller only: Linked list of nodes authorized to connect to this one. Basically
        we could do security without this: Alternatively if an IO device is breached and we
        need to revoke it's access rights we could maintain revokation list.
     */
    nodeconfAuthorization *authorizations;
}
nodeconfNodeExts;


/**
****************************************************************************************************
    Data structure to describe network node configuration for one node. A node is either IO device or
    controller.
****************************************************************************************************
*/
typedef struct
{
    /** Basic IO network node configuration (flat structure)
     */
    nodeconfNodeBasics config;

    /** Extra information for IO controller (not flat)
     */
    nodeconfNodeExts *extconfig;

    /** Application name
     */
    os_char app_name[NODECONF_APP_NAME_SZ];

    /** Application version
     */
    os_char app_version[NODECONF_APP_VERSION_SZ];

#if OSAL_MULTITHREAD_SUPPORT

    /** The an IO device is identified by node name, network name and password.
        One directional hash by server?
     */
    os_char password[NODECONF_PASSWORD_SZ];

    /** Mutex to synchronize access and modifications to node configuration, needed for
        multithread mode.
     */
    osalMutex lock;
#endif
}
nodeconfNode;




