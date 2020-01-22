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
    os_timer *timers,
    os_short nrows,
    iocNoteCode code,
    iocSecurityNotification *note,
    const os_char *text);

static os_short ioc_setup_new_device_notification_table(
    iocNotificationSignalRow *table,
    os_int max_rows,
    iocBServerNetwork *n);

static os_short ioc_setup_alarm_notification_table(
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
    iocSecurityStatus *ss;
    os_char *text;
    os_int i;

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
            ss->new_device_nrows = ioc_setup_new_device_notification_table(ss->new_device,
                IOC_MAX_NEW_DEVICE_NOTIFICATIONS, n);

            ss->alarm_nrows = ioc_setup_alarm_notification_table(ss->alarm,
                IOC_MAX_ALARM_NOTIFICATIONS, n);

            ss->initialized = OS_TRUE;
        }

        /* Set notification to be accessible from communication.
         */
        switch (code)
        {
            case IOC_NOTE_NEW_IO_DEVICE:
            case IOC_NOTE_WRONG_IO_DEVICE_PASSWORD:
                text = (code == IOC_NOTE_NEW_IO_DEVICE) ? "NEW DEVICE" : "WRONG PASSWORD";
                ioc_set_notification(ss->new_device, ss->new_device_timer,
                    ss->new_device_nrows, code, note, text);
                break;

            default:
            case IOC_NOTE_BLACKLISTED_IO_DEVICE:
                text = (code == IOC_NOTE_BLACKLISTED_IO_DEVICE) ? "BLACKLISTED" : "UNKNOWN ALARM";
                ioc_set_notification(ss->alarm, ss->alarm_timer,
                    ss->alarm_nrows, code, note, text);
                break;
        }
    }
}


/**
****************************************************************************************************

  @brief Set up notification table (sets signal pointers).
  @anchor ioc_setup_new_device_notification_table

  The ioc_set_notification() function...

  @param   table Pointer to notification table.
  @param   timers Timer for each notification table row.
  @param   nrows Number of valid row structures in table.
  @param   note Pointer to security note structure. Contains network name, device/user name,
           password used to log in, etc.
  @param   text Explanation why we are here.
  @return  None.

****************************************************************************************************
*/
static void ioc_set_notification(
    iocNotificationSignalRow *table,
    os_timer *timers,
    os_short nrows,
    iocNoteCode code,
    iocSecurityNotification *note,
    const os_char *text)
{
    iocNotificationSignalRow *r;
    os_char buf[IOC_NAME_SZ];
    os_short row, empty_row;

    /* If we have row for this user/device already, just update it.
     * Otherwise select empty row, if any.
     */
    empty_row = -1;
    for (row = 0; row < nrows; row++)
    {
        ioc_gets_str(table[row].user_name, buf, sizeof(buf));
        if (!os_strcmp(buf, note->user)) goto row_selected;
        if (buf[0] == '\0' && empty_row == -1) empty_row = row;
    }
    if (empty_row >= 0)
    {
        row = empty_row;
        goto row_selected;
    }

    /* If no empty rows, select the last row.
     */
    row = nrows - 1;

row_selected:
    r = table + row;
    ioc_sets_str(r->user_name, note->user);
    ioc_sets_str(r->password, note->password);
    ioc_sets_str(r->privileges, note->privileges);
    ioc_sets_str(r->ip, note->ip);
    ioc_sets0_int(r->count, 1);
    ioc_sets_str(r->text, text);
    os_get_timer(&timers[row]);

    ioc_send(r->user_name->handle);
}


/**
****************************************************************************************************

  @brief Set up notification table (sets signal pointers).
  @anchor ioc_setup_new_device_notification_table

  The ioc_setup_new_device_notification_table() function...

  @param   table Pointer to notification table, array or row signal structures to set up.
  @param   max_rows Number of row structures allocated in table.
  @param   n Basic server network structure pointer. Holds signal structure.
  @return  None.

****************************************************************************************************
*/
static os_short ioc_setup_new_device_notification_table(
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


/**
****************************************************************************************************

  @brief Set up alarm notification table (sets signal pointers).
  @anchor ioc_setup_alarm_notification_table

  The ioc_setup_alarm_notification_table() function...

  @param   table Pointer to notification table, array or row signal structures to set up.
  @param   max_rows Number of row structures allocated in table.
  @param   n Basic server network structure pointer. Holds signal structure.
  @return  None.

****************************************************************************************************
*/
static os_short ioc_setup_alarm_notification_table(
    iocNotificationSignalRow *table,
    os_int max_rows,
    iocBServerNetwork *n)
{
    os_short nrows = 0;

#ifdef ACCOUNT_SIGNALS_EXP_ALARM1_NAME_ARRAY_SZ
    table[nrows].user_name = &n->asignals.exp.alarm1_name;
    table[nrows].password = &n->asignals.exp.alarm1_password;
    table[nrows].privileges = &n->asignals.exp.alarm1_privileges;
    table[nrows].ip = &n->asignals.exp.alarm1_ip;
    table[nrows].count = &n->asignals.exp.alarm1_count;
    table[nrows].text = &n->asignals.exp.alarm1_text;
    if (nrows < max_rows) nrows++;
#endif

#ifdef ACCOUNT_SIGNALS_EXP_ALARM2_NAME_ARRAY_SZ
    table[nrows].user_name = &n->asignals.exp.alarm2_name;
    table[nrows].password = &n->asignals.exp.alarm2_password;
    table[nrows].privileges = &n->asignals.exp.alarm2_privileges;
    table[nrows].ip = &n->asignals.exp.alarm2_ip;
    table[nrows].count = &n->asignals.exp.alarm2_count;
    table[nrows].text = &n->asignals.exp.alarm2_text;
    if (nrows < max_rows) nrows++;
#endif

    return nrows;
}
