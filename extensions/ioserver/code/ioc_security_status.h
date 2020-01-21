/**

  @file    ioc_security_status.h
  @brief   Secutiry status, like new devices and security alerts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "ioserver.h"

struct iocBServer;

typedef enum
{
    IOC_NOTE_NONE = 0,
    IOC_NOTE_NEW_IO_DEVICE,
    IOC_NOTE_WRONG_IO_DEVICE_PASSWORD,
    IOC_NOTE_BLACKLISTED_IO_DEVICE,
}
iocNoteCode;


typedef struct iocSecurityNotification
{
    const os_char *user;
    const os_char *password;
    const os_char *privileges;
    const os_char *ip;
    const os_char *text;
    const os_char *network_name;
}
iocSecurityNotification;

/* For setting up signal pointers for "new device" and "alarm" tables in memory block.
 */
#define IOC_MAX_NEW_DEVICE_NOTIFICATIONS 4
#define IOC_MAX_ALARM_NOTIFICATIONS 4
typedef struct
{
    iocSignal *user_name;
    iocSignal *password;
    iocSignal *privileges;
    iocSignal *ip;
    iocSignal *count;
    iocSignal *timer;
    iocSignal *text;
}
iocNotificationSignalRow;

/* Network security status structure.
 */
typedef struct iocSecurityStatus
{
    /* Security status structure initialized flag.
     */
    os_boolean initialized;

    /* iocom signals for "new device" table.
     */
    iocNotificationSignalRow new_device[IOC_MAX_NEW_DEVICE_NOTIFICATIONS];
    os_short new_device_nrows;

    /* iocom signals for "alarms" table.
     */
    iocNotificationSignalRow alarm[IOC_MAX_ALARM_NOTIFICATIONS];
    os_short alarm_nrows;
}
iocSecurityStatus;


void ioc_secutiry_notify(
    struct iocBServer *m,
    iocNoteCode code,
    iocSecurityNotification *note);

