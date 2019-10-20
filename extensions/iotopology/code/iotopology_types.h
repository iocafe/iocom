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
#ifndef IOTOPOLOGY_INCLUDED
#define IOTOPOLOGY_INCLUDED


typedef struct
{
}
iocCertificate;

typedef struct
{

}
iocKey;


typedef struct iocAuthority
{
    struct iocAuthority *next;
}
iocAuthority;

typedef struct iocAuthentication
{
    /** Name of authenticated node, for example GRUMPYBORG.
        If asterix "*", then all node names are accepted.
     */
    os_char *none_name;

    /** Name of authenticated IO device network, for example PEKKA.
        If asterix "*", then all node names are accepted.
     */
    os_char *network_name;

    struct iocAuthentication *next;
}
iocAuthentication;

typedef struct iocNetworkConnection
{
    os_int protocol;
    os_char *parameters;

    struct iocNetworkConnection *next;
}
iocNetworkConnection;


/**
****************************************************************************************************
    Node is either IO device or IO domain controller.
****************************************************************************************************
*/
typedef struct
{
    /** Name of this node, for example GRUMPYBORG.
     */
    os_char *none_name;

    /** Name of this IO device network, for example PEKKA.
     */
    os_char *network_name;

    /** Linked list of IP addressess of IO domain controllers to connect. Usually there
        is one connection, but list presentation is used to allow redundant connections
        in future.
     */
    iocNetworkConnection *connect;

    /** Private key of the node.
     */
    iocKey *private_key;

    /** Public key of the node.
     */
    iocKey *public_key;

    /** Client certificate. Used to identify this device (or controller) upwards to IO domain
       controller.
     */
    iocCertificate *client_cert;

    /** Linked list of authorities (a certificate signed by authority is accepted)
     */
    struct iocAuthority *authorities;

    /** Controller only: Linked list of IP protocols/addressess/socket ports to listen.
        There may be more than one, for example if our controller listens for both TLS and serial
        communication.
      */
    iocNetworkConnection *listen;

    /** Controller only: Linked list of nodes authenticated to connect to this one. Basically
     *  we can do security without this: If an IO device is breached and we need to
     *  revoke it's access rights we could maintain revokation list.
     */
    iocAuthentication *authentication;

}
iocNode;


#endif
