/**

  @file    controller_root.h
  @brief   Root class for Buster application.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/



/**
****************************************************************************************************

  Buster main object.

****************************************************************************************************
*/
class ApplicationRoot
{
public:
    /* Constructor.
     */
    ApplicationRoot(
        const os_char *device_name,
        os_int device_nr,
        const os_char *network_name,
        const os_char *publish);

    /* Virtual destructor.
     */
    virtual ~ApplicationRoot();

    /* Functions to start, stop and thread function to run the application.
     */
    void start(const os_char *network_name, os_uint device_nr);
    void stop();
    osalStatus run(os_timer *ti);

    /* Basic server (ioserver extension) structure.
     */
    iocBServer m_bmain;

    /* Structure holding signals for the IO node.
     */
    buster_t m_signals;

    /* Network name.
     */
    os_char m_network_name[IOC_NETWORK_NAME_SZ];

    GinaIoDevice m_minion;
    gina_t *m_minion_def;

    BlinkLedSequence m_test_seq1;

};
