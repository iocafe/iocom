/**

  @file    app_root.h
  @brief   IO application's root class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  The root class starts and runs basic server code from ioserver extension library. This
  provides basic functionality like ability to connect to this application and configure it,
  set up IO networks and user accounts, etc.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
class AppRoot
{
public:
    AppRoot(
        const os_char *device_name,
        os_int device_nr,
        const os_char *network_name,
        const os_char *publish,
        iocLighthouseInfo *lighthouse_info);

    ~AppRoot();

    /* The run is repeatedly to keep control stream alive.
     */
    osalStatus run();

    /* When an IO network is connected for the first time, IO application instance is launced.
     */
    void launch_app(os_char *network_name);

private:
    /* Basic server (ioserver extension) structure.
     */
    iocBServer m_bmain;

    /* Light house state structure. The lighthouse sends periodic UDP broadcards
       to so that this service can be detected in network.
     */
    LighthouseServer m_lighthouse;

    /* Structure holding signals for the IO node.
     */
    frank_t m_signals;

    /* Array of pointers to launced application instances.
     */
    static const os_int MAX_APPS = 20;
    class AppInstance *m_app[MAX_APPS];
};
