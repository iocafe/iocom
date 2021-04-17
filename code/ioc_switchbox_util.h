/**

  @file    ioc_switchbox_util.h
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
#pragma once
#ifndef IOC_SWITCHBOX_UTIL_H_
#define IOC_SWITCHBOX_UTIL_H_
#include "iocom.h"
#if IOC_SWITCHBOX_SUPPORT

/* Get message header from ring buffer (get client id and data length if whole header is in buffer).
 */
osalStatus ioc_switchbox_get_msg_header_from_ringbuf(
    osalRingBuf *r,
    os_short *client_id,
    os_int *data_len);

/* Save message header to ring buffer .
 */
osalStatus ioc_switchbox_store_msg_header_to_ringbuf(
    osalRingBuf *r,
    os_short client_id,
    os_int data_len);

/* Move n bytes from source ring buffer to destination ring buffer.
 */
os_int ioc_switchbox_ringbuf_move(
    osalRingBuf *dst_r,
    osalRingBuf *src_r,
    os_int n);

#endif
#endif
