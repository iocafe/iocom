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
   @return  OSAL_SUCCESS if message header succesfully read from ring buffer. If there
            is not enough data in, the function returns OSAL_PENDING.
 */
osalStatus ioc_switchbox_get_msg_header_from_ringbuf(
    osalRingBuf *r,
    os_short *client_id,
    os_int *data_len)
{
    os_uchar buf[SBOX_HDR_SIZE];
    os_uint u;
    os_int n;

    if (osal_ringbuf_bytes(r) < SBOX_HDR_SIZE) {
        return OSAL_PENDING;
    }

    n = osal_ringbuf_get(r, (os_char*)buf, SBOX_HDR_SIZE);
    osal_debug_assert(n == SBOX_HDR_SIZE);

    *client_id = ((os_ushort)buf[SBOX_HDR_CLIENT_ID_0] |
        ((os_ushort)buf[SBOX_HDR_CLIENT_ID_1]) << 8);

    u = (os_uint)buf[SBOX_HDR_DATA_LEN_0];
    u |= (((os_uint)buf[SBOX_HDR_DATA_LEN_1]) < 8);
    u |= (((os_uint)buf[SBOX_HDR_DATA_LEN_2]) < 16);
    u |= (((os_uint)buf[SBOX_HDR_DATA_LEN_3]) < 24);

    *data_len = (os_int)u;
    return OSAL_SUCCESS;
}


/* Save message header to ring buffer.
   @return  OSAL_SUCCESS if message header succesfully stored to ring buffer. If there
            is not enough space in ring buffer, the function returns OSAL_PENDING.
 */
osalStatus ioc_switchbox_store_msg_header_to_ringbuf(
    osalRingBuf *r,
    os_short client_id,
    os_int data_len)
{
    os_uchar buf[SBOX_HDR_SIZE];
    os_int n;

    if (osal_ringbuf_space(r) < SBOX_HDR_SIZE) {
        return OSAL_PENDING;
    }

    buf[SBOX_HDR_CLIENT_ID_0] = (os_uchar)client_id;
    buf[SBOX_HDR_CLIENT_ID_1] = (os_uchar)(client_id >> 8);
    buf[SBOX_HDR_DATA_LEN_0] = (os_uchar)data_len;
    buf[SBOX_HDR_DATA_LEN_1] = (os_uchar)(data_len >> 8);
    buf[SBOX_HDR_DATA_LEN_2] = (os_uchar)(data_len >> 16);
    buf[SBOX_HDR_DATA_LEN_3] = (os_uchar)(data_len >> 24);

    n = osal_ringbuf_put(r, (os_char*)buf, SBOX_HDR_SIZE);
    osal_debug_assert(n == SBOX_HDR_SIZE);
    return OSAL_SUCCESS;
}

#endif
