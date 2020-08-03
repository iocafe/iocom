/**

  @file    abstract_application.h
  @brief   Base class for static IO device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

namespace IoDevice
{
    /**
    ************************************************************************************************

      Abstract application class.

    ************************************************************************************************
    */
    class AbstractApplication
    {
    public:

        /* Member functions called from application.
         */
        void init_application_basics(
            const os_char *device_name,
            const os_char *network_defaults,
            os_memsz network_defaults_sz,
            const IoPinsHdr *pins_header,
            os_int argc,
            const os_char *argv[]);

        void connect_it();
        void cleanup_app_basics();
        osalStatus run_app_library(os_timer *ti);

        /* IOCOM root object */
        iocRoot m_root;


    #if OS_CONTROL_CONSOLE_SUPPORT
        ioDeviceConsole m_console;
    #endif

        /* IO device/network configuration.  */
        iocNodeConf m_nodeconf;

        iocDeviceId *m_device_id;
        iocConnectionConfig *m_connconf;
        osalSecurityConfig *m_security;
        iocNetworkInterfaces *m_nics;
        iocWifiNetworks *m_wifis;

        osalLighthouseInfo m_lighthouse_server_info;
        LighthouseServer m_lighthouse_server;
        const IoPinsHdr *m_pins_header;
    };

}
