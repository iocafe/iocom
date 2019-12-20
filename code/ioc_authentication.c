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

  @brief Make frame containing memory block information.
  @anchor ioc_make_mblk_info_frame

  The ioc_make_mblk_info_frame() function...

  @param   con Pointer to the connection object.
  @param   mblk Pointer to memory block whose information to send.
  @return  OSAL_SUCCESS if data was sent. OSAL_STATUS_PENDING if nothing was sent, but all is fine.
           Other values indicate broken connection error.

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
        *start,
        *version_and_flags;

    os_int
        content_bytes,
        used_bytes;

    os_uint
        device_nr;

    os_int
        bytes;

    os_char
        buf[3*IOC_NAME_SZ + IOC_NETWORK_NAME_SZ];

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
    version_and_flags = p; /* version, for future additions + flags */
    *(p++) = 0;



    /* Set device number. If we are sending to device with automatically number, set zero.
     */
    device_nr = root->device_nr;
    if (device_nr > IOC_AUTO_DEVICE_NR) device_nr = 0;

    ioc_msg_set_uint(device_nr, &p, version_and_flags, IOC_INFO_D_2BYTES, IOC_INFO_D_4BYTES);

#if 0
    if (ioc_msg_set_ushort(mblk->nbytes, &p)) *version_and_flags |= IOC_INFO_N_2BYTES;
    if (ioc_msg_set_ushort(mblk->flags, &p)) *version_and_flags |= IOC_INFO_F_2BYTES;
    if (mblk->device_name[0])
    {
        ioc_msg_setstr(mblk->device_name, &p);
        ioc_msg_setstr(mblk->network_name, &p);
        *version_and_flags |= IOC_INFO_HAS_DEVICE_NAME;
    }
    if (mblk->mblk_name[0] || mblk->network_name[0])
    {
        ioc_msg_setstr(mblk->mblk_name, &p);
        *version_and_flags |= IOC_INFO_HAS_MBLK_NAME;
    }

    /* If other end has not acknowledged enough data to send the
       frame, cancel the send.
     */
    content_bytes = (os_int)(p - start);
    used_bytes = content_bytes + ptrs.header_sz;
    u = con->bytes_sent - con->processed_bytes;
    bytes = con->max_in_air - (os_int)u;
    if (used_bytes > bytes)
    {
        osal_trace2_int("MBLK info canceled by flow control, free space on air=", bytes);
        return;
    }

    /* Fill in data size and flag as system frame.
     */
    *ptrs.data_sz_low = (os_uchar)content_bytes;
    if (ptrs.data_sz_high)
    {
        content_bytes >>= 8;
        *ptrs.data_sz_high = (os_uchar)content_bytes;
    }
    con->frame_out.used = used_bytes;
    *ptrs.flags |= IOC_SYSTEM_FRAME;

#endif

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
