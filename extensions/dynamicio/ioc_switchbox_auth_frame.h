/**


  @file    ioc_switchbox_auth_frame.h
  @brief   Device/user authentication for switchbox and ecom.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Low level handling of authentication frames for ecom and switchbox communication. The base
  iocom library contains it's own authentication frame related code, this implementation is
  intended for switchbox and ecom, to use interchangable IOCOM compatible authentication frames.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_SWITCHBOX_AUTH_FRAME_H_
#define IOC_SWITCHBOX_AUTH_FRAME_H_
#include "iocom.h"

#if IOC_DYNAMIC_MBLK_CODE


#define IOC_MAX_AUTHENTICATION_FRAME_SZ 256

typedef struct iocSwitchboxAuthenticationFrameBuffer {
    os_char buf[IOC_MAX_AUTHENTICATION_FRAME_SZ];
    os_short buf_used;
    os_short buf_pos;
}
iocSwitchboxAuthenticationFrameBuffer;


typedef struct iocSwitchboxAuthenticationParameters {
    os_char *network_name;
    os_char *user_name;
    os_char *password;
}
iocSwitchboxAuthenticationParameters;


typedef struct iocAuthenticationResults {
    int ulle;
}
iocAuthenticationResults;

/* Send switchbox/ecom authentication frame to stream.
 */
osalStatus icom_switchbox_send_authentication_frame(
    osalStream stream,
    iocSwitchboxAuthenticationFrameBuffer *abuf,
    iocSwitchboxAuthenticationParameters *prm);

/* Receive and process swtchbox/ecom authentication frame from stream.
 */
osalStatus icom_switchbox_process_authentication_frame(
    osalStream stream,
    iocSwitchboxAuthenticationFrameBuffer *abuf,
    iocAuthenticationResults *results);

#endif
#endif
