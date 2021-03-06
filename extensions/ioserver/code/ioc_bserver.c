/**

  @file    ioc_bserver.c
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
#define BSERVER_INTERNALS
#include "ioserver.h"

/* Prototypes for forward referred static functions.
 */
static void ioc_setup_bserver_mblks(
    iocBServer *m,
    iocBServerParams *prm);

static void ioc_setup_bserver_network(
    iocBServerNetwork *n,
    iocBServer *m,
    os_int select,
    const os_char *network_name);

static void ioc_release_bserver_network(
    iocBServerNetwork *n);

static osalStatus ioc_run_bserver_network(
    iocBServerNetwork *n,
    iocBServer *m);


/**
****************************************************************************************************

  @brief Initialize basic server components.

  The ioc_initialize_ioserver() function sets up basic server main structure. The structure holds
  static information for transferring configuration, published networks, etc.

  Flat basic server structure is allocated by application, but this function allocates additional
  memory for published networks. Thus it is important that the structure initialized by this
  function is released by calling ioc_release_bserver().

  Note: If server application has other memory blocks, set these up before initializing
  the basic server components.

  @param   m Pointer to basic server structure to initialize.
  @param   root Pointer to iocom root structure.
  @param   prm Parameters for basic server.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_ioserver(
    iocBServer *m,
    iocRoot *root,
    iocBServerParams *prm)
{
    os_memclear(m, sizeof(iocBServer));

    m->root = root;
    os_strncpy(m->device_name, prm->device_name, IOC_NAME_SZ);
    m->device_nr = prm->device_nr;
    os_strncpy(m->network_name, prm->network_name, IOC_NETWORK_NAME_SZ);
    m->account_defaults = prm->account_defaults;
    m->account_defaults_sz = prm->account_defaults_sz;
    m->is_bypass_server = prm->is_bypass_server;
    m->is_cloud_server = prm->is_cloud_server;
    os_get_timer(&m->sec_timer);

    ioc_setup_bserver_mblks(m, prm);
}


/**
****************************************************************************************************

  @brief Release basic server components.

  The ioc_release_bserver() function releases memory allocated for basic server functionality
  and detchased handles from iocom.

  @param   m Pointer to basic server structure to release.
  @return  None.

****************************************************************************************************
*/
void ioc_release_bserver(
    iocBServer *m)
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


/**
****************************************************************************************************

  @brief Keep basic server functionality alive.

  The ioc_run_bserver() function needs to be called repeatedly to keep basic server
  functionality responsive.

  @param   m Pointer to basic server structure.
  @param   ti Timner value now, OS_NULL to get the timer by function.
  @return  If working in something, the function returns OSAL_SUCCESS. Return value
           OSAL_NOTHING_TO_DO indicates that this thread can be switched to slow
           idle mode as far as the bserver knows.

****************************************************************************************************
*/
osalStatus ioc_run_bserver(
    iocBServer *m,
    os_timer *ti)
{
    os_timer tnow;
    os_int i;
    osalStatus s;

    if (ti == OS_NULL) {
        os_get_timer(&tnow);
        ti = &tnow;
    }

    s = ioc_run_control_stream(&m->ctrl_stream, &m->ctrl_stream_params);

    /* Run security about twice per second and move set flag check_cert_chain_etc from root
       to bserver.
     */
    if (os_has_elapsed_since(&m->sec_timer, ti, 522))
    {
        m->sec_timer = *ti;
        ioc_run_security(m);

        /* This is used by ioc_upload_cert_chain_or_flash_prog() to trigger check for missing
           IO device certificate chain (new device installation), etc.
         */
        if (!m->check_cert_chain_etc && m->root->check_cert_chain_etc)
        {
            m->root->check_cert_chain_etc = OS_FALSE;
            m->check_cert_chain_etc = OS_TRUE;
        }
    }

    for (i = 0; i<m->nro_networks; i++)
    {
        if (ioc_run_bserver_network(m->networks + i, m) != OSAL_NOTHING_TO_DO)
        {
            s = OSAL_SUCCESS;
        }
    }

#if IOC_DYNAMIC_MBLK_CODE
    /* Handle uploading certificate chain to IO device, and optionally (in future?) updating
       flash program version.
     */
    ioc_upload_cert_chain_or_flash_prog(m);
#endif

    return s;
}


/**
****************************************************************************************************

  @brief Set up memory blocks and signals.

  The ioc_setup_bserver_mblks() function sets memory blocks:
  - conf_exp and cond_imp memory blocks are used to transfer network configuration data as stream.
  - info memory block blublishes information about server's memory blocks.

  Note: If server application has other memory blocks, set these up before initializing
  the basic server components.

  @param   m Pointer to basic server structure.
  @param   prm Parameters for basic server.
  @return  None.

****************************************************************************************************
*/
static void ioc_setup_bserver_mblks(
    iocBServer *m,
    iocBServerParams *prm)
{
    iocMemoryBlockParams blockprm;

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    blockprm.device_name = m->device_name;
    blockprm.device_nr = m->device_nr;
    blockprm.network_name = m->network_name;
#endif

    blockprm.mblk_name = prm->signals_exp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_exp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_FLOOR;
    ioc_initialize_memory_block(&m->exp, OS_NULL, m->root, &blockprm);
    ioc_mblk_set_signal_header(&m->exp, prm->signals_exp_hdr);
    /* m->exp.mblk->signal_hdr = exp_mblk_signal_hdr; COULD BE */

    blockprm.mblk_name = prm->signals_imp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_imp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_FLOOR;
    ioc_initialize_memory_block(&m->imp, OS_NULL, m->root, &blockprm);
    ioc_mblk_set_signal_header(&m->imp, prm->signals_imp_hdr);

    blockprm.mblk_name = prm->signals_conf_exp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_conf_exp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_FLOOR;
    ioc_initialize_memory_block(&m->conf_exp, OS_NULL, m->root, &blockprm);
    ioc_mblk_set_signal_header(&m->conf_exp, prm->signals_conf_exp_hdr);

    blockprm.mblk_name = prm->signals_conf_imp_hdr->mblk_name;
    blockprm.nbytes = prm->signals_conf_imp_hdr->mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_FLOOR;
    ioc_initialize_memory_block(&m->conf_imp, OS_NULL, m->root, &blockprm);
    ioc_mblk_set_signal_header(&m->conf_imp, prm->signals_conf_imp_hdr);

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


/**
****************************************************************************************************

  @brief Publish IO device networks.

  The ioc_publish_bserver_networks() function published IO device network "hosted" by this IO
  server. Here publishing means making user account accessible.

  @param   m Pointer to basic server structure.
  @param   publish List containing network to publish, comma is separator.
  @return  If successful, the function returns OSAL_SUCCESS. Other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_publish_bserver_networks(
    iocBServer *m,
    const os_char *publish)
{
    os_char network_name[IOC_NETWORK_NAME_SZ];
    const os_char *p;
    iocBServerNetwork *n;
    os_int nro_networks, select;
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
    select = OS_PBNR_ACCOUNTS_1;
    while (!osal_str_list_iter(network_name, sizeof(network_name), &p, OSAL_STR_NEXT_ITEM))
    {
        /* Setup signal structure structure for user accounts.
         */
        account_signals_init_signal_struct(&n->asignals);

        if (select == OS_PBNR_ACCOUNTS_1 + OS_PB_MAX_NETWORKS)
        {
            osal_debug_error("ioc_bserver: too many published networks");
        }

        ioc_setup_bserver_network(n, m, select++, network_name);

        /* Set up control stream for user accounts.
         */
        n->accounts_stream_params.is_device = OS_TRUE;
        n->accounts_stream_params.frd.cmd = &n->asignals.conf_imp.frd_cmd;
        n->accounts_stream_params.frd.select = &n->asignals.conf_imp.frd_select;
        n->accounts_stream_params.frd.buf = &n->asignals.conf_exp.frd_buf;
        n->accounts_stream_params.frd.head = &n->asignals.conf_exp.frd_head;
        n->accounts_stream_params.frd.tail = &n->asignals.conf_imp.frd_tail;
        n->accounts_stream_params.frd.state = &n->asignals.conf_exp.frd_state;
        n->accounts_stream_params.frd.err = &n->asignals.conf_exp.frd_err;
        n->accounts_stream_params.frd.cs = &n->asignals.conf_exp.frd_cs;
        n->accounts_stream_params.frd.to_device = OS_FALSE;
        n->accounts_stream_params.tod.cmd = &n->asignals.conf_imp.tod_cmd;
        n->accounts_stream_params.tod.select = &n->asignals.conf_imp.tod_select;
        n->accounts_stream_params.tod.buf = &n->asignals.conf_imp.tod_buf;
        n->accounts_stream_params.tod.head = &n->asignals.conf_imp.tod_head;
        n->accounts_stream_params.tod.tail = &n->asignals.conf_exp.tod_tail;
        n->accounts_stream_params.tod.err =&n->asignals.conf_exp.tod_err;
        n->accounts_stream_params.tod.cs =&n->asignals.conf_imp.tod_cs;
        n->accounts_stream_params.tod.state = &n->asignals.conf_exp.tod_state;
        n->accounts_stream_params.tod.to_device = OS_TRUE;
        ioc_init_control_stream(&n->accounts_stream, &n->accounts_stream_params);

        n++;
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Set up memory blocks, etc. to published IO network.

  The ioc_setup_bserver_network() function sets up IO network structure and memory blocks
  for published used accounts.

  @param   n Pointer to IO network structure.
  @param   m Pointer to basic server structure.
  @param   select Persistent block number (on flash or in file system) where the user account
           configuration is stored.
  @param   network_name Network name for the published IO network.
  @return  None.

****************************************************************************************************
*/
static void ioc_setup_bserver_network(
    iocBServerNetwork *n,
    iocBServer *m,
    os_int select,
    const os_char *network_name)
{
    iocMemoryBlockParams blockprm;
    const os_uchar *account_defaults;
    os_memsz account_defaults_sz;

    os_strncpy(n->network_name, network_name, IOC_NETWORK_NAME_SZ);
    n->select = select;

    /* Generate memory blocks.
       Note: Device number for accounts is calculated from persistent block number.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = ioc_accounts_device_name;
    blockprm.device_nr = select - OS_PBNR_ACCOUNTS_1 + 1;
    blockprm.network_name = n->network_name;

    blockprm.mblk_name = n->asignals.exp.hdr.mblk_name;
    blockprm.nbytes = n->asignals.exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_NO_CLOUD|IOC_FLOOR;
    ioc_initialize_memory_block(&n->accounts_exp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = n->asignals.conf_exp.hdr.mblk_name;
    blockprm.nbytes = n->asignals.conf_exp.hdr.mblk_sz;
    blockprm.flags = m->is_cloud_server
        ? IOC_MBLK_UP|IOC_NO_CLOUD|IOC_FLOOR
        : IOC_MBLK_UP|IOC_FLOOR;
    ioc_initialize_memory_block(&n->accounts_conf_exp, OS_NULL, m->root, &blockprm);

    blockprm.mblk_name = n->asignals.conf_imp.hdr.mblk_name;
    blockprm.nbytes = n->asignals.conf_imp.hdr.mblk_sz;
    blockprm.flags = m->is_cloud_server
        ? IOC_MBLK_DOWN|IOC_NO_CLOUD|IOC_FLOOR
        : IOC_MBLK_DOWN|IOC_FLOOR;
    ioc_initialize_memory_block(&n->accounts_conf_imp, OS_NULL, m->root, &blockprm);

    account_defaults = m->account_defaults;
    account_defaults_sz = m->account_defaults_sz;
    if (account_defaults == OS_NULL)
    {
        account_defaults = ioapp_server_accounts;
        account_defaults_sz  = sizeof(ioapp_server_accounts);
    }

    /* Load user account configuration from persistent storage and publish it
     * as data memory block.
     */
    blockprm.mblk_name = "data";
    blockprm.flags = (m->is_bypass_server || m->is_cloud_server)
        ? IOC_MBLK_DOWN|IOC_ALLOW_RESIZE|IOC_CLOUD_ONLY|IOC_NO_CLOUD|IOC_FLOOR
        : IOC_MBLK_DOWN|IOC_ALLOW_RESIZE|IOC_CLOUD_ONLY;
    blockprm.nbytes = 0;
    ioc_initialize_memory_block(&n->accounts_data, OS_NULL, m->root, &blockprm);
    ioc_load_persistent_into_mblk(&n->accounts_data, select, account_defaults,
        account_defaults_sz);

    blockprm.mblk_name = "info";
    blockprm.buf = (os_char*)ioapp_server_signals_config;
    blockprm.nbytes = sizeof(ioapp_server_signals_config);
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&n->accounts_info, OS_NULL, m->root, &blockprm);

    ioc_set_handle_to_signals(&n->asignals.exp.hdr, &n->accounts_exp);
    ioc_set_handle_to_signals(&n->asignals.conf_imp.hdr, &n->accounts_conf_imp);
    ioc_set_handle_to_signals(&n->asignals.conf_exp.hdr, &n->accounts_conf_exp);

    n->accounts_stream_params.default_config = account_defaults;
    n->accounts_stream_params.default_config_sz = account_defaults_sz;
}


/**
****************************************************************************************************

  @brief Release resources allocated for published network (internal).

  The ioc_release_bserver_network() function releases iocom memory blocks used for
  published account information.

  @param   n Pointer to published network structure.
  @return  None.

****************************************************************************************************
*/
static void ioc_release_bserver_network(
    iocBServerNetwork *n)
{
    ioc_release_memory_block(&n->accounts_conf_exp);
    ioc_release_memory_block(&n->accounts_conf_imp);
    ioc_release_memory_block(&n->accounts_data);
    ioc_release_memory_block(&n->accounts_info);
}


/**
****************************************************************************************************

  @brief Keep account configuration responsive (internal).

  The ioc_run_bserver_network() function needs to be called repeatedly to keep published
  network account configuration responsive.

  @param   n Pointer to published network structure.
  @return  If working in something, the function returns OSAL_SUCCESS. Return value
           OSAL_NOTHING_TO_DO indicates that this thread can be switched to slow
           idle mode as far as the bserver knows.

****************************************************************************************************
*/
static osalStatus ioc_run_bserver_network(
    iocBServerNetwork *n,
    iocBServer *m)
{
    osalStatus s;

    s = ioc_run_control_stream(&n->accounts_stream, &n->accounts_stream_params);

    /* If running control stream returns information that the user account information has
       been changed, reload it.
     */
    if (s == OSAL_SUCCESS || s == OSAL_NOTHING_TO_DO)
    {
        if (n->accounts_stream.transfer_status == IOC_BLOCK_WRITTEN &&
            n->accounts_stream.transferred_block_nr == n->select)
        {
            ioc_load_persistent_into_mblk(&n->accounts_data, n->select, m->account_defaults,
                m->account_defaults_sz);
        }
    }

    return s;
}
