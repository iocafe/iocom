/**

  @file    ioc_dyn_stream.c
  @brief   Dynamic streamed data transfer API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    9.12.2019

  This is interface to ioc_streamer function to implement typical functionality easier.

  Read:
    call ioc_open_stream
    call ioc_start_stream_read
    call ioc_run_stream repeatedly until completed or failed
    call ioc_get_stream_data to get data
    call either ioc_release_stream or ioc_start_stream_read

  Write:
    call ioc_open_stream
    call ioc_start_stream_write
    call ioc_run_stream repeatedly until completed or failed
    call ioc_release_stream

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

static void ioc_stream_init_signals(
    iocStreamerSignals *ptrs,
    iocStreamSignals *signal_struct,
    iocHandle *exp_handle,
    iocHandle *imp_handle,
    os_boolean is_frd);

static iocSignal *ioc_stream_set_handle(
    iocSignal *signal,
    iocHandle *handle);

static void ioc_stream_cleanup(
    iocStream *stream);

static osalStatus ioc_stream_setup_signals(
    iocStream *stream,
    iocStreamSignals *sigs,
    os_boolean is_frd);

static osalStatus ioc_stream_setup_one(
    iocSignal *signal,
    char *signal_name_prefix,
    char *signal_name_end,
    iocIdentifiers *identifiers,
    iocRoot *root);


/**
****************************************************************************************************

  @brief Allocate and initialize stream.
  @anchor ioc_initialize_dynamic_signal

  The ioc_open_stream() function allocates and sets up a new stream structure.

  @return  Pointer to stream structure, or OS_NULL if memory allocation failed.

****************************************************************************************************
*/
iocStream *ioc_open_stream(
    iocRoot *root,
    os_int select,
    const os_char *frd_buf_name,
    const os_char *tod_buf_name,
    const os_char *exp_mblk_path,
    const os_char *imp_mblk_path,
    const os_char *device_name, /* Optional, may be NULL If in path */
    os_short device_nr,
    const os_char *network_name, /* Optional, may be NULL If in path */
    os_int flags)
{
    iocStream *stream;
    iocStreamerParams *prm;
    os_char *p;

    osal_debug_assert(exp_mblk_path && imp_mblk_path);

    stream = (iocStream*)os_malloc(sizeof(iocStream), OS_NULL);
    if (stream == OS_NULL) return OS_NULL;
    os_memclear(stream, sizeof(iocStream));
    stream->root = root;
    stream->select = select;

    prm = &stream->prm;

    ioc_stream_init_signals(&prm->frd, &stream->frd, &stream->exp_handle,
        &stream->imp_handle, OS_TRUE);
    ioc_stream_init_signals(&prm->tod, &stream->tod, &stream->exp_handle,
        &stream->imp_handle, OS_FALSE);
    prm->tod.to_device = OS_TRUE;
    prm->is_device = OS_FALSE;

    os_strncpy(stream->frd_signal_name_prefix, frd_buf_name, IOC_SIGNAL_NAME_SZ);
    p = os_strchr(stream->frd_signal_name_prefix, '_');
    if (p) p[1] = '\0';
    os_strncpy(stream->tod_signal_name_prefix, tod_buf_name, IOC_SIGNAL_NAME_SZ);
    p = os_strchr(stream->tod_signal_name_prefix, '_');
    if (p) p[1] = '\0';

    ioc_iopath_to_identifiers(root, &stream->exp_identifiers,
        exp_mblk_path, IOC_EXPECT_MEMORY_BLOCK);
    ioc_iopath_to_identifiers(root, &stream->imp_identifiers,
        imp_mblk_path, IOC_EXPECT_MEMORY_BLOCK);

    if (device_name)
    {
        os_strncpy(stream->exp_identifiers.device_name, device_name, IOC_NAME_SZ);
        stream->exp_identifiers.device_nr = device_nr;
        os_strncpy(stream->imp_identifiers.device_name, device_name, IOC_NAME_SZ);
        stream->imp_identifiers.device_nr = device_nr;
    }
    if (network_name)
    {
        os_strncpy(stream->exp_identifiers.network_name, network_name, IOC_NETWORK_NAME_SZ);
        os_strncpy(stream->imp_identifiers.network_name, network_name, IOC_NETWORK_NAME_SZ);
    }

    return stream;
}

static void ioc_stream_init_signals(
    iocStreamerSignals *ptrs,
    iocStreamSignals *signal_struct,
    iocHandle *exp_handle,
    iocHandle *imp_handle,
    os_boolean is_frd)
{
    ptrs->cmd = ioc_stream_set_handle(&signal_struct->cmd, imp_handle);
    ptrs->select = ioc_stream_set_handle(&signal_struct->select, imp_handle);
    ptrs->buf = ioc_stream_set_handle(&signal_struct->buf, is_frd ? exp_handle : imp_handle);
    ptrs->head = ioc_stream_set_handle(&signal_struct->head, is_frd ? exp_handle : imp_handle);
    ptrs->tail = ioc_stream_set_handle(&signal_struct->tail, is_frd ? imp_handle : exp_handle);
    ptrs->state = ioc_stream_set_handle(&signal_struct->state, exp_handle);
}

static iocSignal *ioc_stream_set_handle(
    iocSignal *signal,
    iocHandle *handle)
{
    signal->handle = handle;
    return signal;
}


/**
****************************************************************************************************

  @brief Release stream structure.
  @anchor ioc_release_stream

  The ioc_release_stream() function frees memory allocated for the stream structure.

  @param   dsignal Pointer to stream structure to release.
  @return  None.

****************************************************************************************************
*/
void ioc_release_stream(
    iocStream *stream)
{
    if (stream)
    {
        ioc_stream_cleanup(stream);
        os_free(stream, sizeof(iocStream));
    }
}

/* Start writing data to stream.
 */
static void ioc_stream_cleanup(
    iocStream *stream)
{
    ioc_streamer_close(stream->streamer, OSAL_STREAM_DEFAULT);
    stream->streamer_opened = OS_FALSE;
    stream->streamer = OS_NULL;

    if (stream->read_buf)
    {
        osal_stream_buffer_close(stream->read_buf, OSAL_STREAM_DEFAULT);
        stream->read_buf = OS_NULL;
    }

    if (stream->write_buf)
    {
        os_free(stream->write_buf, stream->write_buf_sz);
        stream->write_buf = OS_NULL;
    }

    if (stream->exp_handle.mblk)
    {
        ioc_release_handle(&stream->exp_handle);
    }
    if (stream->imp_handle.mblk)
    {
        ioc_release_handle(&stream->imp_handle);
    }
}


/* Lock must be on
 * */
static osalStatus ioc_stream_try_setup(
    iocStream *stream)
{
    /* If we have all set up already ?
     */
    if (stream->exp_handle.mblk && stream->imp_handle.mblk)
    {
        return OSAL_SUCCESS;
    }

    if (stream->frd_signal_name_prefix[0] != '\0')
    {
        if (ioc_stream_setup_signals(stream, &stream->frd, OS_TRUE)) goto failed;
    }
    if (stream->tod_signal_name_prefix[0] != '\0')
    {
        if (ioc_stream_setup_signals(stream, &stream->tod, OS_FALSE)) goto failed;
    }

    return OSAL_SUCCESS;

failed:
    ioc_release_handle(&stream->exp_handle);
    ioc_release_handle(&stream->imp_handle);
    return OSAL_STATUS_FAILED;
}


static osalStatus ioc_stream_setup_signals(
    iocStream *stream,
    iocStreamSignals *sigs,
    os_boolean is_frd)
{
    os_char *prefix;
    iocIdentifiers *ei, *ii;
    iocRoot *root = stream->root;

    ei = &stream->exp_identifiers;
    ii = &stream->imp_identifiers;

    prefix = is_frd ? stream->frd_signal_name_prefix : stream->tod_signal_name_prefix;

    if (ioc_stream_setup_one(&sigs->cmd, prefix, "cmd", ii, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->select, prefix, "select", ii, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->buf, prefix, "buf", is_frd ? ei : ii, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->head, prefix, "head", is_frd ? ei : ii, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->tail, prefix, "tail", is_frd ? ii : ei, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->state, prefix, "state", ei, root)) return OSAL_STATUS_FAILED;

    return OSAL_SUCCESS;
}


static osalStatus ioc_stream_setup_one(
    iocSignal *signal,
    char *signal_name_prefix,
    char *signal_name_end,
    iocIdentifiers *identifiers,
    iocRoot *root)
{
    iocDynamicSignal *dsignal;

    os_strncpy(identifiers->signal_name, signal_name_prefix, IOC_SIGNAL_NAME_SZ);
    os_strncat(identifiers->signal_name, signal_name_end, IOC_SIGNAL_NAME_SZ);

    dsignal = ioc_setup_signal_by_identifiers(root, identifiers, signal);
    return dsignal ? OSAL_SUCCESS : OSAL_STATUS_FAILED;
}



/* Start reading data from stream.
 */
void ioc_start_stream_read(
    iocStream *stream)
{
    ioc_stream_cleanup(stream);
    stream->flags = OSAL_STREAM_READ;

    stream->read_buf = osal_stream_buffer_open(OS_NULL,
        OS_NULL, OS_NULL, OSAL_STREAM_WRITE);
}


/* Start writing data to stream.
 */
void ioc_start_stream_write(
    iocStream *stream,
    const os_char *buf,
    os_memsz buf_sz)
{
    ioc_stream_cleanup(stream);
    stream->flags = OSAL_STREAM_WRITE;

    stream->write_buf_sz = (os_int)buf_sz;
    stream->write_buf = os_malloc(buf_sz, OS_NULL);
    stream->write_buf_pos = 0;
    if (stream->write_buf == OS_NULL) return;
    os_memcpy(stream->write_buf, buf, buf_sz);
}


/* Call run repeatedly until data transfer is complete or has failed.
 * @param  flags IOC_CALL_SYNC cal ioc_receive() and ioc_send()
 */
osalStatus ioc_run_stream(
    iocStream *stream,
    os_int flags)
{
    os_char buf[256], nbuf[OSAL_NBUF_SZ];
    os_memsz n_read, n_written, n;
    osalStatus s;

    s = ioc_stream_try_setup(stream);
    if (s)
    {
        return OSAL_STATUS_PENDING;
    }

    if (flags & IOC_CALL_SYNC)
    {
        ioc_receive(&stream->exp_handle);
    }

    if (!stream->streamer_opened)
    {
        osal_int_to_str(nbuf, sizeof(nbuf), stream->select);
        stream->streamer = ioc_streamer_open(nbuf, &stream->prm, OS_NULL,
            stream->flags);

        if (stream->streamer == OS_NULL)
        {
            return OSAL_STATUS_FAILED;
        }
        stream->streamer_opened = OS_TRUE;
    }

    if (stream->streamer == OS_NULL) return OSAL_STATUS_FAILED;

    if (stream->flags & OSAL_STREAM_READ)
    {
        s = ioc_streamer_read(stream->streamer, buf, sizeof(buf), &n_read, OSAL_STREAM_DEFAULT);
        if (n_read > 0)
        {
            osal_stream_buffer_write(stream->read_buf, buf, n_read, &n_written, OSAL_STREAM_DEFAULT);
        }
        if (s)
        {
            ioc_streamer_close(stream->streamer, OSAL_STREAM_DEFAULT);
            stream->streamer = OS_NULL;
        }
    }
    else
    {
        n = stream->write_buf_sz - stream->write_buf_pos;
        if (n > 0)
        {
            s = ioc_streamer_write(stream->streamer,
                stream->write_buf + stream->write_buf_pos,
                n, &n_written, OSAL_STREAM_DEFAULT);

            stream->write_buf_pos += (os_int)n_written;
        }
        else
        {
            s = ioc_streamer_write(stream->streamer, "", -1, &n_written, OSAL_STREAM_DEFAULT);
        }
    }

    if (flags & IOC_CALL_SYNC)
    {
        ioc_send(&stream->imp_handle);
    }

    return s;
}


/* Get pointer to received data, valid until next stream function call.
   Does not allocate new copy, returns the pointer to data stored
   within the stream object.
 */
os_char *ioc_get_stream_data(
    iocStream *stream,
    os_memsz *buf_sz)
{
    /* Verify that ioc_start_stream_read() has been called.
     */
    osal_debug_assert(stream->flags & OSAL_STREAM_READ);

    if (stream->read_buf)
    {
        return osal_stream_buffer_content(stream->read_buf, buf_sz);
    }
    *buf_sz = 0;
    return OS_NULL;
}

#endif
