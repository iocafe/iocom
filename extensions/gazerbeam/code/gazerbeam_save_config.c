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

#if GAZERBEAM_PINS_SUPPORT
#define PINS_OS_INT_HANDLER_HDRS 1
#include "pins.h"
#endif


/**
****************************************************************************************************

  @brief Receive and process gazerbeam WiFi configuration.

  Call this function periodically to get and process Wifi configuration messages from Android
  phone flash light -> phototransistor. If message is received

  @param   gb Pointer to the GazerbeamReceiver structure.
  @param   flags Reserved for future, set GAZERBEAM_DEFAULT (0) for now.
  @return  None.

****************************************************************************************************
*/
void gazerbeam_run_configurator(
    GazerbeamReceiver *gb,
    os_short flags)
{
    os_char buf[GAZERBEAM_MAX_MSG_SZ];
    os_memsz buf_sz;

    /* Get Wifi configuration messages from Android phone flash light -> phototransistor.
     */
    buf_sz = gazerbeam_get_message(gb, buf, sizeof(buf), GAZERBEAM_DEFAULT);
    if (buf_sz > 0)
    {
#if GAZERBEAM_PINS_SUPPORT
        /* If we are using pin interrupt, disable it.
         */
        if (gb->pin)
        {
            pin_detach_interrupt(gb->pin);
        }
#endif

        /* Save stuff
         */
        if (gazerbeam_save_config(buf, buf_sz) == OSAL_SUCCESS)
        {
            /* Here we reboot only if something changed.
             */
            osal_reboot(0);
        }

#if GAZERBEAM_PINS_SUPPORT
        /* If we wew using pin interrupt, enable it back.
         */
        if (gb->pin)
        {
            pinInterruptParams prm;
            os_memclear(&prm, sizeof(prm));
            prm.int_handler_func = gb->int_handler_func;
            prm.flags = PINS_INT_CHANGE;
            pin_attach_interrupt(gb->pin, &prm);
        }
#endif
    }
}


/**
****************************************************************************************************

  @brief Save wifi configuration from gazerbeam message to persistent storage.

  @param   message Received Gazebeam message.
  @param   message_sz Message size in bytes.
  @return  OSAL_SUCCESS if field was succesfully set.
           OSAL_NOTHING_TO_DO if field was unchanged.

****************************************************************************************************
*/
osalStatus gazerbeam_save_config(
    os_char *message,
    os_memsz message_sz)
{
    osalWifiPersistent block;
    osalStatus s, rval = OSAL_NOTHING_TO_DO;

    os_load_persistent(OS_PBNR_WIFI, (os_char*)&block, sizeof(block));

    s = gazerbeam_get_config_item(GAZERBEAM_ID_WIFI_NETWORK,
        block.wifi[0].wifi_net_name, OSAL_WIFI_PRM_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

    s = gazerbeam_get_config_item(GAZERBEAM_ID_WIFI_PASSWORD,
        block.wifi[0].wifi_net_password, OSAL_WIFI_PRM_SZ,
        message, message_sz, GAZERBEAM_DEFAULT);
    if (s == OSAL_SUCCESS) rval = s;

    if (rval == OSAL_SUCCESS) {
        os_save_persistent(OS_PBNR_WIFI, (const os_char*)&block, sizeof(block), OS_FALSE);
    }

    return rval;
}


/**
****************************************************************************************************

  @brief Get one field from received gazerbeam message.

  The gazerbeam_get_config_item function gets value of a field specified by id from message
  received through gazerbeam.

  If the message doesn't contain requested field, field is left unmodified.

  @param   id Identifier of the field to get.
  @param   field Pointer to buffer where to store '\0' terminated field value.
  @param   field_sz Size if field value buffer in bytes.
  @param   message Received Gazebeam message.
  @param   message_sz Message size in bytes.
  @param   flags Reserved for future, set GAZERBEAM_DEFAULT (0) for now.
  @return  OSAL_SUCCESS if field was succesfully set.
           OSAL_NOTHING_TO_DO if field was unchanged.
           OSAL_STATUS_FAILED if field was not set in message.

****************************************************************************************************
*/
osalStatus gazerbeam_get_config_item(
    gazerbeamFieldId id,
    os_char *field,
    os_memsz field_sz,
    os_char *message,
    os_memsz message_sz,
    os_short flags)
{
    os_uchar *p, *e;
    os_uint sz;

    p = (os_uchar*)message;
    e = p + message_sz;
    while (p + 2 < e)
    {
        sz = p[1];
        if (p[0] == id && sz)
        {
            if (sz >= field_sz) sz = (os_uint)field_sz - 1;

            /* If unchanged?
             */
            if (!os_memcmp(field, p + 2, sz) && field[sz] == '\0') {
                return OSAL_NOTHING_TO_DO;
            }

            os_memcpy(field, p + 2, sz);
            field[sz] = '\0';
            return OSAL_SUCCESS;
        }
        p += 2 + sz;
    }

    return OSAL_STATUS_FAILED;
}