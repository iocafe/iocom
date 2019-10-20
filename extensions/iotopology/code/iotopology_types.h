/**

  @file    iotopology.h
  @brief   Data structures, defines and functions for managing network topology and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/


#define IOTOPOLOGY_NODE_NAME_SZ 16
#define IOTOPOLOGY_NETWORK_NAME_SZ 24


typedef struct
{
    os_char *network_name;
}
iotopologyCertificate;

typedef struct
{

}
iotopologyKey;


typedef struct iocAuthority
{
    struct iocAuthority *next;
}
iotopologyAuthority;

/** An IO device (or controller below) authorized to connect to this one.
 */
typedef struct iotopologyAuthorization
{
    /** Name of authenticated node, for example GRUMPYBORG.
        If asterix "*", then all node names are accepted.
     */
    os_char node_name[IOTOPOLOGY_NODE_NAME_SZ];

    /** Name of authenticated IO device network, for example PEKKA.
        If asterix "*", then all node names are accepted.
     */
    os_char network_name[IOTOPOLOGY_NETWORK_NAME_SZ];

    /** Pointer to next authorization in linked list.
     */
    struct iotopologyAuthorization *next;
}
iotopologyAuthorization;


typedef struct iocNetworkConnection
{
    os_int flags;
    os_char parameters[IOC_CONNECTION_PRMSTR_SZ];

    struct iotopologyNetworkConnection *next;
}
iotopologyNetworkConnection;


/**
****************************************************************************************************
    Node is either IO device or IO domain controller.
****************************************************************************************************
*/
typedef struct
{
    /** Name of this node, for example GRUMPYBORG.
     */
    os_char node_name[IOTOPOLOGY_NODE_NAME_SZ];

    /** Name of this IO device network, for example PEKKA. This can be also in two parts,
        like VARKAUS.MIGHTYCORP.
     */
    os_char network_name[IOTOPOLOGY_NETWORK_NAME_SZ];

    /** Linked list of IP addressess of IO domain controllers to connect. Usually there
        is one connection, but list presentation is used to allow redundant connections
        in future.
     */
    iotopologyNetworkConnection *connect;

    /** Private key of the node.
     */
    iotopologyKey private_key;

    /** Public key of the node.
     */
    iotopologyKey public_key;

    /** Client certificate. Used to identify this device (or controller) upwards to IO domain
        controller.
     */
    iotopologyCertificate client_cert;

    /** Linked list of authorities (a certificate signed by authority is accepted)
     */
    iotopologyAuthority *authorities;

    /** Controller only: Server certificate. Used to identify this controller as legimate
        to IO devices and controllers below it.
     */
    iotopologyCertificate server_cert;

    /** Controller only: Linked list of IP protocols/addressess/socket ports to listen.
        There may be more than one, for example if our controller listens for both TLS and serial
        communication.
      */
    iotopologyNetworkConnection *listen;

    /** Controller only: Linked list of nodes authorized to connect to this one. Basically
        we could do security without this: Alternatively if an IO device is breached and we
        need to revoke it's access rights we could maintain revokation list.
     */
    iotopologyAuthorization *authorizations;

#if OSAL_MULTITHREAD_SUPPORT
    /** Mutex to synchronize access and modifications to node configuration, needed for
        multithread mode.
     */
    osalMutex lock;
#endif
}
iotopologyNode;

