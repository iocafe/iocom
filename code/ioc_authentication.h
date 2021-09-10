/**

  @file    ioc_authentication.h
  @brief   Device/user authentication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Low level of user authentication and authorization. Handles serialization of authentication
  frames over connection and on server (IOC_FULL_AUTHENTICATION) works as interface between
  iocom and authentication code. Notice that ioserver extension library contains default
  server authentication which should be sufficient for simpler applications.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_AUTHENTICATION_H_
#define IOC_AUTHENTICATION_H_
#include "iocom.h"

struct iocRoot;
struct iocConnection;

/**
****************************************************************************************************
  Flags in authentication frame and in iocAllowedNetwork structure.
****************************************************************************************************
*/
#define IOC_AUTH_ADMINISTRATOR 1
#define IOC_AUTH_NO_CERT_CHAIN 4
#define IOC_AUTH_CLOUD_CON 8
#define IOC_AUTH_CONNECT_UP 16
#define IOC_AUTH_DEVICE_NR_2_BYTES 32
#define IOC_AUTH_DEVICE_NR_4_BYTES 64
#define IOC_AUTH_BIDIRECTIONAL_COM 128

/**
****************************************************************************************************
  User account.
****************************************************************************************************
*/
typedef struct iocUser
{
    /** Device or user name, including serial number (if applicable).
     */
    os_char user_name[IOC_DEVICE_ID_SZ];

    /** Network name. Empty string = any network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Password (cryprographic hash).
     */
    os_char password[IOC_PASSWORD_SZ];

    /** Flags (privileges, etc)
     */
    os_ushort flags;
}
iocUser;


/**
****************************************************************************************************
  Networks allowed tough a specific connection.
  When a device connects to server it identifies itself as "gina3.cafenet", etc. This
  identification is matched to user accounts in server, resulting accepted or terminated
  connection and set of IO networks which can be accessed tough this connection.
  This list is stored in iocConnection structure as iocAllowedNetworkConf.
  The iocAllowedNetwork names one allowed network, like "cafenet". Allowed privileges
  are stored in flags. IOC_AUTH_ADMINISTRATOR bit indicates that administrative
  (configuration and software update) privileges.
****************************************************************************************************
*/
typedef struct iocAllowedNetwork
{
    /** Network name. Empty string = any network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Flags (privileges, etc)
     */
    os_ushort flags;
}
iocAllowedNetwork;

typedef struct iocAllowedNetworkConf
{
    iocAllowedNetwork *network;
    os_int n_networks;

    /** Number of bytes allocated for network array.
     */
    os_memsz bytes;
}
iocAllowedNetworkConf;

/**
****************************************************************************************************
  Authentication functions
****************************************************************************************************
 */
/*@{*/

/* Make authentication data frame.
 */
void ioc_make_authentication_frame(
    struct iocConnection *con);

/* Process received authentication data frame.
 */
osalStatus ioc_process_received_authentication_frame(
    struct iocConnection *con,
    os_uint mblk_id,
    os_char *data);

#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION

/* Authentication function type. This is called trough a function pointer to allow
   implementing user authentication mechanism for application. ioserver extension
   library contains the default iocom user authentication.
   The allowed_networks is structure set up to hold list of networks which can be
   accessed trough the connection and privileges for each network. Must be released
   by ioc_release_allowed_networks().
 */
typedef osalStatus ioc_authorize_user_func(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUser *user_account,
    os_char *ip,
    void *context);

/* Enable user authentication (set authentication callback function).
 */
void ioc_enable_user_authentication(
    struct iocRoot *root,
    ioc_authorize_user_func *func,
    void *context);

/* Add a network to allowed networks structure
 */
void ioc_add_allowed_network(
    iocAllowedNetworkConf *allowed_networks,
    const os_char *network_name,
    os_ushort flags);

/* Release allowed networks structure set up by ioc_authorize_user_func()
 */
void ioc_release_allowed_networks(
    iocAllowedNetworkConf *allowed_networks);

/* Check if network is authorized.
 */
os_boolean ioc_is_network_authorized(
    struct iocConnection *con,
    os_char *network_name,
    os_ushort flags);

#endif

/*@}*/

#endif
