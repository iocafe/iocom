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
    FrankMain(
        const os_char *device_name,
        os_int device_nr,
        const os_char *network_name);

	/* Virtual destructor.
 	 */
    ~FrankMain();

    /* Setup the memory blocks for the IO node.
     */
    void setup_mblks();
    void release_mblks();

    /* The run is repeatedly to keep control stream alive.
     */
    void setup_ctrl_stream();
    void run();

    /* Identification of this IO network node.
     */
    os_char m_device_name[IOC_NAME_SZ];
    os_int m_device_nr;
    os_char m_network_name[IOC_NETWORK_NAME_SZ];

    /* Memory block handles.
     */
    iocHandle
        m_exp,
        m_imp,
        m_conf_exp,
        m_conf_imp,
        m_info;

    /* Structure holding signals for the IO node.
     */
    frank_t
        m_signals;

    /* Control stream to configure the IO node.
     */
    iocStreamerParams m_ctrl_stream_params;
    iocControlStreamState m_ctrl_state;

    /* Memory block handles.
     */
    iocHandle
        m_accounts_export,
        m_accounts_import;

    /* IO device/user account IO definition structure.
     */
    accounts_t
        m_accounts;

    static const os_int MAX_APPS = 20;
    class FrankApplication *m_app[MAX_APPS];

    osalStatus listen_for_clients();
    osalStatus connect_to_device();
    void launch_app(os_char *network_name);

    void inititalize_accounts(const os_char *network_name);
    void release_accounts();

};
