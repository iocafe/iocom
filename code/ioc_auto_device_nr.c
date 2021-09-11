/**

  @file    ioc_auto_device_nr.c
  @brief   Automatic device numbering.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

/* Forward referred static functions.
 */
#if IOC_AUTO_DEVICE_NR_SUPPORT
static os_uint ioc_get_reserved_autonr(
    os_uchar *unique_id_bin,
    iocAutoDeviceNrState *a);

static osalStatus ioc_load_autonr_data(
    iocSavedAutoNrData *nr_data);

static void ioc_save_autonr_data(
    iocSavedAutoNrData *nr_data);
#endif

/**
****************************************************************************************************

  @brief Get automatic client device number.
  @anchor ioc_get_automatic_device_nr

  Some devices, like UI clients, games, etc, may not have device number associate with them
  and return IOC_AUTO_DEVICE_NR as device number to controller. Controller uses this
  function to assign unique device ID for the device. If same device reconnects, it is best to
  reassing it with the same number as before. Unique device ID can be used to recognize devices.

  ioc_lock() must be on before calling this function.

  @param   root Pointer to the root object.
  @param   unique_id_bin Unique ifentifier of remote device, OS_NULL if not available.
  @return  Unique device identifier IOC_AUTO_DEVICE_NR + 1 .. 0xFFFFFFFF.

****************************************************************************************************
*/
os_uint ioc_get_automatic_device_nr(
    iocRoot *root,
    os_uchar *unique_id_bin)
{
    iocConnection *con;
    os_uint id;
    os_int count;

#if IOC_AUTO_DEVICE_NR_SUPPORT
    /* If we have unique ID, which is not NULL or full of zeros, use reserved method.
     */
    if (unique_id_bin) {
        for (count = 0; count<OSAL_UNIQUE_ID_BIN_SZ; count++) {
            if (unique_id_bin[count]) {
                return ioc_get_reserved_autonr(unique_id_bin, &root->autonr);
            }
        }
    }
#endif

    /* Just return next number
     */
    if (root->autonr.auto_device_nr) {
        id = root->autonr.auto_device_nr++;
    }

    /* We run out of numbers. Strange, this can be possible only if special effort is
       made for this to happen. Handle anyhow.
     */
    count = 100000;
    while (count--)
    {
        id = (os_uint)osal_rand(IOC_AUTO_DEVICE_NR + 1, 0x7FFFFFFFL);
        for (con = root->con.first;
             con;
             con = con->link.next)
        {
            if (id == con->auto_device_nr) break;
        }
        if (con == OS_NULL) return id;
    }

    osal_debug_error("Out of numbers (devices)");
    return 1;
}

#if IOC_AUTO_DEVICE_NR_SUPPORT

static os_uint ioc_get_reserved_autonr(
    os_uchar *unique_id_bin,
    iocAutoDeviceNrState *a)
{
    iocSavedAutoNrData *d;
    os_int i, smallest_i;
    os_uint id;

    d = &a->saved;

    if (!a->data_loaded) {
        ioc_load_autonr_data(d);
        a->data_loaded = OS_TRUE;
    }

    /* Check if we already have reserved device number for this ID.
     */
    for (i = 0; i<IOC_NRO_SAVED_DEVICE_NRS; i++) {
        if (!os_memcmp(unique_id_bin, d->unique_id_bin[i], OSAL_UNIQUE_ID_BIN_SZ)) {
            return d->device_nr[i];
        }
    }

    /* Make sure that next device number to reserve is in range from IOC_RESERVED_AUTO_DEVICE_NR_START to
       IOC_AUTO_DEVICE_NR_START - 2.
     */
    if (d->reserve_auto_device_nr < IOC_RESERVED_AUTO_DEVICE_NR_START ||
        d->reserve_auto_device_nr > IOC_AUTO_DEVICE_NR_START - 2)
    {
        d->reserve_auto_device_nr = IOC_RESERVED_AUTO_DEVICE_NR_START;
    }

    /* Select which reservation table row to use. Start from smallest, normally reservations numbers
       are incremented (unless we flood whole reservation number space, which should not be possible).
       Roll around is handled anyhow.
     */
    id = d->device_nr[0];
    if (id < d->reserve_auto_device_nr) {
        id += IOC_AUTO_DEVICE_NR_START;
    }
    smallest_i = 0;
    for (i = 0; i<IOC_NRO_SAVED_DEVICE_NRS; i++) {
        if (d->device_nr[i] >= d->reserve_auto_device_nr) {
            if (d->device_nr[i] < id) {
                smallest_i = i;
                id = d->device_nr[i];
            }
        }
        if (d->device_nr[i] + IOC_AUTO_DEVICE_NR_START < id) {
            smallest_i = i;
            id = d->device_nr[i] + IOC_AUTO_DEVICE_NR_START;
        }
    }

    d->device_nr[smallest_i] = d->reserve_auto_device_nr++;
    os_memcpy(d->unique_id_bin[smallest_i], unique_id_bin, OSAL_UNIQUE_ID_BIN_SZ);

    /* Save reservation data and return decision on device number.
     */
    ioc_save_autonr_data(d);
    return d->device_nr[smallest_i];
}

static osalStatus ioc_load_autonr_data(
    iocSavedAutoNrData *d)
{
    return os_load_persistent(OS_PBNR_AUTONR_DATA, (os_char*)d, sizeof(iocSavedAutoNrData));
}

static void ioc_save_autonr_data(
    iocSavedAutoNrData *d)
{
    os_save_persistent(OS_PBNR_AUTONR_DATA, (os_char*)d, sizeof(iocSavedAutoNrData), OS_FALSE);
}

#endif
