/**

  @file    ioc_accounts.c
  @brief   User/device accounts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION


/* Load user account configuration from persistent storage.
 */
void ioc_load_account_config(
    iocAccountConf *accounts,
    const os_char *default_config,
    os_memsz default_config_sz)
{

}


#endif
