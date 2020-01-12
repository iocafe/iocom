/**

  @file    ioc_authentication.h
  @brief   Device/user authentication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

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
  Flags in authentication frame.
****************************************************************************************************
*/
#define IOC_AUTH_ADMINISTRATOR 1
#define IOC_AUTH_CONNECT_UP 4
#define IOC_AUTH_DEVICE_NR_2_BYTES 8
#define IOC_AUTH_DEVICE_NR_4_BYTES 16
#define IOC_AUTH_BIDIRECTIONAL_COM 128

#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION

/**
****************************************************************************************************
  User account.
****************************************************************************************************
*/
typedef struct iocUserAccount
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
iocUserAccount;


/**
****************************************************************************************************
  Networks allowed tough a specific connection.
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
typedef osalStatus ioc_authenticate_user_func(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUserAccount *user_account,
    void *context);

/* Release allowed networks structure set up by ioc_authenticate_user_func()
 */
void ioc_release_allowed_networks(
    iocAllowedNetworkConf *allowed_networks);

/* Enable user authentication.
 */
typedef osalStatus ioc_enable_user_authentication(
    struct iocRoot *root,
    ioc_authenticate_user_func *func,
    void *context);

/*@}*/

#endif
#endif
