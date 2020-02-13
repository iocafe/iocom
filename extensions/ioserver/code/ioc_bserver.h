/**

  @file    ioc_bserver.h
  @brief   Structures and functions to implement basic server.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2020

  The basic server helpers functions and structures here wrap together bunch of IOCOM structures
  and API calls which are needed by typical basic server, much like ioc_ioboard does for IO boards.
  This layer is optional and written only for convinience.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

#ifdef BSERVER_INTERNALS
#include "config/include/account-signals.h"
#include "config/include/account-defaults.h"
#include "config/include/accounts-mblk-binary.h"
#endif

struct iocBServerNetwork;
struct iocPersistentWriter;

/**
****************************************************************************************************
  Basic server parameters
****************************************************************************************************
 */
typedef struct iocBServerParams
{
    const os_char *device_name;
    os_int device_nr;
    const os_char *network_name;

    os_boolean is_bypass_server;
    os_boolean is_cloud_server;

    iocMblkSignalHdr *signals_exp_hdr;
    iocMblkSignalHdr *signals_imp_hdr;
    iocMblkSignalHdr *signals_conf_exp_hdr;
    iocMblkSignalHdr *signals_conf_imp_hdr;
    const os_char *signal_config;
    os_memsz signal_config_sz;
    const os_char *network_defaults;
    os_memsz network_defaults_sz;
    const os_char *account_defaults;
    os_memsz account_defaults_sz;
}
iocBServerParams;


/**
****************************************************************************************************
  Basic server state structure
****************************************************************************************************
 */
typedef struct iocBServer
{
    /* Pointer to IOCOM root structure.
     */
    iocRoot *root;

    /* Identification of this IO network node.
     */
    os_char device_name[IOC_NAME_SZ];
    os_int device_nr;
    os_char network_name[IOC_NETWORK_NAME_SZ];

    os_boolean is_bypass_server;
    os_boolean is_cloud_server;

    /* Check for missing certificate chain and flash program versions. This is optimization flag
       for automatic uploader.
     */
    os_boolean check_cert_chain_etc;

    /* Memory block handles for the server.
     */
    iocHandle exp, imp, conf_exp, conf_imp, info;

    /* Control stream to configure the IO node.
     */
    iocStreamerParams ctrl_stream_params;
    iocControlStreamState ctrl_stream;

    /* Data for published networks.
     */
    struct iocBServerNetwork *networks;
    os_int nro_networks;

    /* Saved pointers from parameters.
     */
    const os_char *account_defaults;
    os_memsz account_defaults_sz;

    /** Pointer to persistent writer object, if currently uploading client certificate
        or automatically updating IO device's flash program.
     */
    struct iocPersistentWriter *persistent_writer;

    /** Security run timer.
     */
    os_timer sec_timer;
}
iocBServer;


#ifdef BSERVER_INTERNALS
/* Internal basic server state structure to publish account information of an IO network.
 */
typedef struct iocBServerNetwork
{
    /* IO device/user account IO definition structure.
     */
    account_signals_t asignals;

    /* Network name, accounts are for this IO network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /* Persistent block number in which account configuration is saved.
     */
    os_short select;

    /* Memory block handles for accounts.
     */
    iocHandle accounts_exp, accounts_conf_exp, accounts_conf_imp, accounts_data, accounts_info;

    /* Control stream to configure the device/user accounts.
     */
    iocStreamerParams accounts_stream_params;
    iocControlStreamState accounts_stream;

    /* Security status.
     */
    iocSecurityStatus sec_status;
}
iocBServerNetwork;
#endif


/**
****************************************************************************************************
  Basic server functions and macros
****************************************************************************************************
 */
/* Initialize basic server components.
 */
void ioc_initialize_bserver(
    iocBServer *m,
    iocRoot *root,
    iocBServerParams *prm);

/* Release basic server components.
 */
void ioc_release_bserver(
    iocBServer *m);

/* Publish IO device networks.
 */
osalStatus ioc_publish_bserver_networks(
    iocBServer *m,
    const os_char *publish);

/* Keep basic server functionality alive.
 */
osalStatus ioc_run_bserver(
    iocBServer *m);

/* Macro to set up a control stream by typical signal configuration.
 */
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
    ioc_init_control_stream(&bmain.ctrl_stream, &bmain.ctrl_stream_params);

