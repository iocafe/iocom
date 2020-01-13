/**

  @file    ioc_basic_server.h
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
#include "ioserver.h"
#include "account-signals.h"
#include "account-defaults.h"
#include "accounts-mblk-binary.h"


typedef struct iocBServerNetwork
{
    /* IO device/user account IO definition structure.
     */
    account_signals_t asignals;

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
}
iocBServerNetwork;


/* Prototypes for forward referred static functions.
 */
static void ioc_setup_bserver_mblks(
    iocBServerMain *m,
    iocBServerParams *prm);

static void ioc_setup_bserver_network(
    iocBServerNetwork *n,
    iocRoot *root,
    const os_char *network_name);

static void ioc_release_bserver_network(
    iocBServerNetwork *n);

static void ioc_run_bserver_network(
    iocBServerNetwork *n);


void ioc_initialize_bserver(
    iocBServerMain *m,
    iocRoot *root,
    iocBServerParams *prm)
{
    os_memclear(m, sizeof(iocBServerMain));

    m->root = root;
    os_strncpy(m->device_name, prm->device_name, IOC_NAME_SZ);
    m->device_nr = prm->device_nr;
    os_strncpy(m->network_name, prm->network_name, IOC_NETWORK_NAME_SZ);

    ioc_setup_bserver_mblks(m, prm);
}


void ioc_release_bserver(
    iocBServerMain *m)
{
    os_int i;

    if (m->networks)
    {
        for (i = 0; i<m->nro_networks; i++)
        {
            ioc_release_bserver_network(m->networks + i);
        }

        os_free(m->networks, sizeof(iocBServerNetwork) * m->nro_networks);
    }

    ioc_release_memory_block(&m->exp);
    ioc_release_memory_block(&m->imp);
    ioc_release_memory_block(&m->conf_exp);
    ioc_release_memory_block(&m->conf_imp);
    ioc_release_memory_block(&m->info);
}


void ioc_run_bserver_main(
    iocBServerMain *m)
{
    os_int i;

    ioc_run_control_stream(&m->ctrl_stream, &m->ctrl_stream_params);

    for (i = 0; i<m->nro_networks; i++)
    {
        ioc_run_bserver_network(m->networks + i);
    }
}


/* Set up memory blocks and signals.
 */
static void ioc_setup_bserver_mblks(
    iocBServerMain *m,
    iocBServerParams *prm)
{
    iocMemoryBlockParams blockprm;

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = m->device_name;
    blockprm.device_nr = m->device_nr;
    blockprm.network_name = m->network_name;

    blockprm.mblk_name = prm->signals_exp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_exp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->exp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = prm->signals_imp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_imp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->imp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = prm->signals_conf_exp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_conf_exp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->conf_exp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = prm->signals_conf_imp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_conf_imp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m->conf_imp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = "info";
    blockprm.buf = (char*)prm->signal_config;
    blockprm.nbytes = (os_int)prm->signal_config_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&m->info, OS_NULL, m->root, &blockprm);

    /* Store memory block handle pointer for signals within the "signals" structure.
     */
    ioc_set_handle_to_signals(prm->signals_exp_hdr, &m->exp);
    ioc_set_handle_to_signals(prm->signals_imp_hdr, &m->imp);
    ioc_set_handle_to_signals(prm->signals_conf_exp_hdr, &m->conf_exp);
    ioc_set_handle_to_signals(prm->signals_conf_imp_hdr, &m->conf_imp);

    m->ctrl_stream_params.default_config = prm->network_defaults;
    m->ctrl_stream_params.default_config_sz = prm->network_defaults_sz;
}

osalStatus ioc_publish_bserver_networks(
    iocBServerMain *m,
    const os_char *publish)
{
    os_char network_name[IOC_NETWORK_NAME_SZ];
    const os_char *p;
    iocBServerNetwork *n;
    os_int nro_networks;
    os_memsz sz;

    /* Count number of networks to publish (nro_networks)
     */
    p = publish;
    nro_networks = 0;
    while (!osal_str_list_iter(network_name, sizeof(network_name), &p, OSAL_STR_NEXT_ITEM))
    {
        nro_networks++;
    }
    if (nro_networks == 0)
    {
        osal_debug_error("bserver: no networks to publish");
        return OSAL_STATUS_FAILED;
    }

    /* Allocate memory for networks.
     */
    sz = sizeof(iocBServerNetwork) * nro_networks;
    n = (iocBServerNetwork*)os_malloc(sz, OS_NULL);
    os_memclear(n, sz);
    if (n== OS_NULL) return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    m->networks = n;
    m->nro_networks = nro_networks;

    p = publish;
    while (!osal_str_list_iter(network_name, sizeof(network_name), &p, OSAL_STR_NEXT_ITEM))
    {
        /* Setup signal structure structure for user accounts.
         */
        account_signals_init_signal_struct(&n->asignals);

        ioc_setup_bserver_network(n, m->root, network_name);

        /* Set up control stream for user accounts.
         */
        n->accounts_stream_params.is_device = OS_TRUE;
        n->accounts_stream_params.frd.cmd = &n->asignals.conf_imp.frd_cmd;
        n->accounts_stream_params.frd.select = &n->asignals.conf_imp.frd_select;
        n->accounts_stream_params.frd.buf = &n->asignals.conf_exp.frd_buf;
        n->accounts_stream_params.frd.head = &n->asignals.conf_exp.frd_head;
        n->accounts_stream_params.frd.tail = &n->asignals.conf_imp.frd_tail;
        n->accounts_stream_params.frd.state = &n->asignals.conf_exp.frd_state;
        n->accounts_stream_params.frd.to_device = OS_FALSE;
        n->accounts_stream_params.tod.cmd = &n->asignals.conf_imp.tod_cmd;
        n->accounts_stream_params.tod.select = &n->asignals.conf_imp.tod_select;
        n->accounts_stream_params.tod.buf = &n->asignals.conf_imp.tod_buf;
        n->accounts_stream_params.tod.head = &n->asignals.conf_imp.tod_head;
        n->accounts_stream_params.tod.tail = &n->asignals.conf_exp.tod_tail;
        n->accounts_stream_params.tod.state = &n->asignals.conf_exp.tod_state;
        n->accounts_stream_params.tod.to_device = OS_TRUE;
        ioc_init_control_stream(&n->accounts_stream, &n->accounts_stream_params);

        n++;
    }

    return OSAL_SUCCESS;
}


static void ioc_setup_bserver_network(
    iocBServerNetwork *n,
    iocRoot *root,
    const os_char *network_name)
{
    iocMemoryBlockParams blockprm;
    const os_char *accounts_device_name = "accounts";
    const os_int accounts_device_nr = 1;

    os_strncpy(n->network_name, network_name, IOC_NETWORK_NAME_SZ);

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = accounts_device_name;
    blockprm.device_nr = accounts_device_nr;
    blockprm.network_name = n->network_name;

    blockprm.mblk_name = n->asignals.conf_exp.hdr.mblk_name;
    blockprm.nbytes = n->asignals.conf_exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&n->accounts_exp, OS_NULL, root, &blockprm);

    blockprm.mblk_name = n->asignals.conf_imp.hdr.mblk_name;
    blockprm.nbytes = n->asignals.conf_imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&n->accounts_imp, OS_NULL, root, &blockprm);

    /* Load user account configuration from persistent storage and publish it
     * as data memory block.
     */
    blockprm.mblk_name = "data";
    blockprm.flags = IOC_MBLK_DOWN|IOC_ALLOW_RESIZE|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&n->accounts_data, OS_NULL, root, &blockprm);
    ioc_load_persistent_into_mblk(&n->accounts_data, 4, ioapp_account_defaults,
        sizeof(ioapp_account_defaults));

    blockprm.mblk_name = "info";
    blockprm.buf = (os_char*)ioapp_account_config;
    blockprm.nbytes = sizeof(ioapp_account_config);
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&n->accounts_info, OS_NULL, root, &blockprm);

    ioc_set_handle_to_signals(&n->asignals.conf_imp.hdr, &n->accounts_imp);
    ioc_set_handle_to_signals(&n->asignals.conf_exp.hdr, &n->accounts_exp);

    n->accounts_stream_params.default_config = ioapp_account_defaults;
    n->accounts_stream_params.default_config_sz = sizeof(ioapp_account_defaults);
}


static void ioc_release_bserver_network(
    iocBServerNetwork *n)
{
    ioc_release_memory_block(&n->accounts_exp);
    ioc_release_memory_block(&n->accounts_imp);
    ioc_release_memory_block(&n->accounts_data);
    ioc_release_memory_block(&n->accounts_info);
}


static void ioc_run_bserver_network(
    iocBServerNetwork *n)
{
    ioc_run_control_stream(&n->accounts_stream, &n->accounts_stream_params);
}

