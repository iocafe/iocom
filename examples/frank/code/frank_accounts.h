/**

  @file    frank_accounts.h
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


/**
****************************************************************************************************
  Class to get network configuration in separate thread.
****************************************************************************************************
*/
#if 0
class FrankAccounts
{
public:
    /* Constructor and virtual destructor.
	 */
    FrankAccounts(const os_char *network_name);
    ~FrankAccounts();
    void run();

private:
    /* IO device/user account IO definition structure.
     */
    accounts_t m_accounts;

    /* Basic server (from ioserver extensions) accounts structure.
     */
    iocBServerAccounts m_baccts;
};
#endif
