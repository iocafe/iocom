/**

  @file    ioc_authorize.c
  @brief   User/device accounts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "ioserver.h"
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION




osalStatus ioc_authorize(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    iocUserAccount *user_account,
    void *context)
{
    return OSAL_SUCCESS;
    // return OSAL_STATUS_FAILED;
}


#endif
