/**

  @file    ioc_authentication.c
  @brief   Connection object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.12.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_AUTHENTICATION

/* Forward referred static functions.
 */


/**
****************************************************************************************************

  @brief Make authentication data frame.
  @anchor ioc_make_authentication_frame

  The ioc_make_authentication_frame() generates ourgoind data frame which contains information
  to authenticate this IO device, etc.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_make_authentication_frame(
    iocConnection *con)
{
    iocRoot
        *root;

    iocSendHeaderPtrs
        ptrs;

    os_uchar
        *p,
        *auth_flags_ptr,
        flags,
        *start;

    os_char
        *password;

    os_int
        content_bytes,
        used_bytes;

    os_uint
        device_nr;

    os_int
        bytes;

    os_ushort
        crc,
        u;

    root = con->link.root;

    /* Set frame header.
     */
    ioc_generate_header(con, con->frame_out.buf, &ptrs, 0, 0);

    /* Generate frame content. Here we do not check for buffer overflow,
       we know (and trust) that it fits within one frame.
     */
    p = start = (os_uchar*)con->frame_out.buf + ptrs.header_sz;
    *(p++) = IOC_AUTHENTICATION_DATA;
    flags = 0;
    auth_flags_ptr = p;
    *(p++) = 0;

    if (root->device_name[0] != 0)
    {
        ioc_msg_setstr(root->device_name, &p);
        flags |= IOC_AUTH_DEVICE;

        ioc_msg_set_uint(root->device_nr < IOC_AUTO_DEVICE_NR ? root->device_nr : 0,
            &p, &flags, IOC_AUTH_DEVICE_NR_2_BYTES, IOC_AUTH_DEVICE_NR_4_BYTES);
    }

    if (root->network_name[0] != 0)
    {
        ioc_msg_setstr(root->network_name, &p);
        flags |= IOC_AUTH_NETWORK_NAME;
    }

    password = (con->iface == OSAL_TLS_IFACE)
        ? root->password_tls : root->password_clear;
    if (password[0] != 0)
    {
        ioc_msg_setstr(password, &p);
        flags |= IOC_AUTH_PASSWORD;
    }

    /* Set device number. If we are sending to device with automatically number, set zero.
     */
    device_nr = root->device_nr;
    if (device_nr > IOC_AUTO_DEVICE_NR) device_nr = 0;

    /* If other end has not acknowledged enough data to send the
       frame, cancel the send.
     */
    content_bytes = (os_int)(p - start);
    used_bytes = content_bytes + ptrs.header_sz;
    u = con->bytes_sent - con->processed_bytes;
    bytes = con->max_in_air - (os_int)u;
    if (used_bytes > bytes)
    {
        osal_trace2_int("Authentication canceled by flow control, free space on air=", bytes);
        return;
    }

    /* Set connect upward flag.
     */
    if (con->flags & IOC_CONNECT_UPWARDS)
    {
        flags |= IOC_AUTH_CONNECT_UPWARDS;
    }

    /* Fill in data size, flag as system frame, and flags for authentication data.
     */
    *ptrs.data_sz_low = (os_uchar)content_bytes;
    if (ptrs.data_sz_high)
    {
        content_bytes >>= 8;
        *ptrs.data_sz_high = (os_uchar)content_bytes;
    }
    con->frame_out.used = used_bytes;
    *ptrs.flags |= IOC_SYSTEM_FRAME;
    *auth_flags_ptr = flags;

    /* Frame not rejected by flow control, increment frame number.
     */
    if (con->frame_out.frame_nr++ >= IOC_MAX_FRAME_NR)
    {
        con->frame_out.frame_nr = 1;
    }

    /* Calculate check sum out of whole used frame buffer. Notice that check sum
     * position within frame is zeros when calculating the check sum.
     */
    if (ptrs.checksum_low)
    {
        crc = os_checksum(con->frame_out.buf, con->frame_out.used, OS_NULL);
        *ptrs.checksum_low = (os_uchar)crc;
        *ptrs.checksum_high = (os_uchar)(crc >> 8);
    }

    con->authentication_sent = OS_TRUE;
}


#endif
