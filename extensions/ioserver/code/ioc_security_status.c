/**

  @file    ioc_security_status.c
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
#define BSERVER_INTERNALS
#include "ioserver.h"

/* Forward referred static functions.
 */
static osalStatus ioc_security_notify2(
    struct iocBServer *m,
    iocNoteCode code,
    iocSecurityNotification *note,
    os_char *network_name);

static void ioc_set_notification(
    iocNotificationSignalRow *table,
    os_timer *timers,
    os_boolean *is_set,
    os_short nrows,
    iocNoteCode code,
    iocSecurityNotification *note,
    const os_char *text);

static void ioc_notifications_time_out(
    iocNotificationSignalRow *table,
    os_timer *timers,
    os_boolean *is_set,
    os_short nrows,
    os_int timeout_ms);

static os_short ioc_setup_new_device_notification_table(
    iocNotificationSignalRow *table,
    os_int max_rows,
    iocBServerNetwork *n);


/**
****************************************************************************************************

  @brief Share security notification (in iocom memory block).
  @anchor ioc_security_notify

  The ioc_security_notify() function...

  @param   m Pointer to basic server object.
  @param   code Reason for notification, like IOC_NOTE_NEW_IO_DEVICE,
           IOC_NOTE_WRONG_IO_DEVICE_PASSWORD.
  @param   note Pointer to security note structure. Contains network name, device/user name,
           password used to log in, etc.
  @return  None.

****************************************************************************************************
*/
void ioc_security_notify(
    struct iocBServer *m,
    iocNoteCode code,
    iocSecurityNotification *note)
{
    if (m->networks == OS_NULL) return;

    if (ioc_security_notify2(m, code, note, OS_NULL))
    {
        ioc_security_notify2(m, code, note, m->network_name);
    }
}


/**
****************************************************************************************************

  @brief Run security (make "new device" notifications time out)
  @anchor ioc_run_security

  The ioc_run_security() function...

  @param   m Pointer to basic server object.
  @return  None.

****************************************************************************************************
*/
void ioc_run_security(
    struct iocBServer *m)
{
    iocSecurityStatus *ss;
    os_int i;

    if (m->networks == OS_NULL) return;

    for (i = 0; i<m->nro_networks; i++)
    {
        ss = &m->networks[i].sec_status;

        ioc_notifications_time_out(ss->new_device, ss->new_device_timer,
            ss->new_device_is_set, ss->new_device_nrows, 8000);
    }
}


/**
****************************************************************************************************

  @brief Share security notification (in iocom memory block).
  @anchor ioc_security_notify

  The ioc_security_notify() function...

  @param   m Pointer to basic server object.
  @param   code Reason for notification, like IOC_NOTE_NEW_IO_DEVICE,
           IOC_NOTE_WRONG_IO_DEVICE_PASSWORD.
  @param   note Pointer to security note structure. Contains network name, device/user name,
           password used to log in, etc.
  @param   network_name Set network name to process for a specficic network, or OS_NULL to
           select network by network name in note.
  @return  If notification was processed, the function returns OSAL_SUCCESS. Other values indicate
           that it was not processed.

****************************************************************************************************
*/
static osalStatus ioc_security_notify2(
    struct iocBServer *m,
    iocNoteCode code,
    iocSecurityNotification *note,
    os_char *network_name)
{
    iocBServerNetwork *n;
    iocSecurityStatus *ss;
    os_char *text = OS_NULL;
    osalStatus s = OSAL_STATUS_FAILED;
    os_int i;

    for (i = 0; i<m->nro_networks; i++)
    {
        n = m->networks + i;
        if (os_strcmp(network_name ? network_name : note->network_name, n->network_name)) continue;
        s = OSAL_SUCCESS;

        /* Initialize signal pointers for "new devices" table.
         */
        ss = &n->sec_status;
        if (!ss->initialized)
        {
            ss->new_device_nrows = ioc_setup_new_device_notification_table(ss->new_device,
                IOC_MAX_NEW_DEVICE_NOTIFICATIONS, n);

            ss->initialized = OS_TRUE;
        }

        /* Set notification to be accessible from communication.
         */
        switch (code)
        {
            case IOC_NOTE_NEW_IO_DEVICE:
                text =  "NEW DEVICE";
                /* continues... */

            case IOC_NOTE_WRONG_IO_DEVICE_PASSWORD:
                if (text == OS_NULL) {
                    text = os_strcmp(note->password, osal_str_empty) ? "WRONG PASSWORD" : "NO PASSWORD";
                }
                ioc_set_notification(ss->new_device, ss->new_device_timer, ss->new_device_is_set,
                    ss->new_device_nrows, code, note, text);
                break;

            default:
                break;
        }
    }

    return s;
}


/**
****************************************************************************************************

  @brief Set up notification table (sets signal pointers).
  @anchor ioc_set_notification

  The ioc_set_notification() function...

  @param   table Pointer to notification table.
  @param   timers Timer for each notification table row.
  @param   is_set Array of "notification row has data" flags.
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
    os_boolean *is_set,
    os_short nrows,
    iocNoteCode code,
    iocSecurityNotification *note,
    const os_char *text)
{
    iocNotificationSignalRow *r;
    os_char buf[IOC_NAME_SZ];
    os_short row, empty_row;
    os_boolean reset_count = OS_FALSE;
    os_long count = 1;

    /* If we have row for this user/device already, just update it.
     * Otherwise select empty row, if any.
     */
    empty_row = -1;
    for (row = 0; row < nrows; row++)
    {
        ioc_get_str(table[row].user_name, buf, sizeof(buf));
        if (!os_strcmp(buf, note->user)) goto row_selected;
        if (buf[0] == '\0' && empty_row == -1) empty_row = row;
    }
    reset_count = OS_TRUE;
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
    ioc_set_str(r->user_name, note->user);
    ioc_set_str(r->password, note->password);
    ioc_set_str(r->privileges, note->privileges);
    ioc_set_str(r->ip, note->ip);
    if (!reset_count)
    {
        count = ioc_get(r->count) + 1;
    }
    ioc_set(r->count, count);
    ioc_set_str(r->text, text);
    os_get_timer(&timers[row]);
    is_set[row] = OS_TRUE;

    ioc_send(r->user_name->handle);
}



/**
****************************************************************************************************

  @brief Make notifications to time out.
  @anchor ioc_notifications_time_out

  The ioc_notifications_time_out() function...

  @param   table Pointer to notification table.
  @param   timers Timer for each notification table row.
  @param   is_set Array of "notification row has data" flags.
  @param   nrows Number of valid row structures in table.
  @param   timeout_ms How long for notifications to time out.
  @return  None.

****************************************************************************************************
*/
static void ioc_notifications_time_out(
    iocNotificationSignalRow *table,
    os_timer *timers,
    os_boolean *is_set,
    os_short nrows,
    os_int timeout_ms)
{
    iocNotificationSignalRow *r = OS_NULL;
    os_timer now_t;
    os_short row;

    /* If we have row for this user/device already, just update it.
     * Otherwise select empty row, if any.
     */
    os_get_timer(&now_t);
    for (row = 0; row < nrows; row++)
    {
        if (!is_set[row]) continue;
        if (!os_has_elapsed_since(&timers[row], &now_t, timeout_ms)) continue;

        r = table + row;
        ioc_move_str(r->password, OS_NULL, -1,  0, IOC_SIGNAL_WRITE|OS_STR);
        ioc_move_str(r->privileges, OS_NULL, -1,  0, IOC_SIGNAL_WRITE|OS_STR);
        ioc_move_str(r->ip, OS_NULL, -1,  0, IOC_SIGNAL_WRITE|OS_STR);
        ioc_move_str(r->text, OS_NULL, -1,  0, IOC_SIGNAL_WRITE|OS_STR);
        ioc_set_ext(r->count, 0, 0);
        ioc_move_str(r->user_name, OS_NULL, -1,  0, IOC_SIGNAL_WRITE|OS_STR);
        is_set[row] = OS_FALSE;
    }

    if (r)
    {
        ioc_send(r->user_name->handle);
    }
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
