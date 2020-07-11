/**

  @file    ioc_dyn_stream.c
  @brief   Dynamic streamed data transfer API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  This is interface to ioc_streamer function to implement typical functionality easier
  in environment where use dynamic memory allocation is feasible. This interface is
  not suitable for microcontrollers with limited resources: In limited resource environment
  use ioc_streamer directly, it doesn't need dynamic memory allocation and doesn't make
  buffer all data to be transferred in RAM.

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

/* Forward referred static functions.
 */
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

  @brief Open and initialize IOC stream.
  @anchor ioc_open_stream

  The ioc_open_stream() function allocates and sets up a new IOC stream structure. The IOC
  stream is higher level object used to present underlyin streamer trough easier API.

  @param   root IOCOM root object.
  @param   select Select what to transfer, typically persistant parameter block number like
           OS_PBNR_FLASH_PROGRAM (1) for microcontroller flash program, OS_PBNR_CONFIG (2)
           for network configuration, like device identification, network addresses, security
           certificates, etc.
  @param   frd_buf_name Name of signal (char array) used as ring buffer for transfers from
           device to controller, typically "frd_buf".
  @param   toc_buf_name Name of signal (char array) used as ring buffer for transfers from
           controller to IO device, typically "tod_buf".
  @param   exp_mblk_path IO path to memory block expored by IO device and used for transfer.
           An device end, this could simply be "conf_exp" or in controller end more
           precisely "conf_exp.gina7.iocafenet", etc.
  @param   imp_mblk_path IO path to memory block imported by IO device and used for transfer,
           An device end, this could be "conf_imp" or "conf_imp.gina7.iocafenet" in controller.

  @param   device_name Optional: Sometimes it is more convinient to give device_name,
           device_number and network name as separate arguments. Device name without serial
           number, like "gina", can be here and the device names in exp_mblk_path and imp_mblk_path
           are ignored. Set to OS_NULL to use device name in path.
  @param   device_nr See "device_name" above. If device name is given as separate argument,
           device number in this argument is used. If not needed, set 0.
  @param   network_name See "device_name" above, alternative way to give network name as
           separate argument. Set OS_NULL if not needed.

  @param   flags Set IOC_IS_DEVICE if this is device end, or IOC_IS_CONTROLLER if this is
           controller end.

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
    const os_char *device_name,
    os_uint device_nr,
    const os_char *network_name,
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
    prm->is_device = (flags & IOC_IS_DEVICE) ? OS_TRUE : OS_FALSE;

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


/**
****************************************************************************************************

  @brief Setup "ptrs" structure and set handles (internal for this module)
  @anchor ioc_stream_set_handle

  Set signal pointers with within "ptrs" structure and store appropriate memory block
  handle for each signal. This function takes care of signals to one direction.

  @param   ptrs Pointers structure to set up. This will be needed as argument to lower
           level streamer.
  @param   signal_struct Structure which allocates memory for signals.
  @param   exp_handle Memory block handle for signals exported by IO device.
  @param   imp_handle Memory block handle for signals imposted by IO device.
  @return  None.

****************************************************************************************************
*/
static void ioc_stream_init_signals(
    iocStreamerSignals *ptrs,
    iocStreamSignals *signal_struct,
    iocHandle *exp_handle,
    iocHandle *imp_handle,
    os_boolean is_frd)
{
    ptrs->cmd = ioc_stream_set_handle(&signal_struct->cmd, imp_handle);
    ptrs->select = ioc_stream_set_handle(&signal_struct->select, imp_handle);
    ptrs->err = ioc_stream_set_handle(&signal_struct->err, exp_handle);
    ptrs->cs = ioc_stream_set_handle(&signal_struct->cs, is_frd ? exp_handle : imp_handle);
    ptrs->buf = ioc_stream_set_handle(&signal_struct->buf, is_frd ? exp_handle : imp_handle);
    ptrs->head = ioc_stream_set_handle(&signal_struct->head, is_frd ? exp_handle : imp_handle);
    ptrs->tail = ioc_stream_set_handle(&signal_struct->tail, is_frd ? imp_handle : exp_handle);
    ptrs->state = ioc_stream_set_handle(&signal_struct->state, exp_handle);
}


/**
****************************************************************************************************

  @brief Just set the stream handle (internal for this module)
  @anchor ioc_stream_set_handle

  This function exists only to make ioc_stream_init_signals more readable. It sets the
  memory block handle pointer within signal structure and return pointer to signal
  structure given as argument.

  @param   signal Pointer to signal sturture.
  @param   handle Pointer to memory block handle to set within signal structure.
  @return  Same signal structure pointer as was given as argument.

****************************************************************************************************
*/
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

  The ioc_release_stream() function deleted the stream object and releases all resources
  associated with it. Stream pointer is not valid after this call.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
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


/**
****************************************************************************************************

  @brief Release up allocated resources (internal for the code file).
  @anchor ioc_stream_cleanup

  Close contained IOC streamer, release any memory allocated for read or write buffers and
  release memory block handles.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @return  None.

****************************************************************************************************
*/
static void ioc_stream_cleanup(
    iocStream *stream)
{
    ioc_streamer_close(stream->streamer, OSAL_STREAM_DEFAULT);
    stream->streamer_opened = OS_FALSE;
    stream->streamer = OS_NULL;

    if (stream->read_buf) {
        osal_stream_buffer_close(stream->read_buf, OSAL_STREAM_DEFAULT);
        stream->read_buf = OS_NULL;
    }

    if (stream->write_buf) {
        if (stream->write_buf_allocated) {
            os_free(stream->write_buf, stream->write_buf_sz);
        }
        stream->write_buf = OS_NULL;
    }

    if (stream->exp_handle.mblk) {
        ioc_release_handle(&stream->exp_handle);
    }

    if (stream->imp_handle.mblk) {
        ioc_release_handle(&stream->imp_handle);
    }
}


/**
****************************************************************************************************

  @brief Setup all signals for the stream (internal for the code file).
  @anchor ioc_stream_try_setup

  This function sets up memory block handle, signal address within memory block, data type, etc
  for all signals used for the stream in either direction.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @return  If successful, the function returns OSAL_SUCCESS. If it was not possible to set up
           all the signals, the function returns error code (!= OSAL_SUCCESS).

****************************************************************************************************
*/
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


/**
****************************************************************************************************

  @brief Setup one signals to stream to one direction (internal for the code file).
  @anchor ioc_stream_setup_signals

  This function calls ioc_stream_setup_one for "cmd", "select", "buf", "head", "tail" and
  "state" to set memory block handle, signal address within memory block, data type, etc
  witin the signal structure.

****************************************************************************************************
*/
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
    if (ioc_stream_setup_one(&sigs->err, prefix, "err", ei, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->cs, prefix, "cs", is_frd ? ei : ii, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->buf, prefix, "buf", is_frd ? ei : ii, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->head, prefix, "head", is_frd ? ei : ii, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->tail, prefix, "tail", is_frd ? ii : ei, root)) return OSAL_STATUS_FAILED;
    if (ioc_stream_setup_one(&sigs->state, prefix, "state", ei, root)) return OSAL_STATUS_FAILED;

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Setup one signal (internal for the code file).
  @anchor ioc_stream_setup_one

  This function sets memory block handle, signal address within memory block, data type, etc
  witin the signal structure for one signal.

****************************************************************************************************
*/
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


/**
****************************************************************************************************

  @brief Prepare to start reading data from stream.
  @anchor ioc_start_stream_read

  This function prepares stream object for reading data. Call ioc_run_stream() function
  to actually transfer the data.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @return  None.

****************************************************************************************************
*/
void ioc_start_stream_read(
    iocStream *stream)
{
    ioc_stream_cleanup(stream);
    stream->flags = OSAL_STREAM_READ;

    stream->read_buf = osal_stream_buffer_open(OS_NULL,
        OS_NULL, OS_NULL, OSAL_STREAM_WRITE);
}


/**
****************************************************************************************************

  @brief Prepare to start writing data to stream.
  @anchor ioc_start_stream_write

  This function stores data to write into buffer within the IOC stream object and prepares
  stream object for writing. Call ioc_run_stream() function to actually transfer the data.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @param   buf Pointer to data to write. This can be network configuration as packed JSON
           or flash program, etc.
  @param   buf_sz Data size in bytes.
  @param   copy_buf If OS_TRUE, new buffer is allocated and content is copied to it.
           If OS_FALSE, buffer is used as is and must extst until stream is released.
  @return  None.

****************************************************************************************************
*/
void ioc_start_stream_write(
    iocStream *stream,
    const os_char *buf,
    os_memsz buf_sz,
    os_boolean copy_buf)
{
    ioc_stream_cleanup(stream);
    stream->flags = OSAL_STREAM_WRITE;

    stream->write_buf_sz = (os_int)buf_sz;
    stream->write_buf_pos = 0;
    stream->write_buf_allocated = copy_buf;

    if (copy_buf)
    {
        stream->write_buf = os_malloc(buf_sz, OS_NULL);
        if (stream->write_buf == OS_NULL) return;
        os_memcpy(stream->write_buf, buf, buf_sz);
    }
    else
    {
        stream->write_buf = (os_char*)buf;
    }
}


/**
****************************************************************************************************

  @brief Transfer the data
  @anchor ioc_run_stream

  Nonblocking function to do data transfer initiated either by ioc_start_stream_read() or
  ioc_start_stream_write() function call. Call run repeatedly until data transfer is complete
  or has failed.
  Send: Data is written from buffer within the IOC stream object, initialized by
  ioc_start_stream_write() call.
  Receive: Received data is buffered within stream object.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @param   flags If IOC_CALL_SYNC flags is given, the function calls ioc_send() and ioc_receive()
           to move data between memory block and transport (sync buffers).
  @return  As long as transfer is still going on, the function returns OSAL_SUCCESS.
           Once the transfer has successfully been completed, the function returns
           OSAL_COMPLETED. Other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_run_stream(
    iocStream *stream,
    os_int flags)
{
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_char *buf;
    os_memsz buf_sz;
#else
    os_char buf[256];
    const os_memsz buf_sz = sizeof(buf);
#endif
    os_char nbuf[OSAL_NBUF_SZ];
    os_memsz n_read, n_written, n;
    osalStatus s;

    s = ioc_stream_try_setup(stream);
    if (s) {
        return OSAL_PENDING;
    }

    if (flags & IOC_CALL_SYNC) {
        ioc_receive(stream->prm.is_device ? &stream->imp_handle : &stream->exp_handle);
    }

    if (!stream->streamer_opened) {
        osal_int_to_str(nbuf, sizeof(nbuf), stream->select);
        stream->streamer = ioc_streamer_open(nbuf, &stream->prm, OS_NULL,
            stream->flags);

        if (stream->streamer == OS_NULL) {
            s = OSAL_STATUS_FAILED;
            goto getout;
        }
        stream->streamer_opened = OS_TRUE;
    }

    if (stream->streamer == OS_NULL) return OSAL_STATUS_FAILED;

    if (stream->flags & OSAL_STREAM_READ)
    {
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
        buf_sz = stream->prm.frd.buf->n - 1;
        if (stream->prm.is_device) {
            buf_sz = stream->prm.tod.buf->n - 1;
        }
        osal_debug_assert(buf_sz > 0);
        buf = os_malloc(buf_sz, OS_NULL);
        if (buf == OS_NULL) {
            return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        }
#endif
        s = ioc_streamer_read(stream->streamer, buf, buf_sz, &n_read, OSAL_STREAM_DEFAULT);
        if (n_read > 0) {
            osal_stream_buffer_write(stream->read_buf, buf, n_read, &n_written, OSAL_STREAM_DEFAULT);
            stream->bytes_moved += n_read;
        }

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
        os_free(buf, buf_sz);
#endif
        if (s) {
            ioc_streamer_close(stream->streamer, OSAL_STREAM_DEFAULT);
            stream->streamer = OS_NULL;
        }
    }
    else
    {
        n = stream->write_buf_sz - stream->write_buf_pos;
        if (n > 0) {
            s = ioc_streamer_write(stream->streamer,
                stream->write_buf + stream->write_buf_pos,
                n, &n_written, OSAL_STREAM_DEFAULT);

            stream->write_buf_pos += (os_int)n_written;
        }
        else {
            s = ioc_streamer_write(stream->streamer, osal_str_empty, -1,
                &n_written, OSAL_STREAM_DEFAULT);
        }
        stream->bytes_moved += n_written;
    }

getout:
    if (flags & IOC_CALL_SYNC)
    {
        ioc_send(stream->prm.is_device ? &stream->exp_handle : &stream->imp_handle);
    }

    return s;
}


/**
****************************************************************************************************

  @brief Get pointer to received data
  @anchor ioc_get_stream_data

  Pointer to received data is valid until next IOC stream function call. The function not
  allocate new copy of data, it returns the pointer to data stored within the stream object.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @param   buf_sz Pointer to integer where to store number of buffered data bytes.
  @param   flags Reserved for future, set 0.
  @return  Pointer to received data.

****************************************************************************************************
*/
os_char *ioc_get_stream_data(
    iocStream *stream,
    os_memsz *buf_sz,
    os_int flags)
{
    OSAL_UNUSED(flags);

    /* Verify that ioc_start_stream_read() has been called.
     */
    osal_debug_assert(stream->flags & OSAL_STREAM_READ);

    /* If we have received data, return pointer to it.
     */
    if (stream->read_buf)
    {
        return osal_stream_buffer_content(stream->read_buf, buf_sz);
    }

    /* No data, return NULL pointer and zero size.
     */
    *buf_sz = 0;
    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Initialize IO device stream state states
  @anchor ioc_stream_initconf

  Get delayed stream status (for example when programming flash). Can be used after
  ioc_run_stream() has returned OSAL_COMPLETED or failed.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @return  OSAL_SUCCESS = not really started.
           OSAL_PENDING = waiting for results.
           OSAL_COMPLETED = successfully completed.
           Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_stream_status(
    iocStream *stream)
{
    osalStatus s;
    iocSignal *sig;
    os_char state_bits;

    sig = (stream->flags & OSAL_STREAM_READ) ? &stream->frd.err : &stream->tod.err;
    if (sig) if (sig->handle)
    {
        s = (osalStatus)ioc_get_ext(sig, &state_bits, IOC_SIGNAL_DEFAULT);
        if ((state_bits & OSAL_STATE_CONNECTED) == 0) {
            return OSAL_SUCCESS;
        }

        return s;
    }

    return OSAL_STATUS_FAILED;
}

/**
****************************************************************************************************

  @brief Initialize IO device stream state states
  @anchor ioc_stream_initconf

  IO device needs to initialize "state" signals to both direction to idle state (0). This
  enables OSAL_STATE_CONNECTED bit for the signals, indicating to controller that IO device is
  ready for streaming.
  This function sets "frd_state" and "tod_state" in "conf_exp" and "conf_imp memory blocks
  used to transfer flash program and network configration, etc.

  @param   stream Pointer to IOC stream, as returned by ioc_open_stream().
  @param   flags IOC_CALL_SYNC to call ioc_send() to transfer the initial signal values.
  @return  If successful, the function returns OSAL_SUCCESS. Other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_stream_initconf(
    iocStream *stream,
    os_int flags)
{
    os_char nbuf[OSAL_NBUF_SZ];
    osalStatus s;

    s = ioc_stream_try_setup(stream);
    if (s)
    {
        return OSAL_PENDING;
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

    if (stream->prm.is_device)
    {
        ioc_set(stream->prm.frd.state, 0);
        ioc_set(stream->prm.tod.state, 0);

        if (flags & IOC_CALL_SYNC)
        {
            ioc_send(&stream->exp_handle);
        }
    }
    return OSAL_SUCCESS;
}

#endif
