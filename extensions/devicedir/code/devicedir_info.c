/**

  @file    devicedir_dynamic.c
  @brief   List dynamic signals.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"

/**
****************************************************************************************************

  @brief Shor define and network information.

  The devicedir_info() function..

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write connection list JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_info(
    iocRoot *root,
    osalStream list,
    os_short flags)
{
    os_char buf[128], nbuf[OSAL_NBUF_SZ];

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    /* Synchronize.
     */
    ioc_lock(root);

    /* Device name and number.
     */
    osal_stream_print_str(list, "{", 0);
    os_strncpy(buf, root->device_name, sizeof(buf));

    if (root->device_nr == IOC_AUTO_DEVICE_NR) {
        os_strncat(buf, "*", sizeof(buf));
    }
    else {
        osal_int_to_str(nbuf, sizeof(nbuf), root->device_nr);
        os_strncat(buf, nbuf, sizeof(buf));
    }
    devicedir_append_str_param(list, "device", buf,
        DEVICEDIR_FIRST|DEVICEDIR_NEW_LINE|DEVICEDIR_TAB);

    devicedir_append_str_param(list, "network_name", root->network_name,
        DEVICEDIR_NEW_LINE|DEVICEDIR_TAB);

    osal_get_network_state_str(OSAL_NS_NIC_IP_ADDR, 0, buf, sizeof(buf));
    if (buf[0] != '\0') {
        devicedir_append_str_param(list, "nic", buf, DEVICEDIR_NEW_LINE|DEVICEDIR_TAB);
    }

    /* Connected wifi network.
     */
    osal_get_network_state_str(OSAL_NS_WIFI_NETWORK_NAME, 0, buf, sizeof(buf));
    if (buf[0] != '\0') {
        devicedir_append_str_param(list, "wifi", buf, DEVICEDIR_NEW_LINE|DEVICEDIR_TAB);
    }

    /* End synchronization.
     */
    ioc_unlock(root);
    osal_stream_print_str(list, "\n}\n", 0);
}

