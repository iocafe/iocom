/**

  @file    frank_main.h
  @brief   Controller example with static IO defice configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.12.2011

  Copyright 2012 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "frank.h"



/**
****************************************************************************************************
  Construct application.
****************************************************************************************************
*/
FrankMain::FrankMain(
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name)
{
    os_int i;

    os_strncpy(m_device_name, device_name, IOC_NAME_SZ);
    m_device_nr = device_nr;
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);

    for (i = 0; i < MAX_APPS; i++)
        m_app[i] = OS_NULL;

    setup_mblks();
    setup_ctrl_stream();
}


/**
****************************************************************************************************
  Application destructor.
****************************************************************************************************
*/
FrankMain::~FrankMain()
{
    os_int i;

    /* Finish with 'frank' applications.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        delete m_app[i];
    }

    release_mblks();
}


/* This needs to move to library
 */
static void doit(iocMblkSignalHdr *mblk_hdr, iocHandle *handle)
{
    iocSignal *sig;
    os_int count;

    mblk_hdr->handle = handle;

    count = mblk_hdr->n_signals;
    sig = mblk_hdr->first_signal;

    while (count--)
    {
        sig->handle = handle;
        sig++;
    }
}


/* Set up memory blocks and signals.
 */
void FrankMain::setup_mblks()
{
    iocMemoryBlockParams blockprm;

    /* Initialize signal structure for this device.
     */
    frank_init_signal_struct(&m_signals);

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = m_device_name;
    blockprm.device_nr = m_device_nr;
    blockprm.network_name = m_network_name;

    blockprm.mblk_name = m_signals.exp.hdr.mblk_name;
    blockprm.nbytes = m_signals.exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m_exp, OS_NULL, &ioapp_root, &blockprm);

    blockprm.mblk_name = m_signals.imp.hdr.mblk_name;
    blockprm.nbytes = m_signals.imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m_imp, OS_NULL, &ioapp_root, &blockprm);

    blockprm.mblk_name = m_signals.conf_exp.hdr.mblk_name;
    blockprm.nbytes = m_signals.conf_exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m_conf_exp, OS_NULL, &ioapp_root, &blockprm);

    blockprm.mblk_name = m_signals.conf_imp.hdr.mblk_name;
    blockprm.nbytes = m_signals.conf_imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC;
    ioc_initialize_memory_block(&m_conf_imp, OS_NULL, &ioapp_root, &blockprm);

    blockprm.mblk_name = "info";
    blockprm.buf = (char*)ioapp_signal_config;
    blockprm.nbytes = sizeof(ioapp_signal_config);
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&m_info, OS_NULL, &ioapp_root, &blockprm);

    /* Store memory block handle pointer for signals within the "signals" structure.
     */
    doit(&m_signals.exp.hdr, &m_exp);
    doit(&m_signals.imp.hdr, &m_imp);
    doit(&m_signals.conf_exp.hdr, &m_conf_exp);
    doit(&m_signals.conf_imp.hdr, &m_conf_imp);
}


void FrankMain::release_mblks()
{
    ioc_release_memory_block(&m_exp);
    ioc_release_memory_block(&m_imp);
    ioc_release_memory_block(&m_conf_exp);
    ioc_release_memory_block(&m_conf_imp);
    ioc_release_memory_block(&m_info);
}


/**
****************************************************************************************************
  Start thread which listens for client connections.
****************************************************************************************************
*/
osalStatus FrankMain::listen_for_clients()
{
    iocEndPoint *ep = OS_NULL;
    iocEndPointParams epprm;

    // const osalStreamInterface *iface = OSAL_SOCKET_IFACE;
    const osalStreamInterface *iface = OSAL_TLS_IFACE;

    ep = ioc_initialize_end_point(OS_NULL, &ioapp_root);
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = iface;
    epprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS; /* Notice IOC_DYNAMIC_MBLKS */
    ioc_listen(ep, &epprm);

    os_sleep(100);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************
  Or start thread which connects to client.
****************************************************************************************************
*/
osalStatus FrankMain::connect_to_device()
{
    iocConnection *con = OS_NULL;
    iocConnectionParams conprm;

    // const osalStreamInterface *iface = OSAL_SOCKET_IFACE;
    const osalStreamInterface *iface = OSAL_TLS_IFACE;

    con = ioc_initialize_connection(OS_NULL, &ioapp_root);
    os_memclear(&conprm, sizeof(conprm));

    conprm.iface = iface;
    conprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS|IOC_BIDIRECTIONAL_MBLKS;
    conprm.parameters = "127.0.0.1";
    ioc_connect(con, &conprm);

    os_sleep(100);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************
  Setup control stream parameters to configure this IO network node.
****************************************************************************************************
*/
void FrankMain::setup_ctrl_stream()
{
    os_memclear(&m_ctrl_stream_params, sizeof(iocStreamerParams));

    m_ctrl_stream_params.is_device = OS_TRUE;

    m_ctrl_stream_params.frd.cmd = &m_signals.conf_imp.frd_cmd;
    m_ctrl_stream_params.frd.select = &m_signals.conf_imp.frd_select;
    m_ctrl_stream_params.frd.buf = &m_signals.conf_exp.frd_buf;
    m_ctrl_stream_params.frd.head = &m_signals.conf_exp.frd_head;
    m_ctrl_stream_params.frd.tail = &m_signals.conf_imp.frd_tail;
    m_ctrl_stream_params.frd.state = &m_signals.conf_exp.frd_state;
    m_ctrl_stream_params.frd.to_device = OS_FALSE;

    m_ctrl_stream_params.tod.cmd = &m_signals.conf_imp.tod_cmd;
    m_ctrl_stream_params.tod.select = &m_signals.conf_imp.tod_select;
    m_ctrl_stream_params.tod.buf = &m_signals.conf_imp.tod_buf;
    m_ctrl_stream_params.tod.head = &m_signals.conf_imp.tod_head;
    m_ctrl_stream_params.tod.tail = &m_signals.conf_exp.tod_tail;
    m_ctrl_stream_params.tod.state = &m_signals.conf_exp.tod_state;
    m_ctrl_stream_params.tod.to_device = OS_TRUE;

    m_ctrl_stream_params.default_config = ioapp_network_defaults;
    m_ctrl_stream_params.default_config_sz = sizeof(ioapp_network_defaults);

    ioc_init_control_stream(&m_ctrl_state, &m_ctrl_stream_params);
}


/**
****************************************************************************************************
  Keep the control stream alive.
****************************************************************************************************
*/
void FrankMain::run()
{
    ioc_run_control_stream(&m_ctrl_state, &m_ctrl_stream_params);
}


/**
****************************************************************************************************
  Launc a client application.
****************************************************************************************************
*/
void FrankMain::launch_app(
    os_char *network_name)
{
    os_int i;

    /* If app is already running for this network.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (m_app[i])
        {
            if (!os_strcmp(network_name, m_app[i]->m_network_name)) return;
        }
    }

    /* Launc app.
     */
    for (i = 0; i < MAX_APPS; i++)
    {
        if (m_app[i] == OS_NULL)
        {
            m_app[i] = new FrankApplication();
            m_app[i]->start(network_name, 1);
            return;
        }
    }

    osal_debug_error("Too many IO networks");
}




void FrankMain::inititalize_accounts(const os_char *network_name)
{
    iocMemoryBlockParams blockprm;
    const os_char *accounts_device_name = "accounts";
    os_int accounts_device_nr = 1;

    /* Setup initial Gina IO board definition structure.
     */
    // gina_init_signal_struct(&m_gina_def);


    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = accounts_device_name;
    blockprm.device_nr = accounts_device_nr;
    blockprm.network_name = network_name;

    blockprm.mblk_name = m_accounts.exp.hdr.mblk_name;
    blockprm.nbytes = m_accounts.exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC /* |IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_accounts_export, OS_NULL, &ioapp_root, &blockprm);

    blockprm.mblk_name = m_accounts.imp.hdr.mblk_name;
    blockprm.nbytes = m_accounts.imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC /* |IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_accounts_import, OS_NULL, &ioapp_root, &blockprm);

    doit(&m_accounts.imp.hdr, &m_accounts_import);
    doit(&m_accounts.exp.hdr, &m_accounts_export);

    /* Set callback to detect received data and connection status changes.
     */
    // ioc_add_callback(&ctx.inputs, iocontroller_callback, &ctx);
}


void FrankMain::release_accounts()
{
    ioc_release_memory_block(&m_accounts_export);
    ioc_release_memory_block(&m_accounts_import);
}
