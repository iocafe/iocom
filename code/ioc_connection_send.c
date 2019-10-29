/**

  @file    ioc_connection_send.c
  @brief   Send data to connection.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.8.2018

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/* Pointers to modify generated header afterwards.
 */
typedef struct
{
    /** Pointer to check sum in header
     */
    os_char *checksum_low;
    os_char *checksum_high;

    /** Pointer to flags
     */
    os_char *flags;

    /** Pointers to data size in bytes
     */
    os_char *data_sz_low;
    os_char *data_sz_high;

    /* Header size in bytes.
     */
    int header_sz;
}
iocSendHeaderPtrs;


/* Forward referred static functions.
 */
static void ioc_make_data_frame(
    iocConnection *con,
    iocSourceBuffer *sbuf);

static osalStatus ioc_write_to_stream(
    iocConnection *con);

static void ioc_make_mblk_info_frame(
    iocConnection *con,
    iocMemoryBlock *mblk);

static void ioc_generate_header(
    iocConnection *con,
    os_char *hdr,
    iocSendHeaderPtrs *ptrs,
    int remote_mblk_id,
    os_uint addr);

static void ioc_msg_setstr(
    os_char *str,
    os_char **p);

static os_boolean ioc_msg_setint(
    os_ushort i,
    os_char **p);


/**
****************************************************************************************************

  @brief Select source buffer and send a frame it.
  @anchor ioc_connection_send

  The ioc_connection_send() function selects a source buffer from which frame is to be sent
  and calls ioc_send_frame to send it.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all data was sent. OSAL_STATUS_PENDING if nothing or part of data
           was sent. Other values indicate broken connection error.

****************************************************************************************************
*/
osalStatus ioc_connection_send(
    iocConnection *con)
{
    IOC_MT_ROOT_PTR;

    iocMemoryBlock 
        *mblk;

    iocSourceBuffer
        *start_sbuf,
        *sbuf;

    osalStatus
        status = OSAL_STATUS_PENDING;

    ioc_set_mt_root(root, con->link.root);
    ioc_lock(root);

    /* If there is unsent or partly sent message in frame buffer,
       we can not place new message into it.
     */
    if (con->frame_out.used) 
    {
        goto just_move_data;
    }

    /* Is there received data to be acknowledged? (more than N 
       unacknowledged bytes, N = relax not ot acknowledge every
       small message separately)
     */
    status = ioc_acknowledge_as_needed(con);
    switch (status)
    {
        case OSAL_SUCCESS:
            break;

        case OSAL_STATUS_PENDING:
            goto just_move_data;

        default:
            ioc_unlock(root);
            return OSAL_STATUS_FAILED;
    }

    /* Did we send whole acknowledge? If not, return OSAL_STATUS_PENDING
     */
    if (con->frame_out.used) 
    {
        ioc_unlock(root);
        return OSAL_STATUS_PENDING;
    }
    
    /* Do we have memory block information to send?
     */
    mblk = ioc_get_mbinfo_to_send(con);
    if (mblk)
    {
        ioc_make_mblk_info_frame(con, mblk);
        goto just_move_data;
    }

    start_sbuf = con->sbuf.current ? con->sbuf.current : con->sbuf.first;
    if (start_sbuf == OS_NULL) goto just_move_data;

    /* Find out source buffer which has modified data.
     */
    sbuf = start_sbuf;
    while (!sbuf->syncbuf.used)
    {
        sbuf = sbuf->clink.next;
        if (sbuf == OS_NULL) sbuf = con->sbuf.first;
        if (sbuf == start_sbuf) goto just_move_data;
    }
    con->sbuf.current = sbuf->clink.next;

    /* Move data from source buffer to frame buffer.
       This compresses the data. All data may not
       fit into frame buffer at once, thus source buffer
       may not be empty.
     */
    ioc_make_data_frame(con, sbuf);

just_move_data:
    /* Send data from frame buffer to socket.
     */
    status = ioc_write_to_stream(con);
    ioc_unlock(root);
    return status;
}


/**
****************************************************************************************************

  @brief Create a data frame to send.
  @anchor ioc_make_data_frame

  The ioc_make_data_frame() function creates a data frame ready for sending.

  @param   con Pointer to the connection object.
  @param   sbuf Source buffer.
  @return  None

****************************************************************************************************
*/
static void ioc_make_data_frame(
    iocConnection *con,
    iocSourceBuffer *sbuf)
{
    int
        compressed_bytes,
        saved_start_addr,
        max_dst_bytes,
        src_bytes,
        start_addr,
        used_bytes;

    os_int
        bytes;

    os_char
        *dst,
        *delta;

    iocSendHeaderPtrs
        ptrs;

    os_ushort
        crc,
        u;

    /* Set frame header
     */
    ioc_generate_header(con, con->frame_out.buf, &ptrs,
        sbuf->remote_mblk_id,
        (os_uint)sbuf->syncbuf.start_addr);

    delta = sbuf->syncbuf.delta;
    saved_start_addr = sbuf->syncbuf.start_addr;
    max_dst_bytes = con->frame_sz - ptrs.header_sz;
    dst = con->frame_out.buf + ptrs.header_sz;

    /* Compress data from synchronized buffer. Save start addr in case
       we need to cancel send because of flow control.
     */
    con->frame_out.pos = 0;
    start_addr = sbuf->syncbuf.start_addr;
    if (delta == OS_NULL) /* IOC_STATIC -> delta=0 */
    {
        delta = sbuf->mlink.mblk->buf;
    }
    else if (!sbuf->syncbuf.is_keyframe)
    {
        *ptrs.flags |= IOC_DELTA_ENCODED;
    }
    compressed_bytes = ioc_compress(delta,
        &start_addr,
        sbuf->syncbuf.end_addr,
        dst, max_dst_bytes);

    src_bytes = sbuf->syncbuf.end_addr - saved_start_addr + 1;
    if (src_bytes > max_dst_bytes) src_bytes = max_dst_bytes;
    used_bytes = (compressed_bytes < 0 ? src_bytes : compressed_bytes) 
        + ptrs.header_sz;

    /* If other end has not acknowledged enough data to send the
       frame, cancel the send.
     */
    u = con->bytes_sent - con->processed_bytes; 
    bytes = con->max_in_air - (os_int)u;
    if (used_bytes > bytes)
    {
        osal_trace2_int("Data frame canceled by flow control, free space on air=", bytes);
        return;
    }

    sbuf->syncbuf.start_addr = start_addr;

    /* Frame not rejected by flow control, increment frame number.
     */
    if (con->frame_out.frame_nr++ >= IOC_MAX_FRAME_NR)
    {
        con->frame_out.frame_nr = 1;
    }

    if (compressed_bytes < 0)
    {
        os_memcpy(dst, delta + saved_start_addr, src_bytes);
        sbuf->syncbuf.start_addr += src_bytes;
    }
    else
    {
        *ptrs.flags |= IOC_COMPRESESSED;
    }
    con->frame_out.used = used_bytes;

    src_bytes = con->frame_out.used - ptrs.header_sz;
    *ptrs.data_sz_low = (os_uchar)src_bytes;
    if (ptrs.data_sz_high)
    {
        src_bytes >>= 8;
        *ptrs.data_sz_high = (os_uchar)src_bytes;
    }

    /* If we completed syncronization.
     */
    if (sbuf->syncbuf.start_addr > sbuf->syncbuf.end_addr)
    {
        sbuf->syncbuf.used = OS_FALSE;
        *ptrs.flags |= IOC_SYNC_COMPLETE;
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
}


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
static void ioc_make_mblk_info_frame(
    iocConnection *con,
    iocMemoryBlock *mblk)
{
    iocSendHeaderPtrs
        ptrs;

    os_char
        *p,
        *start,
        *version_and_flags;

    int
        content_bytes,
        used_bytes;

    os_int
        bytes;

    os_ushort
        crc,
        u;

    /* Set frame header.
     */
    ioc_generate_header(con, con->frame_out.buf, &ptrs,
        mblk->mblk_id, mblk->mblk_nr);

    /* Generate frame content. Here we do not check for buffer overflow,
       we know (and trust) that it fits within one frame.
     */
    p = start = con->frame_out.buf + ptrs.header_sz;
    *(p++) = IOC_SYSRAME_MBLK_INFO; 
    version_and_flags = p; /* version, for future additions + flags */
    *(p++) = 0;
    if (ioc_msg_setint(mblk->device_nr, &p)) *version_and_flags |= IOC_INFO_D_2BYTES;
    /* ioc_msg_setint(mblk->mblk_nr, &p);
    ioc_msg_setint(mblk->mblk_id, &p); */
    if (ioc_msg_setint(mblk->nbytes, &p)) *version_and_flags |= IOC_INFO_N_2BYTES;
    if (ioc_msg_setint(mblk->flags, &p)) *version_and_flags |= IOC_INFO_F_2BYTES;
    if (mblk->device_name[0])
    {
        ioc_msg_setstr(mblk->device_name, &p);
        *version_and_flags |= IOC_INFO_HAS_DNAME;
    }
    if (mblk->mblk_name[0])
    {
        ioc_msg_setstr(mblk->mblk_name, &p);
        *version_and_flags |= IOC_INFO_HAS_MBNAME;
    }

    /* If other end has not acknowledged enough data to send the
       frame, cancel the send.
     */
    content_bytes = (int)(p - start);
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
    con->frame_out.used = used_bytes;
    *ptrs.flags |= IOC_SYSTEM_FRAME;

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

    /* Memory block succesfully placed to frame out buffer, now we can forget about it.
     */
    ioc_mbinfo_sent(con, mblk);
}


/**
****************************************************************************************************

  @brief Send keep alive or acknowledge.
  @anchor ioc_send_acknowledge

  The ioc_send_acknowledge() function sends an acknowledge message to socket or serial port.
  The acknowledge message has dual purpose: First is flow control. Number of received and processed
  bytes is sent to another end so that it knwos it can send more data. Second is keep alive.
  If there is no events, no data to transfer, line is silent. To know that we are still
  connected, and acknowledge message is sent as keep alive when there has been nothing else
  to send for a while.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all data was sent. OSAL_STATUS_PENDING if nothing or part of data
           was sent. Other values indicate broken connection error.

****************************************************************************************************
*/
osalStatus ioc_send_acknowledge(
    iocConnection *con)
{
    osalStatus
        status;

    IOC_MT_ROOT_PTR;

    os_char
        *p;

    os_ushort
        rbytes;

    ioc_set_mt_root(root, con->link.root);
    ioc_lock(root);

    /* If frame buffer is used, we can do nothing.
     */
    if (con->frame_out.used) 
    {
        ioc_unlock(root);
        return OSAL_STATUS_PENDING;
    }

    /* Generate acknowledge/keepalive message
     */
    p = con->frame_out.buf;
    *(p++) = IOC_ACKNOWLEDGE; 
    rbytes = con->bytes_received;
    *(p++) = (os_uchar)rbytes;
    *p = (os_uchar)(rbytes >> 8);
    con->frame_out.used = IOC_ACK_SIZE;
    con->bytes_acknowledged = rbytes;

    status = ioc_write_to_stream(con);
    os_get_timer(&con->last_send);
 
    ioc_unlock(root);
    return status;
}


/**
****************************************************************************************************

  @brief Send keep alive frame.
  @anchor ioc_send_timed_keepalive

  The ioc_send_timed_keepalive() function sends acknowledge/keep alive message if nothing has
  been sent for a while.

  @param   con Pointer to the connection object.
  @param   tnow Timer now.

  @return  OSAL_SUCCESS if control frame was sent. Other values indicate broken connection error.

****************************************************************************************************
*/
osalStatus ioc_send_timed_keepalive(
    iocConnection *con,
    os_timer *tnow)
{
    osalStatus
        status;

    os_boolean
        timed_keepalive,
        is_serial;

    /* In serial communication, we do not send keep alives until connection is established.
     */
    is_serial = (os_boolean)((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL);
    if (is_serial && con->sercon_state != OSAL_SERCON_STATE_CONNECTED_5)
    {
        return OSAL_SUCCESS;
    }

    timed_keepalive = os_elapsed2(&con->last_send, tnow,
        is_serial ? IOC_SERIAL_KEEPALIVE_MS : IOC_SOCKET_KEEPALIVE_MS);
    if (timed_keepalive)
    {
        status = ioc_send_acknowledge(con);
        if (status != OSAL_SUCCESS && status != OSAL_STATUS_PENDING)
        {
            osal_debug_error("send keepalive failed");
            return OSAL_STATUS_FAILED;
        }
        osal_trace2_int("connection: keep alive sent, received byte count=", con->bytes_received);
    }
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Acknowledge if we have reached the limit of unacknowledged bytes.
  @anchor ioc_acknowledge_as_needed

  The ioc_acknowledge_as_needed() function sends acknowledge is we have reached limit of 
  unacknowledged bytes.

  @param   con Pointer to the connection object.
  @return  The function returns OSAL_SUCCESS if acknowledgement was not needed or was sent (at
           least stored in frame out buffer). The function returns OSAL_STATUS_PENDING is
           other end has not acknowledged enough free space for acknowledge message.
           Other return values indicate a broken connection.

****************************************************************************************************
*/
osalStatus ioc_acknowledge_as_needed(
    iocConnection *con)
{
    os_int 
        bytes;

    os_ushort
        u;

    osalStatus
        status;

    /* If other end has not acknowledged 3 bytes of free space, return
       OSAL_STATUS_PENDING.
     */
    u = con->bytes_sent - con->processed_bytes; 
    bytes = con->max_ack_in_air - (os_int)u;
    if (bytes < IOC_ACK_SIZE)
    {
        return OSAL_STATUS_PENDING;
    }

    /* If we have received enough unacknowledged bytes to acknowledge now.
     */
    u = con->bytes_received - con->bytes_acknowledged; 
    bytes = (os_int)u;
    if (bytes < con->unacknogledged_limit)
    {
        return OSAL_SUCCESS;
    }

    status = ioc_send_acknowledge(con);
    if (status != OSAL_SUCCESS && status != OSAL_STATUS_PENDING)
    {
        osal_debug_error("send acknowledge failed");
        return OSAL_STATUS_FAILED;
    }
    osal_trace2_int("connection: ACK sent, received byte count=", con->bytes_received);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Send frame to connection.
  @anchor ioc_write_to_stream

  The ioc_write_to_stream() function sends one frame to connection.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all data was sent. OSAL_STATUS_PENDING if nothing or part of data
           was sent. Other values indicate broken connection error.

****************************************************************************************************
*/
static osalStatus ioc_write_to_stream(
    iocConnection *con)
{
    osalStatus
        status;

    os_memsz
        n_written;

    os_int
        n;

#if OSAL_TRACE  >= 3
    os_char msg[64], nbuf[OSAL_NBUF_SZ];
    int i;
#endif

    n = con->frame_out.used - con->frame_out.pos;
    if (n <= 0) return OSAL_STATUS_PENDING;

    status = osal_stream_write(
        con->stream,
        con->frame_out.buf + con->frame_out.pos,
        n,
        &n_written,
        OSAL_STREAM_DEFAULT);

    if (n_written)
    {
        os_get_timer(&con->last_send);

        /* Add sent bytes to flow control.
         */
        con->bytes_sent += (os_ushort)n_written;
    }

    /* If this is late return for refused connection,
       delay trying to reopen.
     */
    if (status == OSAL_STATUS_CONNECTION_REFUSED)
    {
        osal_debug_error("late connect refused");
        os_get_timer(&con->socket_open_fail_timer);
    }

    /* If not all data was sent, set pending status
     */
    if (!status && n != n_written)
    {
        status = OSAL_STATUS_PENDING;
    }

#if OSAL_DEBUG
    if (status && status != OSAL_STATUS_PENDING)
    {
        osal_trace("Writing to stream failed");
    }
#if OSAL_TRACE >= 3
    else if (n_written)
    {
        osal_int_to_string(msg, sizeof(msg), n_written);
        os_strncat(msg, " bytes written to stream", sizeof(msg));
        osal_trace3(msg);

        msg[0] = '\0';
        for (i = 0; i < n_written; i++)
        {
            if (i) os_strncat(msg, ", ", sizeof(msg));
            osal_int_to_string(nbuf, sizeof(nbuf), con->frame_out.buf[con->frame_out.pos + i]);
            os_strncat(msg, nbuf, sizeof(msg));
        }
        osal_trace3(msg);
    }
#endif
#endif

    /* Advance current frame buffer position. If we got the whole frame buffer written, 
       mark it unused.
     */
    con->frame_out.pos += (int)n_written;
    if (con->frame_out.pos >= con->frame_out.used)
    {
        con->frame_out.used = 0;
        con->frame_out.pos = 0;
    }

    return status;
}


/**
****************************************************************************************************

  @brief Generate frame header.
  @anchor ioc_generate_header

  The ioc_generate_header() function generates framing header for the outgoing data messages.
  The generated header is different for serial and socket communications.

  @param   con Pointer to the connection object.
  @param   hdr Ponter to buffer where to store the generated binary header.
  @param   ptrs Pointers to structure into which to store pointers to binary header fileds
           which need to be set or modfied later. Generates header length is also stored
           in this structure.
  @param   remote_mblk_id Identifier of remote memory block to which the message being
           generated is addressed to.
  @param   addr Beginning address within the memory block where this data is written to.
   
  @return  None.

****************************************************************************************************
*/
static void ioc_generate_header(
    iocConnection *con,
    os_char *hdr,
    iocSendHeaderPtrs *ptrs,
    int remote_mblk_id,
    os_uint addr)
{
    os_boolean
        is_serial;

    os_char
        *p;

    os_uchar
        flags;

    flags = remote_mblk_id > 255 ? IOC_MBLK_HAS_TWO_BYTES : 0;
    os_memclear(ptrs, sizeof(iocSendHeaderPtrs));
    is_serial = (os_boolean)((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL);
    p = hdr;

    if (is_serial)
    {
        /* FRAME_NR: Frame number is used to check that no frame is dropped.
         */
        *(p++) = con->frame_out.frame_nr;

        /* CHECKSUM: Set zero for now and save pointer. Set at the end.
         */
        ptrs->checksum_low = p;
        *(p++) = 0;
        ptrs->checksum_high = p;
        *(p++) = 0;
    }

    /* FLAGS: Set zero for now and save pointer. Set at the end.
     */
    ptrs->flags = p;
    *(p++) = 0;

    /* BYTES: Data size in bytes, as in this message. 1 bytes 
       in serial communication, two bytes for socket.
       Set zero for now and save pointer. Set at the end.
     */
    ptrs->data_sz_low = p;
    *(p++) = 0;
    if (!is_serial)
    {
        ptrs->data_sz_high = p;
        *(p++) = 0;
    }

    /* MBLK: Memory block idenfier. 1 byte if value is less
       than 255, two otherwise.
     */
    *(p++) = (os_uchar)remote_mblk_id;
    if (flags & IOC_MBLK_HAS_TWO_BYTES)
    {
        *(p++) = (os_uchar)(remote_mblk_id >> 8);
    }

    /* ADDR: Start memory address.
     */
    *(p++) = (os_uchar)addr;
    addr >>= 8;
    if (addr)
    {
        *(p++) = (os_uchar)addr;
        addr >>= 8;
        if (addr)
        {
           *(p++) = (os_uchar)addr;
            addr >>= 8;
           *(p++) = (os_uchar)addr;
           flags |= IOC_ADDR_HAS_FOUR_BYTES;
        }
        else
        {
           flags |= IOC_ADDR_HAS_TWO_BYTES;
        }
    }

    /* Set flags and store header size.
     */
    *ptrs->flags = flags;
    ptrs->header_sz = (int)(p - hdr);
}


/**
****************************************************************************************************

  @brief Store string into message beging generated.
  @anchor ioc_msg_setstr

  The ioc_msg_setstr() function stores a string into message position p and advances p.
  The string is store as 1 byte length followed by UTF-8 characters.

  @param   str Pointer to '\0' terminated string.
  @param   p Pointer to message position pointer.
  @return  None.

****************************************************************************************************
*/
static void ioc_msg_setstr(
    os_char *str,
    os_char **p)
{
    os_memsz
        len;

    len = os_strlen(str) - 1;
    *((*p)++) = (os_char)len;
    while (len-- > 0)
    {
        *((*p)++) = (os_char)*(str++);
    }
}


/**
****************************************************************************************************

  @brief Store integer into message beging generated.
  @anchor ioc_msg_setint

  The ioc_msg_setint() function stores a 16 bit integer i into message position p and advances
  p by two bytes.

  @param   i Integer value to store, 0 .. 65535.
  @param   p Pointer to message position pointer.
  @return  OS_TRUE if resulting number has 2 bytes, OS_FALSE if 1.

****************************************************************************************************
*/
static os_boolean ioc_msg_setint(
    os_ushort i,
    os_char **p)
{
    *((*p)++) = (os_char)i;
    if (i < 256) return OS_FALSE;
    *((*p)++) = (os_char)(i >> 8);
    return OS_TRUE;
}
