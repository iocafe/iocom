/**

  @file    ioc_connection_receive.c
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

/* Forward referred static functions.
 */
static osalStatus ioc_process_received_data_frame(
    iocConnection *con,
    int mblk_id,
    int addr,
    os_char *data,
    int data_sz,
    os_uchar flags);

static osalStatus ioc_process_received_system_frame(
    iocConnection *con,
    int mblk_id,
    int addr,
    os_char *data,
    int data_sz,
    os_uchar flags);

static osalStatus ioc_store_data_frame(
    iocTargetBuffer *tbuf,
    int addr,
    os_char *data,
    int data_sz,
    os_uchar flags);

static osalStatus ioc_msg_getstr(
    os_char *str,
    os_memsz str_sz,
    os_uchar **p);

static os_ushort ioc_msg_getint(
    os_uchar **p,
    os_char two_bytes);


/**
****************************************************************************************************

  @brief Receive data from connection.
  @anchor ioc_connection_receive

  The ioc_connection_receive() function

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if whole frame was received. OSAL_STATUS_PENDING if nothing or
           some data was received. Other values indicate broken connection error.

****************************************************************************************************
*/
osalStatus ioc_connection_receive(
    iocConnection *con)
{
    iocRoot
        *root;

    os_uchar
        flags,
        *buf, /* keep unsigned */
        *p; /* keep unsigned */

    os_boolean
        is_serial;

    int
        n,
        needed;

    os_ushort
        data_sz,
        mblk_id;

    os_uint
        addr;

    osalStatus
        status;

    os_memsz
        n_read;

    os_ushort
        crc;

#if OSAL_TRACE >= 3
    os_char msg[64], nbuf[OSAL_NBUF_SZ];
    int i;
#endif

    root = con->link.root;
    ioc_lock(root);

    is_serial = (os_boolean)((con->flags & (IOC_SOCKET|IOC_SERIAL)) == IOC_SERIAL);

    buf = (os_uchar*)con->frame_in.buf;
    n = con->frame_in.pos;

    do
    {
        /* How many bytes we need in frame buffer at minimum to complete a frame.
         */
#if OSAL_SERIAL_SUPPORT
        if (is_serial)
        {
            /* If we know the frame size
             */
            if (buf[0] == IOC_ACKNOWLEDGE && n >= 1)
            {
                data_sz = 0;
                needed = 3;
                flags = 0;
            }
            else if (n >= 5)
            {
                data_sz = buf[4];
                needed = data_sz + 7;
                flags = buf[3];
                if (flags & IOC_MBLK_HAS_TWO_BYTES) needed++;
                if (flags & IOC_ADDR_HAS_FOUR_BYTES) needed+=3;
                else if (flags & IOC_ADDR_HAS_TWO_BYTES) needed++;

                if (needed > IOC_SERIAL_FRAME_SZ)
                {
                    ioc_unlock(root);
                    osal_trace("Too big serial frame");
                    return OSAL_STATUS_FAILED;
                }
            }
            else if (n >= 1)
            {
                data_sz = 0xFFFF;
                needed = 7;
                flags = 0;
            }
            else
            {
                data_sz = 0xFFFF;
                needed = 3;
                flags = 0;
            }
        }
#endif

#if OSAL_SOCKET_SUPPORT
        if (!is_serial)
        {
            if (buf[0] == IOC_ACKNOWLEDGE && n >= 1)
            {
                data_sz = 0;
                needed = 3;
                flags = 0;
            }
            else if (n >= 3)
            {
                data_sz = buf[1] | (((os_ushort)buf[2]) << 8);
                needed = data_sz + 5;
                flags = buf[0];
                if (flags & IOC_MBLK_HAS_TWO_BYTES) needed++;
                if (flags & IOC_ADDR_HAS_FOUR_BYTES) needed += 3;
                else if (flags & IOC_ADDR_HAS_TWO_BYTES) needed++;

                if (needed > IOC_SOCKET_FRAME_SZ)
                {
                    ioc_unlock(root);
                    osal_trace("Too big socket frame");
                    return OSAL_STATUS_FAILED;
                }
            }
            else if (n >= 1)
            {
                data_sz = 0xFFFF;
                needed = 5;
                flags = 0;
            }
            else
            {
                data_sz = 0xFFFF;
                needed = 3;
                flags = 0;
            }
        }
#endif

        /* If we already got it all. This may happen when looping back
           with message zero length and no additional bytes (keep alives
           mostly).
         */
        if (needed == n) break;

        status = osal_stream_read(
            con->stream,
            (os_char*)buf + n,
            (os_memsz)needed - n,
            &n_read,
            OSAL_STREAM_DEFAULT);

        if (status)
        {
            /* If this is late return for refused connection,
               delay trying to reopen.
             */
            if (status == OSAL_STATUS_CONNECTION_REFUSED)
            {
                os_get_timer(&con->socket_open_fail_timer);
            }
            ioc_unlock(root);
            osal_trace("Reading stream failed");
            return status;
        }
#if OSAL_TRACE >= 3
        else if (n_read)
        {
            osal_trace3_int("Data received, bytes=", n_read);

            msg[0] = '\0';
            for (i = 0; i < n_read; i++)
            {
                if (i) os_strncat(msg, ", ", sizeof(msg));
                osal_int_to_string(nbuf, sizeof(nbuf), buf[n + i]);
                os_strncat(msg, nbuf, sizeof(msg));
            }
            osal_trace3(msg);
        }
#endif

        /* If we receeived some data.
         */
        if (n_read)
        {
            /* Add number of bytes read to current buffer position and
             * number of received bytes.
             */
            n += (int)n_read;
            con->bytes_received += (os_ushort)n_read;

            /* Record time of receive.
             */
            os_get_timer(&con->last_receive);

#if OSAL_SERIAL_SUPPORT
            if (is_serial && n > 0)
            {
                /* If we have received nonzero frame count from serial
                   communication , make sure that it is correct already here
                   before whole package is received. This speeds up
                   detecting many errors.
                 */
                if (buf[0] != IOC_ACKNOWLEDGE && 
                    buf[0] != con->frame_in.frame_nr)
                {
                    ioc_unlock(root);
                    osal_trace("Frame number error 1");
                    return OSAL_STATUS_FAILED;
                }
            }
#endif
        }

        /* If we got all what we wanted but did't know data size yet,
           then loop back.
         */
    }
    while (n == needed && data_sz == 0xFFFF);

    con->frame_in.pos = n;

    /* If we have not received whole frame, we need to wait.
     */
    if (n < needed)
    {
        ioc_unlock(root);
        return OSAL_STATUS_PENDING;
    }

    /* If this is acknowledge.
     */
    if (buf[0] == IOC_ACKNOWLEDGE)
    {
        con->processed_bytes = (os_ushort)buf[1] | (((os_ushort)buf[2]) << 8);

        osal_trace2_int("ACK received, outgoing bytes in air=", 
            (con->bytes_sent - con->processed_bytes));
        goto alldone;
    }

#if OSAL_SERIAL_SUPPORT
    /* Get and verify check sum.
     */
    if (is_serial)
    {
        /* Get the checksum from the received data and clear the checksum position
           in received data. The check sum must be zeroed, because those values
           are zeroes when calculating check sum while sending.
         */
        crc = buf[1] | ((os_ushort)buf[2] << 8);
        buf[1] = buf[2] = 0;

        if (crc != os_checksum((os_char*)buf, needed, OS_NULL))
        {
            ioc_unlock(root);
            osal_trace("Checksum error");
            return OSAL_STATUS_FAILED;
        }
    }
#endif

    /* MBLK: Memory block identifier.
     */
    p = buf + (is_serial ? 5 : 3);
    mblk_id = *(p++);
    if (flags & IOC_MBLK_HAS_TWO_BYTES)
    {
        mblk_id |= (((os_ushort)*(p++)) << 8);
    }

    /* ADDR: Address within memory block.
     */
    addr = *(p++);
    if (flags & (IOC_ADDR_HAS_FOUR_BYTES|IOC_ADDR_HAS_TWO_BYTES))
    {
        addr |= ((os_uint)*(p++)) << 8;
        if (flags & IOC_ADDR_HAS_FOUR_BYTES)
        {
            addr |= ((os_uint)*(p++)) << 16;
            addr |= ((os_uint)*(p++)) << 24;
        }
    }

#if OSAL_SERIAL_SUPPORT
    /* Save frame number to expect next. Notice that the frame count can
        be zero only for the very first frame. never be zero again.
     */
    if (is_serial)
    {
        con->frame_in.frame_nr = buf[0] + 1u;
        if (con->frame_in.frame_nr > IOC_MAX_FRAME_NR)
        {
            con->frame_in.frame_nr = 1;
        }
    }
#endif

    /* Process the data frame.
     */
    if (flags & IOC_SYSTEM_FRAME)
    {
        status = ioc_process_received_system_frame(
            con, mblk_id, addr, (os_char*)p, data_sz, flags);
    }
    else
    {
        status = ioc_process_received_data_frame(
            con, mblk_id, addr, (os_char*)p, data_sz, flags);
    }

alldone:

    /* Ready to start next frame.
     */
    con->frame_in.pos = 0;

    /* If this is not flagged connected, do it now.
     */
    if (!con->connected)
    {
        con->connected = OS_TRUE;
        ioc_count_connected_streams(root, OS_FALSE);
        ioc_add_con_to_mbinfo(con);
    }

    ioc_unlock(root);
    return status;
}


/**
****************************************************************************************************

  @brief Process a one complete frame received from socket or serial port.
  @anchor ioc_process_received_data_frame

  The ioc_process_received_data_frame() function is called once a complete data frame 
  is received.

  ioc_lock() must be on before calling this function.

  @param   con Pointer to the connection object.
  @param   mblk_id Memory block identifier in this end.
  @param   addr Start address within memory block.
  @param   data Received data, can be compressed and delta encoded, check flags.
  @param   data_sz Size of received data in bytes.
  @param   flags Bits IOC_DELTA_ENCODED, IOC_COMPRESESSED and IOC_SYNC_COMPLETE are
           important here.

  @return  OSAL_SUCCESS if succesfull. Other values indicate corrupted frame.

****************************************************************************************************
*/
static osalStatus ioc_process_received_data_frame(
    iocConnection *con,
    int mblk_id,
    int addr,
    os_char *data,
    int data_sz,
    os_uchar flags)
{
    iocTargetBuffer
        *tbuf;

    /* If we already have connection and target memory block linked together.
     */
    for (tbuf = con->tbuf.first;
         tbuf;
         tbuf = tbuf->clink.next)
    {
        if (mblk_id == tbuf->mlink.mblk->mblk_id) break;
    }

    /* Not yet linked. Find first memory block.
     */
    if (tbuf == OS_NULL)
    {
        osal_trace2("Data for unlinked memory block");
        return OSAL_SUCCESS;
    }

    /* Store data to target buffer and optionally directly to memory block.
     */
    if (ioc_store_data_frame(tbuf, addr, data, data_sz, flags))
    {
        return OSAL_STATUS_FAILED;
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Process a one complete system frame received from socket or serial port.
  @anchor ioc_process_received_system_frame

  The ioc_process_received_system_frame() function is called once a complete systen frame is 
  received. This function processes system frames like memory block information and subscribe 
  requests.

  ioc_lock() must be on before calling this function.

  @param   con Pointer to the connection object.
  @param   mblk_id Memory block identifier in this end.
  @param   addr Start address within memory block.
  @param   data Received data, can be compressed and delta encoded, check flags.
  @param   data_sz Size of received data in bytes.
  @param   flags Bits IOC_DELTA_ENCODED, IOC_COMPRESESSED and IOC_SYNC_COMPLETE are
           important here.

  @return  OSAL_SUCCESS if succesfull. Other values indicate corrupted frame.

****************************************************************************************************
*/
static osalStatus ioc_process_received_system_frame(
    iocConnection *con,
    int mblk_id,
    int addr,
    os_char *data,
    int data_sz,
    os_uchar flags)
{
    iocMemoryBlockInfo
        mbinfo;

    os_uchar 
        version_and_flags,
        *p; /* keep as unsigned */

    switch (data[0])
    {
        /* Memory block information received. 
         */
        case IOC_SYSRAME_MBLK_INFO:
            p = (os_uchar*)data + 1; /* Skip system frame type. */
            version_and_flags = (os_uchar)*(p++);
            os_memclear(&mbinfo, sizeof(mbinfo));
            mbinfo.device_nr = ioc_msg_getint(&p, version_and_flags & IOC_INFO_D_2BYTES);
            mbinfo.mblk_nr = addr; /* ioc_msg_getint(&p); */
            mbinfo.mblk_id = mblk_id; /* ioc_msg_getint(&p); */
            mbinfo.nbytes = ioc_msg_getint(&p, version_and_flags & IOC_INFO_N_2BYTES);
            mbinfo.flags = ioc_msg_getint(&p, version_and_flags & IOC_INFO_F_2BYTES);
            if (version_and_flags & IOC_INFO_HAS_DEVICE_NAME)
            {
                if (ioc_msg_getstr(mbinfo.device_name, IOC_NAME_SZ, &p))
                    return OSAL_STATUS_FAILED;
                if (ioc_msg_getstr(mbinfo.network_name, IOC_NETWORK_NAME_SZ, &p))
                    return OSAL_STATUS_FAILED;
            }
            if (version_and_flags & IOC_INFO_HAS_MBLK_NAME)
            {
                if (ioc_msg_getstr(mbinfo.mblk_name, IOC_NAME_SZ, &p))
                    return OSAL_STATUS_FAILED;
            }
            ioc_mbinfo_received(con, &mbinfo);
            break;

        default:
            osal_debug_error("Unknown system frame received");
            break;
    }

    return OSAL_SUCCESS;
}

/**
****************************************************************************************************

  @brief Store data to target buffer and optionally directly to memory block.
  @anchor ioc_store_data_frame

  ioc_store_data_frame() function...

  ioc_lock() must be on before calling this function.

  @param   tbuf Target buffer lining this connection to memory block.
  @param   addr Start address within memory block.
  @param   data Received data, can be compressed and delta encoded, check flags.
  @param   data_sz Size of received data in bytes.
  @param   flags Bits IOC_DELTA_ENCODED, IOC_COMPRESESSED and IOC_SYNC_COMPLETE are
           important here.

  @return  OSAL_SUCCESS if succesfull. Other values indicate corrupted frame.

****************************************************************************************************
*/
static osalStatus ioc_store_data_frame(
    iocTargetBuffer *tbuf,
    int addr,
    os_char *data,
    int data_sz,
    os_uchar flags)
{
    int
        max_newdata,
        dst_bytes;

    max_newdata = tbuf->syncbuf.nbytes - addr;
    if (max_newdata < 0)
    {
        osal_debug_error("negative max_newdata");
        return OSAL_STATUS_FAILED;
    }

    /* Update newdata buffer.
     * If delta encoding, shared buffer contains delta encoded values.
     */
    dst_bytes = ioc_uncompress(
        data,
        data_sz,
        tbuf->syncbuf.newdata + addr,
        max_newdata,
        flags);

    if (dst_bytes <= 0)
    {
        if (dst_bytes == 0) return OSAL_SUCCESS;

        osal_debug_error("uncompress failed");
        return OSAL_STATUS_FAILED;
    }

    /* Mark address range of changed values (internal).
     */
    ioc_tbuf_invalidate(tbuf, addr, addr + dst_bytes - 1);

    if (flags & IOC_SYNC_COMPLETE)
    {
        /* Move data to from newdata to synchronized buffer.
         */
        ioc_tbuf_synchronize(tbuf);
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get string from received message.
  @anchor ioc_msg_getstr

  The ioc_msg_getstr() function gets string from message position p and advances
  p to first position past the string. String is store in message as 1 byte length followed
  by UTF-8 characters.

  @param   str Pointer to buffer where to store the string.
  @param   str_sz String buffer size in bytes.
  @param   p Pointer to message position pointer.
  @return  None.

****************************************************************************************************
*/
static osalStatus ioc_msg_getstr(
    os_char *str,
    os_memsz str_sz,
    os_uchar **p)
{
    int len;

    len = *((*p)++);
    if (len < 0 || len >= str_sz) 
    {
        return OSAL_STATUS_FAILED;
    }

    if (len > 0)
    {
        os_memcpy(str, *p, len);
        (*p) += len;
    }
    str[len] = '\0';
    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get integer from received message.
  @anchor ioc_msg_getint

  The ioc_msg_getint() function gets 16 bit integer from message position p and advances
  p by two bytes.

  @param   p Pointer to message position pointer.
  @return  Integer value 0 .. 65535.

****************************************************************************************************
*/
static os_ushort ioc_msg_getint(
    os_uchar **p,
    os_char two_bytes)
{
    os_ushort li;
    li = *((*p)++);
    if (two_bytes) li |= (((os_ushort)*((*p)++)) << 8);
    return li;
}
