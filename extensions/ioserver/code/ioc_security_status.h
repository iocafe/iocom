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

struct iocBServerMain;

typedef enum
{
    IOC_NOTE_NEW_IO_DEVICE,
    IOC_NOTE_WRONG_IO_DEVICE_PASSWORD,
    IOC_NOTE_BLACKLISTED_IO_DEVICE,
}
iocNoteCode;


typedef struct iocSecurityNote
{
    os_char *user;
    os_char *password;
    os_char *ip;
    os_char *text;
    os_char *network_name;
}
iocSecurityNote;

typedef struct iocSecurityStatus
{
    os_int dummy;
}
iocSecurityStatus;


void ioc_secutiry_note(
    struct iocBServerMain *m,
    iocNoteCode code,
    iocSecurityNote *note);

