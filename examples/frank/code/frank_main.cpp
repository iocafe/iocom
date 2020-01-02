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
FrankMain::FrankMain()
{
    os_int i;

    for (i = 0; i < MAX_APPS; i++)
        m_app[i] = OS_NULL;
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

    ep = ioc_initialize_end_point(OS_NULL, &frank_root);
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

    con = ioc_initialize_connection(OS_NULL, &frank_root);
    os_memclear(&conprm, sizeof(conprm));

    conprm.iface = iface;
    conprm.flags = IOC_SOCKET|IOC_CREATE_THREAD|IOC_DYNAMIC_MBLKS; /* Notice IOC_DYNAMIC_MBLKS */
    conprm.parameters = "127.0.0.1";
    ioc_connect(con, &conprm);

    os_sleep(100);
    return OSAL_SUCCESS;
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

    blockprm.mblk_name = m_accounts_def.exp.hdr.mblk_name;
    blockprm.nbytes = m_accounts_def.exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC /* |IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_accounts_export, OS_NULL, &frank_root, &blockprm);

    blockprm.mblk_name = m_accounts_def.imp.hdr.mblk_name;
    blockprm.nbytes = m_accounts_def.imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC /* |IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_accounts_import, OS_NULL, &frank_root, &blockprm);

    doit(&m_accounts_def.imp.hdr, &m_accounts_import);
    doit(&m_accounts_def.exp.hdr, &m_accounts_export);

    /* Set callback to detect received data and connection status changes.
     */
    // ioc_add_callback(&ctx.inputs, iocontroller_callback, &ctx);
}


void FrankMain::release_accounts()
{
    ioc_release_memory_block(&m_accounts_export);
    ioc_release_memory_block(&m_accounts_import);
}
