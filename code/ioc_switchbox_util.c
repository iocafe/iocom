/**

  @file    ioc_switchbox_util.c
  @brief   Helper functions for switchbox socket and switchbox extension.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_SWITCHBOX_SUPPORT

/* Get message header (client id and data length from ring buffer if it contains message.
 */
osalStatus ioc_switchbox_get_msg_header_from_ringbuf(
    osalRingBuf *r,
    os_short *client_id,
    os_int *data_len)
{
    return OSAL_PENDING;
}

#endif
