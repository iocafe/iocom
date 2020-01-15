/**

  @file    ioc_authorize.h
  @brief   User/device accounts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION

typedef struct iocAccountConfMblk
{
    iocHandle *handle;
    struct iocAccountConfMblk *next;
}
iocAccountConfMblk;

typedef struct iocAccountConf
{
    iocAccountConfMblk *mblk_list;
}
iocAccountConf;


extern const os_char ioc_accounts_device_name[];
extern const os_int ioc_accounts_device_nr;
extern const os_char ioc_accounts_data_mblk_name[];

/**
****************************************************************************************************
  Device/user account functions
****************************************************************************************************
 */

/* Load user account configuration, generate memory blocks. REMEMBER LOCK
 */
void ioc_publish_io_networks(
    iocAccountConf *accountconf,
    iocRoot *root,
    const os_char *publish_list,
    const os_char *default_config,
    os_memsz default_config_sz);


/* Default authentication and authorization function.
 */
osalStatus ioc_authorize(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUser *user,
    os_char *ip,
    void *context);

#endif
