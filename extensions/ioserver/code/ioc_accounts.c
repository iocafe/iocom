/**

  @file    ioc_accounts.c
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
#include "ioserver.h"
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION

#if 0

/* Load user account configuration from persistent storage.
 */
void ioc_publish_io_networks(
    iocAccountConf *accountconf,
    iocRoot *root,
    const os_char *publish_list,
    const os_char *default_config,
    os_memsz default_config_sz)
{


}

void xxxxxxxxxxxxxxxxxxioc_setup_xbserver_accounts(
    iocBServerAccounts *a,
    iocMblkSignalHdr *accounts_conf_exp_hdr,
    iocMblkSignalHdr *accounts_conf_imp_hdr,
    const os_char *account_config,
    os_memsz account_config_sz,
    const os_char *account_defaults,
    os_memsz account_defaults_sz)
{
    iocMemoryBlockParams blockprm;
    const os_char *accounts_device_name = "accounts";
    const os_int accounts_device_nr = 1;

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = accounts_device_name;
    blockprm.device_nr = accounts_device_nr;
    blockprm.network_name = a->network_name;

    blockprm.mblk_name = accounts_conf_exp_hdr->mblk_name;
    blockprm.nbytes = accounts_conf_exp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&a->accounts_exp, OS_NULL, a->root, &blockprm);

    blockprm.mblk_name = accounts_conf_imp_hdr->mblk_name;
    blockprm.nbytes = accounts_conf_imp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&a->accounts_imp, OS_NULL, a->root, &blockprm);
}


void ioc_load_account_config(
    iocAccountConf *accounts,
    iocRoot *root,
    const os_char *default_config,
    os_memsz default_config_sz)
{

}

void ioc_release_account_config(
    iocAccountConf *accounts)
{

}
#endif


osalStatus ioc_authenticate(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUserAccount *user_account,
    void *context)
{
    return OSAL_SUCCESS;
    // return OSAL_STATUS_FAILED;
}


#endif
