/**

  @file    ioc_security_status.h
  @brief   Secutiry status, like new devices and security alerts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
    IOC_NOTE_WRONG_IO_DEVICE_PASSWORD
}
iocNoteCode;


typedef struct iocSecurityNotification
{
    const os_char *user;
    const os_char *password;
    const os_char *privileges;
    const os_char *ip;
    const os_char *network_name;
}
iocSecurityNotification;

/* Signal pointers for "new devices" table row in memory block.
 */
#define IOC_MAX_NEW_DEVICE_NOTIFICATIONS 4
typedef struct
{
    iocSignal *user_name;
    iocSignal *password;
    iocSignal *privileges;
    iocSignal *ip;
    iocSignal *count;
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
    os_timer new_device_timer[IOC_MAX_NEW_DEVICE_NOTIFICATIONS];
    os_boolean new_device_is_set[IOC_MAX_NEW_DEVICE_NOTIFICATIONS];
    os_short new_device_nrows;
}
iocSecurityStatus;


void ioc_security_notify(
    struct iocBServer *m,
    iocNoteCode code,
    iocSecurityNotification *note);

/* Run security (make "new device" notifications time out)
 */
void ioc_run_security(
    struct iocBServer *m);
