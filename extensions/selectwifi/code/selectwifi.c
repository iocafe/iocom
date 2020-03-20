/**

  @file    selectwifi.c
  @brief   Set wifi network name and password over blue tooth or serial port.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    3.2.2020

  Configure microcontroller WiFi, etc, over blue tooth connection.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#define SELECTWIFI_INTERNALS
#include "selectwifi.h"

/* Define 1 to enable testing selectwi library trough socket. Normally
   it works only trough serial and blue tooth connections.
 */
#define SELECTWIFI_ENABLE_SOCKET_TEST (OSAL_MICROCONTROLLER == 0)

/* We may want to run connection in separate thread, if multithreading is supported?
   Define SWL_CT_FLAG either as IOC_CREATE_THREAD or zero.
 */
#if OSAL_MULTITHREAD_SUPPORT
    #define SWL_CT_FLAG IOC_CREATE_THREAD
#else
    #define SWL_CT_FLAG 0
#endif


/* Sey maximum number of connections and connection type to enable using
   macros in ioboard.h to calculate necessary pool size.
 */
#define IOBOARD_MAX_CONNECTIONS 1
#if SELECTWIFI_ENABLE_SOCKET_TEST
#define IOBOARD_CTRL_CON IOBOARD_CTRL_LISTEN_SOCKET
#else
#define IOBOARD_CTRL_CON IOBOARD_CTRL_LISTEN_SERIAL
#endif

/* Static global signal structure for select wifi. Calculate precise pool size necessary
   using macros in ioboard.h header (somewhat complex).
 */
iocSelectWiFi swf;
static os_char swf_pool[IOBOARD_POOL_SIZE(IOBOARD_CTRL_CON, IOBOARD_MAX_CONNECTIONS,
        SELECTWIFI_EXP_MBLK_SZ, SELECTWIFI_IMP_MBLK_SZ)
        + IOBOARD_POOL_DEVICE_INFO(IOBOARD_MAX_CONNECTIONS)];

/* Prototypes for forward referred static functions.
 */
static void ioc_selectfiwi_imp_data_changed(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void ioc_selectfiwi_load(
    void);

static void ioc_selectfiwi_save(
    void);


/**
****************************************************************************************************

  @brief Initialize selecy wifi library.

  The ioc_initialize_selectwifi() function sets up the wifi select functionality.

  @param   swf Pointer to wifi network select object to be initialized.
  @param   prm Flat parameter structure.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_selectwifi(
    iocSelectWiFiParams *prm)
{
    const os_char device_name[] = "wifi";
    os_int device_nr = IOC_AUTO_DEVICE_NR;
    const os_char network_name[] = "iocafenet";
    const os_char *parameters;
    iocMemoryBlockParams blockprm;
    iocConnectionParams conprm;
    iocEndPointParams epprm;

    /* Parameters
     */
    os_memclear(&swf, sizeof(iocSelectWiFi));
    swf.transport = IOC_SWF_SOCKET_TEST;
    parameters = OS_NULL;
    if (prm)
    {
        swf.transport = prm->transport;
        parameters = prm->parameters;
    }

    ioc_initialize_root(&swf.root);
    ioc_set_memory_pool(&swf.root, swf_pool, sizeof(swf_pool));
    ioc_set_iodevice_id(&swf.root, device_name, device_nr, OS_NULL, network_name);

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = device_name;
    blockprm.device_nr = device_nr;
    blockprm.network_name = network_name;

    blockprm.mblk_name = selectwifi.exp.hdr.mblk_name;
    blockprm.nbytes = SELECTWIFI_EXP_MBLK_SZ;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC|IOC_FLOOR;
    ioc_initialize_memory_block(&swf.exp, &swf.exp_mblk, &swf.root, &blockprm);

    blockprm.mblk_name = selectwifi.imp.hdr.mblk_name;
    blockprm.nbytes = SELECTWIFI_IMP_MBLK_SZ;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC|IOC_FLOOR;
    ioc_initialize_memory_block(&swf.imp, &swf.imp_mblk, &swf.root, &blockprm);

    blockprm.mblk_name = "info";
    blockprm.buf = (char*)&selectwifi_signal_config;
    blockprm.nbytes = sizeof(selectwifi_signal_config);
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&swf.info, &swf.info_mblk, &swf.root, &blockprm);

    /* Load data from persistent storage to memory block
     */
    ioc_selectfiwi_load();

    /* Set callback to know when user wants to save changes.
     */
    ioc_add_callback(&swf.imp, ioc_selectfiwi_imp_data_changed, OS_NULL);
    os_get_timer(&swf.boot_timer);

    switch (swf.transport)
    {
        case IOC_SWF_BLUE_TOOTH:
        case IOC_SWF_SERIAL_PORT:
            os_memclear(&conprm, sizeof(conprm));
            swf.con = ioc_initialize_connection(OS_NULL, &swf.root);
            conprm.iface = swf.transport == IOC_SWF_BLUE_TOOTH
                ? OSAL_BLUETOOTH_IFACE : OSAL_SERIAL_IFACE;
            conprm.parameters = parameters;
            conprm.flags = IOC_LISTENER | IOC_SERIAL
                | IOC_DISABLE_SELECT | IOC_CONNECT_UP | SWL_CT_FLAG;
            ioc_connect(swf.con, &conprm);
            break;

#if SELECTWIFI_ENABLE_SOCKET_TEST
        case IOC_SWF_SOCKET_TEST:
            os_memclear(&epprm, sizeof(epprm));
            swf.epoint = ioc_initialize_end_point(OS_NULL, &swf.root);
            epprm.iface = OSAL_SOCKET_IFACE;
            epprm.parameters = parameters;
            epprm.flags = IOC_SOCKET | IOC_CONNECT_UP | SWL_CT_FLAG;
            ioc_listen(swf.epoint, &epprm);
            break;
#endif
    }
}


/**
****************************************************************************************************

  @brief Release resources allocated for select wifi library.

  The ioc_release_selectwifi() function releases memory and other resources allocated for
  wifi network select.

  @param   swf Pointer to wifi network select object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_selectwifi(
    void)
{
    /* ioc_release_memory_block(&swf.exp);
    ioc_release_memory_block(&swf.imp);
    ioc_release_memory_block(&swf.info); */
    ioc_release_root(&swf.root);
}


/**
****************************************************************************************************

  @brief Callback function when "imp" memory block changes.

  Here we want to detect is user have set on "save" flag (pressed save button, etc). If so,
  we want to modify network configuration of the device, save it and reboot.

  @param   swf Pointer to wifi network select object.
  @return  None.

****************************************************************************************************
*/
static void ioc_selectfiwi_imp_data_changed(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    if (flags != IOC_MBLK_CALLBACK_RECEIVE) return;

    if (ioc_is_my_address(&selectwifi.imp.save, start_addr, end_addr))
    {
        if (ioc_gets0_int(&selectwifi.imp.save))
        {
            if (!os_has_elapsed(&swf.boot_timer, 5000)) return;
            ioc_selectfiwi_save();
        }
    }
}


/**
****************************************************************************************************

  @brief Load wifi configuration from persistent storage.

  X..
  @return  None.

****************************************************************************************************
*/
static void ioc_selectfiwi_load(
    void)
{
    osalWifiPersistent block;

    os_load_persistent(OS_PBNR_WIFI, (os_char*)&block, sizeof(block));

    ioc_sets_str(&selectwifi.exp.net_1, block.wifi[0].wifi_net_name);

#ifdef SELECTWIFI_IMP_SET_NET_2_ARRAY_SZ
    ioc_sets_str(&selectwifi.exp.net_2, block.wifi[1].wifi_net_name);
#endif

#ifdef SELECTWIFI_IMP_SET_NET_3_ARRAY_SZ
    ioc_sets_str(&selectwifi.exp.net_3, block.wifi[2].wifi_net_name);
#endif

#ifdef SELECTWIFI_IMP_SET_NET_4_ARRAY_SZ
    ioc_sets_str(&selectwifi.exp.net_4, block.wifi[3].wifi_net_name);
#endif
}


/**
****************************************************************************************************

  @brief Save wifi configuration to persistent storage.

  X..
  @return  None.

****************************************************************************************************
*/
static void ioc_selectfiwi_save(
    void)
{
    osalWifiPersistent block;
    os_char str[OSAL_WIFI_PRM_SZ];

    os_load_persistent(OS_PBNR_WIFI, (os_char*)&block, sizeof(block));

    ioc_gets_str(&selectwifi.imp.set_net_1, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[0].wifi_net_name, str, OSAL_WIFI_PRM_SZ);
    ioc_gets_str(&selectwifi.imp.set_password_1, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[0].wifi_net_password, str, OSAL_WIFI_PRM_SZ);

#ifdef SELECTWIFI_IMP_SET_NET_2_ARRAY_SZ
    ioc_gets_str(&selectwifi.imp.set_net_2, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[1].wifi_net_name, str, OSAL_WIFI_PRM_SZ);
    ioc_gets_str(&selectwifi.imp.set_password_2, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[1].wifi_net_password, str, OSAL_WIFI_PRM_SZ);
#endif

#ifdef SELECTWIFI_IMP_SET_NET_3_ARRAY_SZ
    ioc_gets_str(&selectwifi.imp.set_net_3, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[2].wifi_net_name, str, OSAL_WIFI_PRM_SZ);
    ioc_gets_str(&selectwifi.imp.set_password_3, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[2].wifi_net_password, str, OSAL_WIFI_PRM_SZ);
#endif

#ifdef SELECTWIFI_IMP_SET_NET_4_ARRAY_SZ
    ioc_gets_str(&selectwifi.imp.set_net_4, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[3].wifi_net_name, str, OSAL_WIFI_PRM_SZ);
    ioc_gets_str(&selectwifi.imp.set_password_4, str, OSAL_WIFI_PRM_SZ);
    if (str[0]) os_strncpy(block.wifi[3].wifi_net_password, str, OSAL_WIFI_PRM_SZ);
#endif

    os_save_persistent(OS_PBNR_WIFI, (const os_char*)&block, sizeof(block), OS_FALSE);

    /* Load to show change to user immediately.
     */
    ioc_selectfiwi_load();
}


/**
****************************************************************************************************

  @brief Keep wifi selection functionality alive.

  The ioc_run_selectwifi() function needs to be called repeatedly to keep the
  functionality responsive.

  @param   swf Pointer to wifi network select object.
  @return  If working in something, the function returns OSAL_SUCCESS. Return value
           OSAL_NOTHING_TO_DO indicates that this thread can be switched to slow
           idle mode as far as the bserver knows.

****************************************************************************************************
*/
osalStatus ioc_run_selectwifi(
    void)
{
    return OSAL_SUCCESS;
}
