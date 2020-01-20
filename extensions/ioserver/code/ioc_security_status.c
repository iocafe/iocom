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
#include "ioserver.h"

/**
****************************************************************************************************

  @brief Store security note in exposed signals.
  @anchor ioc_secutiry_note

  The ioc_secutiry_note() function...

  @param   note Pointer to security note structure.
  @return  None.

****************************************************************************************************
*/
void ioc_secutiry_note(
    struct iocBServerMain *m,
    iocNoteCode code,
    iocSecurityNote *note)
{

}
