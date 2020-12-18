/**

  @file    abstract_application.cpp
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
#include "iodevice.h"

using IoDevice::AbstractApplication;


static void iocom_application_communication_callback_1(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void iocom_application_communication_callback_2(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);


/**
****************************************************************************************************

  @brief Constructor.

  X...

  @return  None.

****************************************************************************************************
*/
void AbstractApplication::init_application_basics(
    const os_char *device_name,
    AbstractAppParams *prm)
{
    osPersistentParams persistentprm;
    os_char buf4[4];

    /* Setup error handling. Here we select to keep track of network state. We could also
       set application specific event handler callback by calling osal_set_net_event_handler().
     */
    osal_initialize_net_state();

    /* Initialize persistent storage
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.subdirectory = device_name;
#if OSAL_MICROCONTROLLER == 0
    for (os_int i = 1; i < prm->argc; i++) {
        os_strncpy(buf4, prm->argv[i], sizeof(buf4));
        if (!os_strcmp(buf4, "-p=")) {
            persistentprm.path = prm->argv[i] + 3;
        }
    }
#endif
    os_persistent_initialze(&persistentprm);

    /* Initialize communication root object.
     */
    ioc_initialize_root(&m_root);

    /* Use devicedir library for development testing, initialize.
     */
    io_initialize_device_console(&m_console, &m_root);

    /* Setup IO pins.
     */
    m_pins_header = prm->pins_header;
    pins_setup(m_pins_header, PINS_DEFAULT);

    /* Load device/network configuration and device/user account congiguration
       (persistent storage is typically either file system or micro-controller's flash).
       Defaults are set in network-defaults.json and in account-defaults.json.
     */
    ioc_load_node_config(&m_nodeconf, prm->network_defaults,
        prm->network_defaults_sz, device_name, IOC_LOAD_PBNR_NODE_CONF);
    m_device_id = ioc_get_device_id(&m_nodeconf);
    ioc_set_iodevice_id(&m_root, device_name, m_device_id->device_nr,
        m_device_id->password, m_device_id->network_name);

    ioc_initialize_dynamic_root(&m_root);

    /* Set callback function to receive information about new dynamic memory blocks.
     */
    // ioc_set_root_callback(&iocom_root, app_root_callback, OS_NULL);

    /* Get service TCP port number and transport (IOC_TLS_SOCKET or IOC_TCP_SOCKET).
     */
    m_connconf = ioc_get_connection_conf(&m_nodeconf);
    ioc_get_lighthouse_info(m_connconf, &m_lighthouse_server_info);

    /* Setup network interface configuration and initialize transport library. This is
       partyly ignored if network interfaces are managed by operating system
       (Linux/Windows,etc),
     */
    m_nics = ioc_get_nics(&m_nodeconf);
    m_wifis = ioc_get_wifis(&m_nodeconf);
    m_security = ioc_get_security_conf(&m_nodeconf);
    osal_tls_initialize(m_nics->nic, m_nics->n_nics, m_wifis->wifi,
        m_wifis->n_wifi, m_security);
}


void AbstractApplication::connect_application()
{
    /* Connect PINS library to IOCOM library
     */
    pins_connect_iocom_library(m_pins_header);

    /* If we are have multithreading and SPI or I2C support, try to start SPI thread.
       If no SPI ir I2C chips are configured (in JSON, etc), the function does nothing.
     */
#if OSAL_MULTITHREAD_SUPPORT && (PINS_SPI || PINS_I2C)
    pins_start_multithread_devicebus(0);
#endif

     /* Connect to network.
     */
    ioc_connect_node(&m_root, m_connconf, IOC_DYNAMIC_MBLKS|IOC_CREATE_THREAD_COND);

    /* Initialize light house. Sends periodic UDP broadcards to so that this service
       can be detected in network.
     */
    ioc_initialize_lighthouse_server(&m_lighthouse_server, m_device_id->publish,
        &m_lighthouse_server_info, OS_NULL);
}


void AbstractApplication::application_cleanup()
{
    /* Finished with lighthouse.
     */
    ioc_release_lighthouse_server(&m_lighthouse_server);

    /* Stop SPI and I2C threads.
     */
#if OSAL_MULTITHREAD_SUPPORT && (PINS_SPI || PINS_I2C)
    pins_stop_multithread_devicebus();
#endif

    pins_shutdown(m_pins_header);

    /* Release any memory allocated for node configuration.
    */
    ioc_release_node_config(&m_nodeconf);

    ioc_release_root(&m_root);
    osal_tls_shutdown();
}


osalStatus AbstractApplication::run_appplication_basics(
    os_timer *ti)
{
    osalStatus s;

    /* The call is here for development/testing.
     */
    s = io_run_device_console(&m_console);
    if (s) return s;

    /* Run light house (send periodic UDP broadcasts so that this service can be detected)
     */
    ioc_run_lighthouse_server(&m_lighthouse_server, ti);

    return OSAL_SUCCESS;
}


/* Enable communication callback 1 for a memory block.
   Multiple memory block can be connected to the same callback function.
 */
void AbstractApplication::enable_communication_callback_1(
    struct iocHandle *handle)
{
    /* Call communication_callback() when data is received, etc..
     */
    ioc_add_callback(handle, iocom_application_communication_callback_1, this);
}

void AbstractApplication::communication_callback_1(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags)
{
    osal_debug_error("communication_callback_1 not implemented by application");
}


/* Enable communication callback 2 for a memory block.
   Multiple memory block can be connected to the same callback function.
 */
void AbstractApplication::enable_communication_callback_2(
    struct iocHandle *handle)
{
    /* Call communication_callback() when data is received, etc..
     */
    ioc_add_callback(handle, iocom_application_communication_callback_2, this);
}

void AbstractApplication::communication_callback_2(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags)
{
    osal_debug_error("communication_callback_1 not implemented by application");
}

static void iocom_application_communication_callback_1(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    AbstractApplication *app;
    app = (AbstractApplication*)context;
    app->communication_callback_1(handle, start_addr, end_addr, flags);
}

static void iocom_application_communication_callback_2(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    AbstractApplication *app;
    app = (AbstractApplication*)context;
    app->communication_callback_2(handle, start_addr, end_addr, flags);
}
