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


typedef struct iocBServerMain
{
    /* Pointer to IOCOM root structure.
     */
    iocRoot *root;

    /* Identification of this IO network node.
     */
    os_char device_name[IOC_NAME_SZ];
    os_int device_nr;
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /* Memory block handles for the server.
     */
    iocHandle exp, imp, conf_exp, conf_imp, info;

    /* Control stream to configure the IO node.
     */
    iocStreamerParams ctrl_stream_params;
    iocControlStreamState ctrl_stream;
}
iocBServerMain;


typedef struct iocBServerAccounts
{
    /* Pointer to IOCOM root structure.
     */
    iocRoot *root;

    /* Network name, accounts are for this IO network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /* Memory block handles for accounts.
     */
    iocHandle accounts_exp, accounts_imp, accounts_data, accounts_info;

    /* Control stream to configure the device/user accounts.
     */
    iocStreamerParams accounts_stream_params;
    iocControlStreamState accounts_stream;

    /* Current device/user account configuration.
     */
    // iocAccountConf account_conf;


}
iocBServerAccounts;


void ioc_initialize_bserver_main(
    iocBServerMain *m,
    iocRoot *root,
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name);

void ioc_release_bserver_main(
    iocBServerMain *m);

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
    os_memsz network_defaults_sz);

void ioc_run_bserver_main(
    iocBServerMain *m);

void ioc_initialize_bserver_accounts(
    iocBServerAccounts *a,
    iocRoot *root,
    const os_char *network_name);

void ioc_release_bserver_accounts(
    iocBServerAccounts *a);

void ioc_setup_bserver_accounts(
    iocBServerAccounts *a,
    iocMblkSignalHdr *accounts_conf_exp_hdr,
    iocMblkSignalHdr *accounts_conf_imp_hdr,
    const os_char *account_config,
    os_memsz account_config_sz,
    const os_char *account_defaults,
    os_memsz account_defaults_sz);

void ioc_run_bserver_accounts(
    iocBServerAccounts *a);

#define IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(bmain, sig) \
    bmain.ctrl_stream_params.is_device = OS_TRUE; \
    bmain.ctrl_stream_params.frd.cmd = &sig.conf_imp.frd_cmd; \
    bmain.ctrl_stream_params.frd.select = &sig.conf_imp.frd_select; \
    bmain.ctrl_stream_params.frd.buf = &sig.conf_exp.frd_buf; \
    bmain.ctrl_stream_params.frd.head = &sig.conf_exp.frd_head; \
    bmain.ctrl_stream_params.frd.tail = &sig.conf_imp.frd_tail; \
    bmain.ctrl_stream_params.frd.state = &sig.conf_exp.frd_state; \
    bmain.ctrl_stream_params.frd.to_device = OS_FALSE; \
    bmain.ctrl_stream_params.tod.cmd = &sig.conf_imp.tod_cmd; \
    bmain.ctrl_stream_params.tod.select = &sig.conf_imp.tod_select; \
    bmain.ctrl_stream_params.tod.buf = &sig.conf_imp.tod_buf; \
    bmain.ctrl_stream_params.tod.head = &sig.conf_imp.tod_head; \
    bmain.ctrl_stream_params.tod.tail = &sig.conf_exp.tod_tail; \
    bmain.ctrl_stream_params.tod.state = &sig.conf_exp.tod_state; \
    bmain.ctrl_stream_params.tod.to_device = OS_TRUE; \
    ioc_init_control_stream(&bmain.ctrl_stream, &bmain.ctrl_stream_params); \

#define IOC_SETUP_BSERVER_ACCOUNTS_STREAM_MACRO(baccts, accts) \
    baccts.accounts_stream_params.is_device = OS_TRUE; \
    baccts.accounts_stream_params.frd.cmd = &accts.conf_imp.frd_cmd; \
    baccts.accounts_stream_params.frd.select = &accts.conf_imp.frd_select; \
    baccts.accounts_stream_params.frd.buf = &accts.conf_exp.frd_buf; \
    baccts.accounts_stream_params.frd.head = &accts.conf_exp.frd_head; \
    baccts.accounts_stream_params.frd.tail = &accts.conf_imp.frd_tail; \
    baccts.accounts_stream_params.frd.state = &accts.conf_exp.frd_state; \
    baccts.accounts_stream_params.frd.to_device = OS_FALSE; \
    baccts.accounts_stream_params.tod.cmd = &accts.conf_imp.tod_cmd; \
    baccts.accounts_stream_params.tod.select = &accts.conf_imp.tod_select; \
    baccts.accounts_stream_params.tod.buf = &accts.conf_imp.tod_buf; \
    baccts.accounts_stream_params.tod.head = &accts.conf_imp.tod_head; \
    baccts.accounts_stream_params.tod.tail = &accts.conf_exp.tod_tail; \
    baccts.accounts_stream_params.tod.state = &accts.conf_exp.tod_state; \
    baccts.accounts_stream_params.tod.to_device = OS_TRUE; \
    ioc_init_control_stream(&baccts.accounts_stream, &baccts.accounts_stream_params); \

