/**

  @file    frank_main.h
  @brief   Controller example with static IO defice configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used, 
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
        const os_char *network_name,
        const os_char *publish);

	/* Virtual destructor.
 	 */
    ~FrankMain();

    osalStatus listen_for_clients();
    osalStatus connect_to_device();

    /* The run is repeatedly to keep control stream alive.
     */
    void run();

    void launch_app(os_char *network_name);

private:
    /* Basic server (ioserver extension) structure.
     */
    iocBServerMain m_bmain;

    /* Structure holding signals for the IO node.
     */
    frank_t m_signals;

    static const os_int MAX_APPS = 20;
    class FrankApplication *m_app[MAX_APPS];
};
