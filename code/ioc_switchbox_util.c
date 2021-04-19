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

/**
****************************************************************************************************

  @brief Get message header (client id and data length) from ring buffer.
  @anchor ioc_switchbox_get_msg_header_from_ringbuf

  This function is retrieves message header (client id and data length) from incoming ring buffer
  of a shared socket. If there is no complete client header, function returns OSAL_PENDING.

  @param   r Source ring buffer, typically incoming ring buffer of shared socket.
  @param   client_id Switchbox client connection identifier. Used to separate messages to/from
           clients.
  @param   data_len Data length to follow in bytes, or control code like
           IOC_SWITCHBOX_NEW_CONNECTION or IOC_SWITCHBOX_CONNECTION_DROPPED.

  @return  OSAL_SUCCESS if message header succesfully read from ring buffer. If there
           is not enough data in, the function returns OSAL_PENDING. Other nonzero
           return values indicate corrupted message header.

****************************************************************************************************
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


/**
****************************************************************************************************

  @brief Save message header into ring buffer.
  @anchor ioc_switchbox_store_msg_header_to_ringbuf

  This function is used to store message header with client id and data length into outgoing
  ring buffer of shared socket.

  @param   r Destination ring buffer, typically outgoing ring buffer of shared socket.
  @param   client_id Switchbox client connection identifier. Used to separate messages to/from
           clients.
  @param   data_len Data length to follow in bytes, or control code like
           IOC_SWITCHBOX_NEW_CONNECTION or IOC_SWITCHBOX_CONNECTION_DROPPED.

  @return  OSAL_SUCCESS if message header succesfully stored to ring buffer. If there
           is not enough space in ring buffer, the function returns OSAL_PENDING.

****************************************************************************************************
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


/**
****************************************************************************************************

  @brief Move n bytes from source ring buffer to destination ring buffer.
  @anchor ioc_switchbox_ringbuf_move

  This function checks number of bytes available in source buffer and free space in destination
  buffers and limits number of bytes moved within those constraints.

  @param   dst_r Destination ring buffer.
  @param   src_r Destination ring buffer.
  @return  Number of bytes moved. This may be less than argument n if source ring buffer doesn't
           hold n data bytes, or there is no free space for n bytes in destination buffer.

****************************************************************************************************
*/
os_int ioc_switchbox_ringbuf_move(
    osalRingBuf *dst_r,
    osalRingBuf *src_r,
    os_int n)
{
    os_int k, head, tail, n_left, smaller;

    k = osal_ringbuf_bytes(src_r);
    if (k < n) {
        n = k;
    }
    k = osal_ringbuf_space(dst_r);
    if (k < n) {
        n = k;
    }
    if (n > 0) {
        head = dst_r->head;
        tail = src_r->tail;
        n_left = n;

        do {
            smaller = n_left;
            k = dst_r->buf_sz - head;
            if (k < smaller) {
                smaller = k;
            }
            k = src_r->buf_sz - tail;
            if (k < smaller) {
                smaller = k;
            }

            os_memcpy(dst_r->buf + head, src_r->buf + tail, smaller);
            head += smaller;
            if (head >= dst_r->buf_sz) {
                head = 0;
            }
            tail += smaller;
            if (tail >= src_r->buf_sz) {
                tail = 0;
            }
            n_left -= smaller;
        }
        while (n_left > 0);

        dst_r->head = head;
        src_r->tail = tail;
    }

    return n;
}

#endif
