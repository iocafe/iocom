/**

  @file    ioc_security_status.c
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
#define BSERVER_INTERNALS
#include "ioserver.h"

static os_int ioc_make_new_device_notification_table(
    iocNotificationSignalRow *table,
    os_int max_rows,
    iocBServerNetwork *n);

/**
****************************************************************************************************

  @brief Share security note in communication.
  @anchor ioc_secutiry_notify

  The ioc_secutiry_notify() function...

  @param   m Pointer to basic server object.
  @param   code Reason for notification, like IOC_NOTE_NEW_IO_DEVICE,
           IOC_NOTE_WRONG_IO_DEVICE_PASSWORD,  or IOC_NOTE_BLACKLISTED_IO_DEVICE.
  @param   note Pointer to security note structure. Contains network name, device/user name,
           password used to log in, etc.
  @return  None.

****************************************************************************************************
*/
void ioc_secutiry_notify(
    struct iocBServer *m,
    iocNoteCode code,
    iocSecurityNotification *note)
{
    iocBServerNetwork *n;
    os_int i;
    iocSecurityStatus *ss;

    if (m->networks == OS_NULL) return;


    for (i = 0; i<m->nro_networks; i++)
    {
        n = m->networks + i;
        if (os_strcmp(note->network_name, n->network_name)) continue;

        /* Initialize signal pointers for "new device" and "alarm" tables.
         */
        ss = &n->sec_status;
        if (!ss->initialized)
        {
            ss->new_device_nrows = ioc_make_new_device_notification_table(ss->new_device,
                IOC_MAX_NEW_DEVICE_NOTIFICATIONS, n);

            ss->initialized = OS_TRUE;
        }
    }
}



static os_int ioc_make_new_device_notification_table(
    iocNotificationSignalRow *table,
    os_int max_rows,
    iocBServerNetwork *n)
{
    os_short nrows = 0;

#ifdef ACCOUNT_SIGNALS_EXP_NEW1_NAME_ARRAY_SZ
    table[nrows].user_name = &n->asignals.exp.new1_name;
    table[nrows].password = &n->asignals.exp.new1_password;
    table[nrows].privileges = &n->asignals.exp.new1_privileges;
    table[nrows].ip = &n->asignals.exp.new1_ip;
    table[nrows].count = &n->asignals.exp.new1_count;
    table[nrows].text = &n->asignals.exp.new1_text;
    if (nrows < max_rows) nrows++;
#endif

#ifdef ACCOUNT_SIGNALS_EXP_NEW2_NAME_ARRAY_SZ
    table[nrows].user_name = &n->asignals.exp.new2_name;
    table[nrows].password = &n->asignals.exp.new2_password;
    table[nrows].privileges = &n->asignals.exp.new2_privileges;
    table[nrows].ip = &n->asignals.exp.new2_ip;
    table[nrows].count = &n->asignals.exp.new2_count;
    table[nrows].text = &n->asignals.exp.new2_text;
    if (nrows < max_rows) nrows++;
#endif

    return nrows;
}
