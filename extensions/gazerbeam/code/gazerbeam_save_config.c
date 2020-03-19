/**

  @file    gazerbeam_save_config.c
  @brief   LED light communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.3.2020

  Save WiFi configuration to persistent storage.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "gazerbeam.h"

/**
****************************************************************************************************

  @brief Save wifi configuration to persistent storage.

  X..
  @return  None.

****************************************************************************************************
*/
void gazerbeam_save_config(
    void)
{
    osalWifiPersistent block;
    // os_char str[OSAL_WIFI_PRM_SZ];

    ioc_load_persistent(OS_PBNR_WIFI, (os_char*)&block, sizeof(block));

#if 0
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
#endif

    ioc_save_persistent(OS_PBNR_WIFI, (const os_char*)&block, sizeof(block), OS_FALSE);
}
