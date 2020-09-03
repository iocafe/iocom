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
    typedef struct AbstractAppParams {
        const os_char *device_name;
        const os_char *network_defaults;
        os_memsz network_defaults_sz;
        const IoPinsHdr *pins_header;
        os_int argc;
        const os_char **argv;
    }
    AbstractAppParams;

    /**
    ************************************************************************************************

      Abstract application class.

    ************************************************************************************************
    */
    class AbstractApplication
    {
    public:
        virtual ~AbstractApplication() {};

        /* Member functions called from actual IO device application. */
        void init_application_basics(
            const os_char *device_name,
            AbstractAppParams *prm);

        void connect_application();

        void application_cleanup();

        osalStatus run_appplication_basics(
            os_timer *ti);

        void enable_communication_callback_1(
            struct iocHandle *handle);

        virtual void communication_callback_1(
            struct iocHandle *handle,
            os_int start_addr,
            os_int end_addr,
            os_ushort flags);

        void enable_communication_callback_2(
            struct iocHandle *handle);

        virtual void communication_callback_2(
            struct iocHandle *handle,
            os_int start_addr,
            os_int end_addr,
            os_ushort flags);

        /* IOCOM root object */
        iocRoot m_root;

        IO_DEVICE_CONSOLE(m_console);

        /* IO device/network, etc configuration. */
        iocNodeConf m_nodeconf;
        iocDeviceId *m_device_id;
        iocConnectionConfig *m_connconf;
        osalSecurityConfig *m_security;
        iocNetworkInterfaces *m_nics;
        iocWifiNetworks *m_wifis;

        /* Lighthouse for IO device discovery by UDP multicasts. */
        osalLighthouseInfo m_lighthouse_server_info;
        LighthouseServer m_lighthouse_server;
        const IoPinsHdr *m_pins_header;
    };
}

