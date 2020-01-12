/**

  @file    ioc_accounts.h
  @brief   User/device accounts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION

typedef struct iocAccountConf
{

    iocUserAccount account[1];
}
iocAccountConf;

/**
****************************************************************************************************
  Device/user account functions
****************************************************************************************************
 */

/* Load user account configuration from persistent storage.
 */
void ioc_load_account_config(
    iocAccountConf *accounts,
    iocRoot *root,
    const os_char *default_config,
    os_memsz default_config_sz);

#if 0
/* Add block of user account data. Remember lock. Must not change when locked, call unlink */
void ioc_link_account_conf_struct(
    iocAllowedNetworkConf *allowed_networks);

/* Add block of user account data */
void ioc_unlink_account_conf(
    iocAllowedNetworkConf *?);
#endif


/* Default authentication and authorization function.
 */
osalStatus ioc_authenticate(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUserAccount *user_account,
    void *context);

#endif
