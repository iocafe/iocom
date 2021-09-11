/**

  @file    ioc_auto_device_nr.h
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
#pragma once
#ifndef IOC_AUTO_DEVICE_NR_H_
#define IOC_AUTO_DEVICE_NR_H_
#include "iocom.h"

struct iocRoot;

/** Start automatically given device numbers from IOC_AUTO_DEVICE_NR + 1. This can be changed by
    compiler define, but communicating devices using automatic device numbers
    must use the same define.
 */
#ifndef IOC_AUTO_DEVICE_NR
#define IOC_AUTO_DEVICE_NR 9000
#endif
#define IOC_TO_AUTO_DEVICE_NR (IOC_AUTO_DEVICE_NR-1)
#define IOC_AUTO_DEVICE_NR_START (IOC_AUTO_DEVICE_NR + 31001)
#define IOC_RESERVED_AUTO_DEVICE_NR_START (IOC_AUTO_DEVICE_NR+1)

/* If we support complete automatic device numbering, needed for server side only
 */
#if OSAL_SECRET_SUPPORT

/* Number of automatic device IDs to memorize.
 */
#ifndef IOC_NRO_SAVED_DEVICE_NRS
  #if IOC_AUTO_DEVICE_NR_SUPPORT
    #define IOC_NRO_SAVED_DEVICE_NRS 5
  #else
    #define IOC_NRO_SAVED_DEVICE_NRS 40
  #endif
#endif

typedef struct iocSavedAutoNrData
{
    os_uint reserve_auto_device_nr;

    os_uint device_nr[IOC_NRO_SAVED_DEVICE_NRS];
    os_uchar unique_id_bin[IOC_NRO_SAVED_DEVICE_NRS][OSAL_UNIQUE_ID_BIN_SZ];
}
iocSavedAutoNrData;

#endif

typedef struct iocAutoDeviceNrState
{
    /** Automatic device number, used if device number is 0.
     */
    os_uint auto_device_nr;

#if IOC_AUTO_DEVICE_NR_SUPPORT

    os_boolean data_loaded;

    /** Saved device number/device ID pairs.
     */
    iocSavedAutoNrData saved;
#endif
}
iocAutoDeviceNrState;


/* Get automatic client device number.
 */
os_uint ioc_get_automatic_device_nr(
    struct iocRoot *root,
    os_uchar *unique_id_bin);

#endif
