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


/**
****************************************************************************************************

  Frank main object.

****************************************************************************************************
*/
class FrankMain
{
public:
    /* Constructor.
	 */
    FrankMain();

	/* Virtual destructor.
 	 */
    ~FrankMain();

    /* Memory block handles.
     */
    iocHandle
        m_accounts_export,
        m_accounts_import;

    /* IO device/user account IO definition structure.
     */
    accounts_t m_accounts_def;

    static const os_int MAX_APPS = 20;
    class FrankApplication *m_app[MAX_APPS];

    osalStatus listen_for_clients();
    osalStatus connect_to_device();
    void launch_app(os_char *network_name);

    void inititalize_accounts(const os_char *network_name);
    void release_accounts();

};
