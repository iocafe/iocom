/**

  @file    ioc_connection_send.c
  @brief   Send data to connection.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

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


/**
****************************************************************************************************

  @brief Select source buffer and send a frame it.
  @anchor ioc_connection_send

  The ioc_connection_send() function selects a source buffer from which frame is to be sent
  and calls ioc_send_frame to send it.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all data was sent. OSAL_PENDING if nothing or part of data
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
        status = OSAL_PENDING;


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

        case OSAL_PENDING:
            goto just_move_data;

        default:
            ioc_unlock(root);
            return OSAL_STATUS_FAILED;
    }

    /* Did we send whole acknowledge? If not, return OSAL_PENDING
     */
    if (con->frame_out.used)
    {
        ioc_unlock(root);
        return OSAL_PENDING;
    }

    /* We must send and receive authentication before sending anything else.
       Controller needs to send authentication before device to allow
     * network name "*" to connect to automatically select the network.
     */
    if ((con->flags & IOC_CONNECT_UP) && !con->authentication_received)
    {
        goto just_move_data;
    }
    if (!con->authentication_sent)
    {
        ioc_make_authentication_message(con);
        goto just_move_data;
    }
    if (!con->authentication_received)
    {
        goto just_move_data;
    }

#if IOC_DYNAMIC_MBLK_CODE
    /* If we have queued "delete memory block requests" to send for the connection,
       then send these now.
     */
    if (ioc_make_remove_mblk_req_frame(con) != OSAL_COMPLETED)
    {
        goto just_move_data;
    }
#endif

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

    /* Find out source buffer which has modified data. If some source buffer
     * is due to immediate sync in auto mode, do it.
     */
    sbuf = start_sbuf;
    while (!sbuf->syncbuf.used || !sbuf->remote_mblk_id)
    {
        if (sbuf->remote_mblk_id && sbuf->immediate_sync_needed)
        {
            if (ioc_sbuf_synchronize(sbuf))
            {
                sbuf->immediate_sync_needed = OS_FALSE;
                break;
            }

#if OSAL_MULTITHREAD_SUPPORT
            if (con->worker.trig)
            {
                osal_event_set(con->worker.trig);
            }
#endif
        }

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
    os_int
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

    os_uint
        u;

    os_ushort
        crc;

#if IOC_STATIC_MBLK_IN_PROGMEN
    os_boolean is_static = OS_FALSE;
#endif

    /* Set frame header
     */
    saved_start_addr = sbuf->syncbuf.start_addr;
    ioc_generate_header(con, con->frame_out.buf, &ptrs,
        sbuf->remote_mblk_id,
        (os_uint)saved_start_addr);

    delta = sbuf->syncbuf.delta;
    max_dst_bytes = con->dst_frame_sz - ptrs.header_sz; // DST_FRAME_SZ
    dst = con->frame_out.buf + ptrs.header_sz;

    /* Compress data from synchronized buffer. Save start addr in case
       we need to cancel send because of flow control.
     */
    con->frame_out.pos = 0;
    start_addr = saved_start_addr;
    if (delta == OS_NULL) /* IOC_STATIC -> delta=0 */
    {
#if IOC_STATIC_MBLK_IN_PROGMEN
        is_static = OS_TRUE;
#endif
        delta = sbuf->mlink.mblk->buf;
        compressed_bytes = -1;
        goto skip_for_static;
    }
    else if (!sbuf->syncbuf.is_keyframe)
    {
#if IOC_BIDIRECTIONAL_MBLK_CODE
        if (saved_start_addr <  sbuf->syncbuf.ndata)
        {
            *ptrs.flags |= IOC_DELTA_ENCODED;
        }
#else
       *ptrs.flags |= IOC_DELTA_ENCODED;
#endif
    }
    compressed_bytes = ioc_compress(delta,
        &start_addr,
        sbuf->syncbuf.end_addr,
        dst, max_dst_bytes);

skip_for_static:
    src_bytes = sbuf->syncbuf.end_addr - saved_start_addr + 1;
    if (src_bytes > max_dst_bytes) src_bytes = max_dst_bytes;
    used_bytes = (compressed_bytes < 0 ? src_bytes : compressed_bytes)
        + ptrs.header_sz;

    /* If other end has not acknowledged enough data to send the
       frame, cancel the send.
     */
    u = con->bytes_sent - con->processed_bytes;
    u &= ((con->flags & IOC_SOCKET) ? 0xFFFFFF : 0xFFFF);
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
#if IOC_STATIC_MBLK_IN_PROGMEN
        if (is_static) {
            os_memcpy_P(dst, delta + saved_start_addr, src_bytes);
        }
        else {
            os_memcpy(dst, delta + saved_start_addr, src_bytes);
        }
#else
        os_memcpy(dst, delta + saved_start_addr, src_bytes);
#endif
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
#if IOC_BIDIRECTIONAL_MBLK_CODE
        if (sbuf->syncbuf.bidir_range_set)
        {
            sbuf->syncbuf.start_addr = sbuf->syncbuf.bidir_start_addr;
            sbuf->syncbuf.end_addr = sbuf->syncbuf.bidir_end_addr;
            sbuf->syncbuf.bidir_range_set = OS_FALSE;
        }
        else
        {
            sbuf->syncbuf.used = OS_FALSE;
            *ptrs.flags |= IOC_SYNC_COMPLETE;

#if OSAL_MULTITHREAD_SUPPORT
            ioc_do_callback(sbuf->mlink.mblk, IOC_MBLK_CALLBACK_WRITE_TRIGGER, 0, 0);
#endif
        }
#else
        sbuf->syncbuf.used = OS_FALSE;
        *ptrs.flags |= IOC_SYNC_COMPLETE;
#if OSAL_MULTITHREAD_SUPPORT
        ioc_do_callback(sbuf->mlink.mblk, IOC_MBLK_CALLBACK_WRITE_TRIGGER, 0, 0);
#endif
#endif
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
  @return  None.

****************************************************************************************************
*/
static void ioc_make_mblk_info_frame(
    iocConnection *con,
    iocMemoryBlock *mblk)
{
    iocSendHeaderPtrs
        ptrs;

    os_uchar
        *p,
        *start,
        *iflags;

    os_uint
        device_nr;

    os_char
        *device_name,
        *network_name;

    /* Set frame header.
     */
    ioc_generate_header(con, con->frame_out.buf, &ptrs,
        mblk->mblk_id, 0);

    /* Generate frame content. Here we do not check for buffer overflow,
       we know (and trust) that it fits within one frame.
     */
    p = start = (os_uchar*)con->frame_out.buf + ptrs.header_sz;
    *(p++) = IOC_SYSFRAME_MBLK_INFO;
    iflags = p; /* version, for future additions (only 1 bit left for version) + flags */
    *(p++) = 0;

    /* Set device number. If we are sending to device with automatically
     * number, mark device number with IOC_TO_AUTO_DEVICE_NR.
     */
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    device_nr = mblk->device_nr;
    device_name = mblk->device_name;
    network_name = mblk->network_name;
#else
    device_nr = con->link.root->device_nr;
    device_name = con->link.root->device_name;
    network_name = con->link.root->network_name;
#endif
    if (device_nr > IOC_AUTO_DEVICE_NR)
    {
        if (device_nr == con->auto_device_nr && (mblk->local_flags & IOC_MBLK_LOCAL_AUTO_ID))
        {
            device_nr = IOC_TO_AUTO_DEVICE_NR;
        }
    }

    ioc_msg_set_uint(device_nr, &p, iflags, IOC_INFO_D_2BYTES, iflags, IOC_INFO_D_4BYTES);
    ioc_msg_set_uint(mblk->nbytes, &p, iflags, IOC_INFO_N_2BYTES, iflags, IOC_INFO_N_4BYTES);
    if (ioc_msg_set_ushort(mblk->flags, &p)) *iflags |= IOC_INFO_F_2BYTES;
    if (device_name[0])
    {
        ioc_msg_setstr(device_name, &p);
        ioc_msg_setstr(network_name, &p);
        *iflags |= IOC_INFO_HAS_DEVICE_NAME;
    }
    if (mblk->mblk_name[0] /* || network_name[0] */)
    {
        ioc_msg_setstr(mblk->mblk_name, &p);
        *iflags |= IOC_INFO_HAS_MBLK_NAME;
    }

    /* Finish outgoing frame with data size, frame number, and optional checksum. Quit here
     * if transmission is blocked by flow control.
     */
    if (ioc_finish_frame(con, &ptrs, start, p))
        return;

    /* Memory block successfully placed to frame out buffer, now we can forget about it.
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
  @return  OSAL_SUCCESS if all data was sent. OSAL_PENDING if nothing or part of data
           was sent. Other values indicate broken connection error.

****************************************************************************************************
*/
osalStatus ioc_send_acknowledge(
    iocConnection *con)
{
    osalStatus
        status;

    IOC_MT_ROOT_PTR;

    os_uchar
        *p;

    os_uint
        rbytes;

    ioc_set_mt_root(root, con->link.root);
    ioc_lock(root);

    /* If frame buffer is used, we can do nothing.
     */
    if (con->frame_out.used)
    {
        ioc_unlock(root);
        return OSAL_PENDING;
    }

    /* Generate acknowledge/keepalive message
     */
    p = (os_uchar*)con->frame_out.buf;
    *(p++) = IOC_ACKNOWLEDGE;
    rbytes = con->bytes_received;
    *(p++) = (os_uchar)rbytes;
    *p = (os_uchar)(rbytes >> 8);
    if (con->flags & IOC_SOCKET) {
        *(++p) = (os_uchar)(rbytes >> 16);
        con->frame_out.used = IOC_SOCKET_ACK_SIZE;
    }
    else {
        con->frame_out.used = IOC_SERIAL_ACK_SIZE;
    }
    con->bytes_acknowledged = rbytes;

    status = ioc_write_to_stream(con);
    os_get_timer(&con->last_send);

    /* Flush now to force acknowledge trough. Other end needs it already.
     */
    if (con->stream)
    {
        osal_stream_flush(con->stream, OSAL_STREAM_DEFAULT);
    }

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
#if OSAL_SERIAL_SUPPORT
    if (is_serial && con->sercon_state != OSAL_SERCON_STATE_CONNECTED_5)
    {
        return OSAL_SUCCESS;
    }
#endif

    timed_keepalive = os_has_elapsed_since(&con->last_send, tnow,
        is_serial ? IOC_SERIAL_KEEPALIVE_MS : IOC_SOCKET_KEEPALIVE_MS);
    if (timed_keepalive)
    {
        status = ioc_send_acknowledge(con);
        if (status != OSAL_SUCCESS && status != OSAL_PENDING)
        {
            osal_debug_error("send keepalive failed");
            return OSAL_STATUS_FAILED;
        }
#if OSAL_TRACE >= 2
        if (status == OSAL_SUCCESS)
        {
            osal_trace_int("connection: keep alive sent, received = ",
                con->bytes_received);
        }
#endif
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
           least stored in frame out buffer). The function returns OSAL_PENDING is
           other end has not acknowledged enough free space for acknowledge message.
           Other return values indicate a broken connection.

****************************************************************************************************
*/
osalStatus ioc_acknowledge_as_needed(
    iocConnection *con)
{
    os_int
        bytes,
        ack_sz;

    os_uint
        u,
        mask;

    osalStatus
        status;

    if (con->flags & IOC_SOCKET)
    {
        mask = 0xFFFFFF;
        ack_sz = IOC_SOCKET_ACK_SIZE;
    }
    else
    {
        mask = 0xFFFF;
        ack_sz = IOC_SERIAL_ACK_SIZE;
    }

    /* If other end has not acknowledged 3 (serial) bytes or 4 (socket) of free space, return
       OSAL_PENDING.
     */
    u = (con->bytes_sent - con->processed_bytes) & mask;
    bytes = con->max_ack_in_air - (os_int)u;
    if (bytes <  ack_sz) {
        return OSAL_PENDING;
    }

    /* If we have received enough unacknowledged bytes to acknowledge now.
     */
    u = (con->bytes_received - con->bytes_acknowledged) & mask;
    if (u < con->unacknogledged_limit) {
        return OSAL_SUCCESS;
    }

    status = ioc_send_acknowledge(con);
    if (status != OSAL_SUCCESS && status != OSAL_PENDING) {
        osal_debug_error("send acknowledge failed");
        return OSAL_STATUS_FAILED;
    }
    osal_trace3_int(status == OSAL_SUCCESS ? "ACK sent (SUCCESS), received="
        : "ACK sent (PENDING), received=", con->bytes_received);
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Send frame to connection.
  @anchor ioc_write_to_stream

  The ioc_write_to_stream() function sends one frame to connection.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all data was sent. OSAL_PENDING if nothing or part of data
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
    os_int i;
#endif

    n = con->frame_out.used - con->frame_out.pos;
    if (n <= 0) return OSAL_PENDING;

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
        con->bytes_sent += (os_uint)n_written;
    }

    /* If this is late return for refused connection,
       delay trying to reopen.
     */
    if (status == OSAL_STATUS_CONNECTION_REFUSED)
    {
        osal_debug_error("late connect refused");
        os_get_timer(&con->open_fail_timer);
        con->open_fail_timer_set = OS_TRUE;
    }

    /* If not all data was sent, set pending status
     */
    if (!status && n != n_written)
    {
        status = OSAL_PENDING;
    }

#if OSAL_DEBUG
    if (status && status != OSAL_PENDING)
    {
        osal_trace("Writing to stream failed");
    }
#if OSAL_TRACE >= 3
    else if (n_written)
    {
        osal_int_to_str(msg, sizeof(msg), n_written);
        os_strncat(msg, " bytes written to stream", sizeof(msg));
        osal_trace(msg);

        msg[0] = '\0';
        for (i = 0; i < n_written; i++)
        {
            if (i) os_strncat(msg, ", ", sizeof(msg));
            osal_int_to_str(nbuf, sizeof(nbuf), (os_uchar)con->frame_out.buf[con->frame_out.pos + i]);
            os_strncat(msg, nbuf, sizeof(msg));
        }
        osal_trace(msg);
    }
#endif
#endif

    /* Advance current frame buffer position. If we got the whole frame buffer written,
       mark it unused.
     */
    con->frame_out.pos += (os_int)n_written;
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

  @param   con Pointer to the connection object. If this is OS_NULL, frame number is set
           to zero, and network connection (not serial) is assumeed. This option is
           supported to alloc creating IOCOM frame header from ecom and switchbox with
           the same function.
  @param   hdr Ponter to buffer where to store the generated binary header.
  @param   ptrs Pointers to structure into which to store pointers to binary header fields
           which need to be set or modfied later. Generates header length is also stored
           in this structure.
  @param   remote_mblk_id Identifier of remote memory block to which the message being
           generated is addressed to.
  @param   addr Beginning address within the memory block where this data is written to.

  @return  None.

****************************************************************************************************
*/
void ioc_generate_header(
    iocConnection *con,
    os_char *hdr,
    iocSendHeaderPtrs *ptrs,
    os_int remote_mblk_id,
    os_int addr)
{
    os_boolean
        is_serial;

    os_uchar
        *p,
        flags;

    flags = 0;
    os_memclear(ptrs, sizeof(iocSendHeaderPtrs));
    p = (os_uchar*)hdr;

    /* FRAME_NR: Frame number is used to check that no frame is dropped.
     */
#if IOC_DYNAMIC_MBLK_CODE
    if (con) {
        is_serial = (os_boolean)((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL);
        *(p++) = con->frame_out.frame_nr;
    }
    else {
        is_serial = OS_FALSE;
        *(p++) = 0;
    }
#else
    is_serial = (os_boolean)((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL);
    *(p++) = con->frame_out.frame_nr;
#endif

    if (is_serial)
    {
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

    /* If we need extra flags.
     */
    if ((remote_mblk_id >> 16) || (addr >> 16))
    {
        flags |= IOC_EXTRA_FLAGS;
        ptrs->extra_flags = p;
        *(p++) = IOC_EXTRA_NO_ZERO;
    }

    /* MBLK: Memory block idenfier, ADDR: Start memory address.
     */
    ioc_msg_set_uint(remote_mblk_id, &p, &flags, IOC_MBLK_HAS_TWO_BYTES,
        ptrs->extra_flags, IOC_EXTRA_MBLK_HAS_FOUR_BYTES);
    ioc_msg_set_uint(addr, &p, &flags, IOC_ADDR_HAS_TWO_BYTES,
        ptrs->extra_flags, IOC_EXTRA_ADDR_HAS_FOUR_BYTES);

    /* Set flags and store header size.
     */
    *ptrs->flags = flags;
    ptrs->header_sz = (os_int)(p - (os_uchar*)hdr);
}



/**
****************************************************************************************************

  @brief Finish outgoing frame with data size, frame number, and optional checksum.
  @anchor ioc_generate_header

  The ioc_finish_frame() function ...

  Quit here if transmission is blocked by flow control.

  @param   con Pointer to the connection object.
  @param   ptrs Structure with pointers to binary header fields.
  @param   start Start position of content data.
  @param   p End position of content data.

  @return  OSAL_SUCCESS if successful. OSAL_PENDING transmission is blocked by the flow
           control and needs to be retried later.

****************************************************************************************************
*/
osalStatus ioc_finish_frame(
    iocConnection *con,
    iocSendHeaderPtrs *ptrs,
    os_uchar *start,
    os_uchar *p)
{
    os_int
        content_bytes,
        used_bytes,
        bytes;

    os_uint
        u;

    os_ushort
        crc;

    /* If other end has not acknowledged enough data to send the
       frame, cancel the send.
     */
    content_bytes = (os_int)(p - start);
    used_bytes = content_bytes + ptrs->header_sz;
    u = con->bytes_sent - con->processed_bytes;
    u &= ((con->flags & IOC_SOCKET) ? 0xFFFFFF : 0xFFFF);
    bytes = con->max_in_air - (os_int)u;
    if (used_bytes > bytes)
    {
        osal_trace2_int("MBLK info canceled by flow control, free space on air=", bytes);
        return OSAL_PENDING;
    }

    /* Fill in data size and flag as system frame.
     */
    *ptrs->data_sz_low = (os_uchar)content_bytes;
    if (ptrs->data_sz_high)
    {
        content_bytes >>= 8;
        *ptrs->data_sz_high = (os_uchar)content_bytes;
    }
    con->frame_out.used = used_bytes;
    *ptrs->flags |= IOC_SYSTEM_FRAME;

    /* Frame not rejected by flow control, increment frame number.
     */
    if (con->frame_out.frame_nr++ >= IOC_MAX_FRAME_NR)
    {
        con->frame_out.frame_nr = 1;
    }

    /* Calculate check sum out of whole used frame buffer. Notice that check sum
     * position within frame is zeros when calculating the check sum.
     */
    if (ptrs->checksum_low)
    {
        crc = os_checksum(con->frame_out.buf, con->frame_out.used, OS_NULL);
        *ptrs->checksum_low = (os_uchar)crc;
        *ptrs->checksum_high = (os_uchar)(crc >> 8);
    }

    return OSAL_SUCCESS;
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
void ioc_msg_setstr(
    const os_char *str,
    os_uchar **p)
{
    os_memsz
        len;

    len = os_strlen(str) - 1;
    *((*p)++) = (os_char)len;
    while (len-- > 0)
    {
        *((*p)++) = (os_uchar)*(str++);
    }
}


/**
****************************************************************************************************

  @brief Store 16 bit integer into message beging generated.
  @anchor ioc_msg_set_ushort

  The ioc_msg_set_ushort() function stores a 16 bit integer i into message position p and advances
  p by one or two bytes.

  @param   i Integer value to store, 0 .. 65535.
  @param   p Pointer to message position pointer.
  @return  OS_TRUE if resulting number has 2 bytes, OS_FALSE if 1.

****************************************************************************************************
*/
os_boolean ioc_msg_set_ushort(
    os_ushort i,
    os_uchar **p)
{
    *((*p)++) = (os_char)i;
    if (i < 256) return OS_FALSE;
    *((*p)++) = (os_char)(i >> 8);
    return OS_TRUE;
}


/**
****************************************************************************************************

  @brief Store 32 bit integer into message beging generated.
  @anchor ioc_msg_set_uint

  The ioc_msg_set_uint() function stores a 32 bit integer i into message position p and advances
  p by one, two or four bytes.

  @param   i Integer value to store, 0 .. 65535.
  @param   p Pointer to message position pointer.
  @param   flags Pointer to message flags to be modified to indicate if two bytes.
  @param   two_bytes_flag Bit in flags which indicates that two bytes were stored.
  @param   flags4 Pointer to message flags to be modified to indicate if four bytes were stored.
  @param   four_bytes_flag Bit in flags which indicates that four bytes were stored.
  @return  OS_TRUE if resulting number has 2 bytes, OS_FALSE if 1.

****************************************************************************************************
*/
void ioc_msg_set_uint(
    os_uint i,
    os_uchar **p,
    os_uchar *flags,
    os_uchar two_bytes_flag,
    os_uchar *flags4,
    os_uchar four_bytes_flag)
{
    os_uchar *q;
    q = *p;
    *(q++) = (os_uchar)i;
    i >>= 8;
    if (i)
    {
        *(q++) = (os_uchar)i;
        i >>= 8;
        if (i)
        {
           *(q++) = (os_uchar)i;
            i >>= 8;
           *(q++) = (os_uchar)i;
           *flags4 |= four_bytes_flag;
        }
        else
        {
           *flags |= two_bytes_flag;
        }
    }
    *p = q;
}
