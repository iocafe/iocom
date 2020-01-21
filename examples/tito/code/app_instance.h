/**

  @file    app_instance.h
  @brief   IO controller application's base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/


/**
****************************************************************************************************
  Tito application base class.
****************************************************************************************************
*/
class AppInstance
{
public:
    /* Constructor and virtual destructor.
	 */
    AppInstance();
    ~AppInstance();

    void initialize(const os_char *network_name, os_uint device_nr);

    /* Functions to start, stop and thread function to run the application.
     */
    void start(const os_char *network_name, os_uint device_nr);

    void stop();
    void run();

    /* Network topology stuff.
     */
    os_char m_network_name[IOC_NETWORK_NAME_SZ];

    GinaIoDevice m_gina1;
    GinaIoDevice m_gina2;
    gina_t *m_gina1_def;
    gina_t *m_gina2_def;

    BlinkLedSequence m_test_seq1;
};
