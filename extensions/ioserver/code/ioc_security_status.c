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

/* Forward referred static functions.
 */
static void ioc_set_notification(
    iocNotificationSignalRow *table,
    os_short nrows,
    iocNoteCode code,
    iocSecurityNotification *note);

static os_short ioc_setup_notification_table(
    iocNotificationSignalRow *table,
    os_int max_rows,
    iocBServerNetwork *n);


/**
****************************************************************************************************

  @brief Share security notification (in iocom memory block).
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
            ss->new_device_nrows = ioc_setup_notification_table(ss->new_device,
                IOC_MAX_NEW_DEVICE_NOTIFICATIONS, n);

            ss->initialized = OS_TRUE;
        }

        /* Set notification to be accessible from communication.
         */
        ioc_set_notification(ss->new_device, ss->new_device_nrows, code, note);
    }
}


/**
****************************************************************************************************

  @brief Set up notification table (sets signal pointers).
  @anchor ioc_setup_notification_table

  The ioc_set_notification() function...

  @param   table Pointer to notification table.
  @param   nrows Number of valid row structures in table.
  @param   note Pointer to security note structure. Contains network name, device/user name,
           password used to log in, etc.
  @return  None.

****************************************************************************************************
*/
static void ioc_set_notification(
    iocNotificationSignalRow *table,
    os_short nrows,
    iocNoteCode code,
    iocSecurityNotification *note)
{
    iocNotificationSignalRow *r;
    os_timer t_now;
    os_short row;

    /* If we have row for this device already, update it
     */

    /* Select empty row. If no empty rows, select the oldest
     */

    row = 0;
    r = table + row;
    ioc_sets_str(r->user_name, note->user);
    ioc_sets_str(r->password, note->password);
    ioc_sets_str(r->privileges, note->privileges);
    ioc_sets_str(r->ip, note->ip);
    ioc_sets0_int(r->count, 1);
    os_get_timer(&t_now);
    ioc_sets0_int(r->timer, t_now);
    ioc_sets_str(r->text, note->text);

    ioc_send(r->user_name->handle);
}


/**
****************************************************************************************************

  @brief Set up notification table (sets signal pointers).
  @anchor ioc_setup_notification_table

  The ioc_setup_notification_table() function...

  @param   table Pointer to notification table, array or row signal structures to set up.
  @param   max_rows Number of row structures allocated in table.
  @param   n Basic server network structure pointer. Holds signal structure.
  @return  None.

****************************************************************************************************
*/
static os_short ioc_setup_notification_table(
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
    table[nrows].timer = &n->asignals.exp.new1_timer;
    table[nrows].text = &n->asignals.exp.new1_text;
    if (nrows < max_rows) nrows++;
#endif

#ifdef ACCOUNT_SIGNALS_EXP_NEW2_NAME_ARRAY_SZ
    table[nrows].user_name = &n->asignals.exp.new2_name;
    table[nrows].password = &n->asignals.exp.new2_password;
    table[nrows].privileges = &n->asignals.exp.new2_privileges;
    table[nrows].ip = &n->asignals.exp.new2_ip;
    table[nrows].count = &n->asignals.exp.new2_count;
    table[nrows].timer = &n->asignals.exp.new2_timer;
    table[nrows].text = &n->asignals.exp.new2_text;
    if (nrows < max_rows) nrows++;
#endif

    return nrows;
}
