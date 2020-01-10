/**

  @file    ioc_basic_server.h
  @brief   Structures and functions to implement basic server.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  The basic server helpers functions and structures here wrap together bunch of IOCOM structures
  and API calls which are needed by typical basic server, much like ioc_ioboard does for IO boards.
  This layer is optional and written only for convinience.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_SERVER_EXTENSIONS

void ioc_initialize_bserver_main(
    iocBServerMain *m,
    iocRoot *root,
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name)
{
    os_memclear(m, sizeof(iocBServerMain));

    m->root = root;
    os_strncpy(m->device_name, device_name, IOC_NAME_SZ);
    m->device_nr = device_nr;
    os_strncpy(m->network_name, network_name, IOC_NETWORK_NAME_SZ);
}

void ioc_release_bserver_main(
    iocBServerMain *m)
{
    ioc_release_memory_block(&m->exp);
    ioc_release_memory_block(&m->imp);
    ioc_release_memory_block(&m->conf_exp);
    ioc_release_memory_block(&m->conf_imp);
    ioc_release_memory_block(&m->info);
}


/* Set up memory blocks and signals.
 */
void ioc_setup_bserver_mblks(
    iocBServerMain *m,
    iocMblkSignalHdr *signals_exp_hdr,
    iocMblkSignalHdr *signals_imp_hdr,
    iocMblkSignalHdr *signals_conf_exp_hdr,
    iocMblkSignalHdr *signals_conf_imp_hdr,
    const os_char *signal_config,
    os_memsz signal_config_sz,
    const os_char *network_defaults,
    os_memsz network_defaults_sz)
{
    iocMemoryBlockParams blockprm;

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = m->device_name;
    blockprm.device_nr = m->device_nr;
    blockprm.network_name = m->network_name;

    blockprm.mblk_name = signals_exp_hdr->mblk_name;
    blockprm.nbytes = signals_exp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->exp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = signals_imp_hdr->mblk_name;
    blockprm.nbytes = signals_imp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->imp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = signals_conf_exp_hdr->mblk_name;
    blockprm.nbytes = signals_conf_exp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->conf_exp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = signals_conf_imp_hdr->mblk_name;
    blockprm.nbytes = signals_conf_imp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->conf_imp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = "info";
    blockprm.buf = (char*)signal_config;
    blockprm.nbytes = signal_config_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&m->info, OS_NULL, m->root, &blockprm);

    /* Store memory block handle pointer for signals within the "signals" structure.
     */
    ioc_set_handle_to_signals(signals_exp_hdr, &m->exp);
    ioc_set_handle_to_signals(signals_imp_hdr, &m->imp);
    ioc_set_handle_to_signals(signals_conf_exp_hdr, &m->conf_exp);
    ioc_set_handle_to_signals(signals_conf_imp_hdr, &m->conf_imp);

    m->ctrl_stream_params.default_config = network_defaults;
    m->ctrl_stream_params.default_config_sz = network_defaults_sz;
}

void ioc_initialize_bserver_accounts(
    iocBServerAccounts *a,
    iocRoot *root,
    const os_char *network_name)
{
    os_memclear(a, sizeof(iocBServerAccounts));
    a->root = root;
    os_strncpy(a->network_name, network_name, IOC_NETWORK_NAME_SZ);
}

void ioc_release_bserver_accounts(
    iocBServerAccounts *a)
{
    ioc_release_memory_block(&a->accounts_exp);
    ioc_release_memory_block(&a->accounts_imp);
    ioc_release_memory_block(&a->accounts_info);
}


void ioc_setup_bserver_accounts(
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

    blockprm.mblk_name = "info";
    blockprm.buf = (os_char*)account_config;
    blockprm.nbytes = account_config_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&a->accounts_info, OS_NULL, a->root, &blockprm);

    ioc_set_handle_to_signals(accounts_conf_imp_hdr, &a->accounts_imp);
    ioc_set_handle_to_signals(accounts_conf_exp_hdr, &a->accounts_exp);

    /* Load user account configuration from persistent storage or
       use static defaults.
     */
    ioc_load_account_config(&a->account_conf, account_defaults,
        account_defaults_sz);

    a->accounts_stream_params.default_config = account_defaults;
    a->accounts_stream_params.default_config_sz = account_defaults_sz;
}


void ioc_run_bserver_main(
    iocBServerMain *m)
{
    ioc_run_control_stream(&m->ctrl_stream, &m->ctrl_stream_params);
}

void ioc_run_bserver_accounts(
    iocBServerAccounts *a)
{
    ioc_run_control_stream(&a->accounts_stream, &a->accounts_stream_params);
}


#endif
