/**

  @file    devicedir_info.c
  @brief   Get information about the IO device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    25.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"

/* Forward referred static functions.
 */
static const os_char *os_devicedir_get_network_state_string(
    void);


/**
****************************************************************************************************

  @brief Show define and network information.

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
        os_strncat(buf, osal_str_asterisk, sizeof(buf));
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

    devicedir_append_str_param(list, "state", os_devicedir_get_network_state_string(),
        DEVICEDIR_NEW_LINE|DEVICEDIR_TAB);

    /* End synchronization.
     */
    ioc_unlock(root);
    osal_stream_print_str(list, "\n}\n", 0);
}

/**
****************************************************************************************************

  @brief Get string description of network state.
  @anchor devicedir_get_network_state_string

  The devicedir_get_network_state_string() function examines network state structure and and
  returns string which best describes it.

  @return  String describing network state.

****************************************************************************************************
*/
static const os_char *os_devicedir_get_network_state_string(
    void)
{
    const os_char *state_str = "network ok";
    osaLightHouseClientState lighthouse_state;
    osalGazerbeamConnectionState gbs;

    /* If Gazerbeam configuration (WiFi with Android phone) is on?
     */
    gbs = osal_get_network_state_int(OSAL_NS_GAZERBEAM_CONNECTED, 0);
    if (gbs)
    {
        state_str = (gbs == OSAL_NS_GAZERBEAM_CONFIGURATION_MATCH)
                   ? "configuration matches" : "configuring";
        goto setit;
    }

    /* If WiFi is not connected?
     */
    if (osal_get_network_state_int(OSAL_NS_NETWORK_USED, 0) &&
        !osal_get_network_state_int(OSAL_NS_NETWORK_CONNECTED, 0))
    {
        state_str = "network not connected";
        goto setit;
    }

    /* Check for light house.
     */
    lighthouse_state
        = (osaLightHouseClientState)osal_get_network_state_int(OSAL_NS_LIGHTHOUSE_STATE, 0);
    if (lighthouse_state != OSAL_LIGHTHOUSE_NOT_USED &&
        lighthouse_state != OSAL_LIGHTHOUSE_OK)
    {
        state_str = (lighthouse_state == OSAL_LIGHTHOUSE_NOT_VISIBLE)
            ? "server multicast not received"
            : "no server multicast for requested network";
        goto setit;
    }

    /* Certificates/keys not loaded.
     */
    if (/* osal_get_network_state_int(OSAL_NS_SECURITY_CONF_ERROR, 0) || */
        osal_get_network_state_int(OSAL_NS_NO_CERT_CHAIN, 0))
    {
        state_str = "security configuration error";
        goto setit;
    }

    /* If no connected sockets?
     */
    if (osal_get_network_state_int(OSAL_NRO_CONNECTED_SOCKETS, 0) == 0)
    {
        state_str =  "no connection to server";
        goto setit;
    }

setit:
    return state_str;
}
