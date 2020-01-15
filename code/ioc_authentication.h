/**

  @file    ioc_authentication.h
  @brief   Device/user authentication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

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
#if IOC_AUTHENTICATION_CODE

struct iocRoot;
struct iocConnection;

/**
****************************************************************************************************
  Flags in authentication frame and in iocAllowedNetwork structure.
****************************************************************************************************
*/
#define IOC_AUTH_ADMINISTRATOR 1
#define IOC_AUTH_CONNECT_UP 16
#define IOC_AUTH_DEVICE_NR_2_BYTES 32
#define IOC_AUTH_DEVICE_NR_4_BYTES 64
#define IOC_AUTH_BIDIRECTIONAL_COM 128

#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION

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
    os_char password[IOC_NAME_SZ];

    /** Flags (priviliges, etc)
     */
    os_ushort flags;
}
iocUser;


/**
****************************************************************************************************
  Networks allowed tough a specific connection.
  When a device connects to server it identifies itself as "gina3.iocafenet", etc. This
  identification is matched to user accounts in server, resulting accepted or terminated
  connection and set of IO networks which can be accessed tough this connection.
  This list is stored in iocConnection structure as iocAllowedNetworkConf.
  The iocAllowedNetwork names one allowed network, like "iocafenet". Allowed priviliges
  are stored in flags. IOC_AUTH_ADMINISTRATOR bit indicates that administrative
  (configuration and software update) priviliges.
****************************************************************************************************
*/
typedef struct iocAllowedNetwork
{
    /** Network name. Empty string = any network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Flags (priviliges, etc)
     */
    os_ushort flags;
}
iocAllowedNetwork;

typedef struct iocAllowedNetworkConf
{
    iocAllowedNetwork *network;
    os_int n_networs;
}
iocAllowedNetworkConf;


#endif

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

/* Process received athentication data frame.
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
   accessed trough the connection and priviliges for each network. Must be released
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

/* Release allowed networks structure set up by ioc_authorize_user_func()
 */
void ioc_release_allowed_networks(
    iocAllowedNetworkConf *allowed_networks);

/*@}*/

#endif
#endif
