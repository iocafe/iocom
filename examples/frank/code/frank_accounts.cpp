/**

  @file    frank_accounts.cpp
  @brief   Host device/user accounts for an IO network.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    9.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "frank.h"


/**
****************************************************************************************************

  @brief Application constructor.

  @return  None.

****************************************************************************************************
*/
FrankAccounts::FrankAccounts(const os_char *network_name)
{
    /* Setup signal structure structure for accounts.
     */
    accounts_init_signal_struct(&m_accounts);

    /* Call basic server implementation to do the rest of accounts setup.
     */
    ioc_initialize_bserver_accounts(&m_baccts, &ioapp_root, network_name);
    ioc_setup_bserver_accounts(&m_baccts,
        &m_accounts.conf_exp.hdr,
        &m_accounts.conf_imp.hdr,
        ioapp_account_config,
        sizeof(ioapp_account_config),
        ioapp_account_defaults,
        sizeof(ioapp_account_defaults));

    /* Call basic server implementation macro to set up control stream.
     */
    IOC_SETUP_BSERVER_ACCOUNTS_STREAM_MACRO(m_baccts, m_accounts)
}


/**
****************************************************************************************************

  @brief Application destructor.

  Join worker thread to this thread and clean up.

  @return  None.

****************************************************************************************************
*/
FrankAccounts::~FrankAccounts()
{
    ioc_release_bserver_accounts(&m_baccts);
}

void FrankAccounts::run()
{
    ioc_run_bserver_accounts(&m_baccts);
}

