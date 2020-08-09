/**

  @file    gazerbeam_save_config.h
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
#pragma once
#ifndef GAZERBEAM_SAVE_CONFIG_H_
#define GAZERBEAM_SAVE_CONFIG_H_
#include "gazerbeam.h"

typedef enum gazerbeamFieldId
{
    GAZERBEAM_ID_WIFI_NETWORK = 1,
    GAZERBEAM_ID_WIFI_PASSWORD = 2,
    GAZERBEAM_ID_WIFI2_NETWORK = 3,
    GAZERBEAM_ID_WIFI2_PASSWORD = 4,
    GAZERBEAM_ID_COMMAND = 9,
    GAZERBEAM_ID_NETWORK_NAME_OVERRIDE = 10,
    GAZERBEAM_ID_DEVICE_NR_OVERRIDE = 11,
    GAZERBEAM_ID_CONNECT_IP_OVERRIDE = 11
}
gazerbeamFieldId;


/* Receive and process gazerbeam WiFi configuration.
 */
void gazerbeam_run_configurator(
    GazerbeamReceiver *gb,
    os_short flags);

/* Save wifi configuration to persistent storage.
 */
osalStatus gazerbeam_save_config(
    os_char *message,
    os_memsz message_sz);

/* Get one field from received gazerbeam message.
 */
osalStatus gazerbeam_get_config_item(
    gazerbeamFieldId id,
    os_char *field,
    os_memsz field_sz,
    os_char *message,
    os_memsz message_sz,
    os_short flags);

#endif
