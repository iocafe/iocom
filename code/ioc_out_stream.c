/**

  @file    ioc_streamer.c
  @brief   Data stream trough memory block API.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Ther end of the stream routed trough memory block is flagged as controller and the other as
  device. Controller is the "boss" who starts the transfers. Transfer ends either when the
  while file, etc, has been transferred, or the controller interrupts the transfer.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#define OSAL_TRACE 3

#include "iocom.h"
#if IOC_STREAMER_SUPPORT


#if IOC_DEVICE_STREAMER
/**
****************************************************************************************************

  @brief Initialize control stream.
  @anchor ioc_init_control_stream

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_output_stream(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    os_memclear(ctrl, sizeof(iocControlStreamState));

    ioc_sets0_int(params->frd.state, 0);
    ioc_sets0_int(params->tod.state, 0);

#if OSAL_DEBUG
    ctrl->initialized = 'I';
#endif
}

/* Release memory block object.
 */
void ioc_release_output_stream(
    iocControlStreamState *ctrl)
{

}


/* Release memory block object.
 */
osalStatus ioc_write_item_to_output_stream(
    iocControlStreamState *ctrl)
{


}

/**
****************************************************************************************************

  @brief Keep control stream for transferring IO device configuration and flash program alive.
  @anchor ioc_run_control_stream

  This is IO device end function, which handles transfer of configuration and flash software,
  etc. The device configuration included device identification, network configuration and
  security configuration, like certificates, etc.

  The streamer is used  to transfer a stream using buffer within memory block. This static
  params structure selects which signals are used for straming.

  The function is called repeatedly to run data this data transfer between the  controller and
  the IO device. The function reads data from the stream buffer in memory block (as much as
  there is) and writes it to persistent storage.

  If the function detects IOC_STREAM_COMPLETED or IOC_STREAM_INTERRUPTED command, or if
  connection has broken, it closes the persistent storage and memory block streamer.
  Closing persistent object is flagged with success OSAL_STREAM_DEFAULT only on
  IOC_STREAM_COMPLETED command, otherwise persistent object is closed with OSAL_STREAM_INTERRUPT
  flag (in this case persient object may not want to use newly received data, especially if
  it is flash program for micro-controller.

  This function must be called from one thread at a time.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.

  @return  If working in something, the function returns OSAL_SUCCESS. Return value
           OSAL_NOTHING_TO_DO indicates that this thread can be switched to slow
           idle mode as far as the control stream knows.

****************************************************************************************************
*/
osalStatus ioc_run_control_stream(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    iocStreamerState cmd;
    osPersistentBlockNr select;
    os_char state_bits;
    osalStatus s = OSAL_NOTHING_TO_DO;

    /* Just for debugging, assert here that ioc_init_control_stream() has been called.
     */
    osal_debug_assert(ctrl->initialized == 'I');

    /* No status yet
     */
    ctrl->transfer_status = IOC_NO_BLOCK_TRANSFERRED;

    if (ctrl->frd == OS_NULL)
    {
        cmd = (iocStreamerState)ioc_gets_int(params->frd.cmd, &state_bits, IOC_SIGNAL_DEFAULT);
        if (cmd == IOC_STREAM_RUNNING && (state_bits & OSAL_STATE_CONNECTED))
        {
            osal_trace3("IOC_STREAM_RUNNING command");
            ctrl->frd = ioc_streamer_open(OS_NULL, params, OS_NULL, OSAL_STREAM_WRITE);

            if (ctrl->frd)
            {
                ctrl->transferring_default_config = OS_FALSE;
                select = (osPersistentBlockNr)ioc_gets0_int(params->frd.select);

                if (select == OS_PBNR_DEFAULTS)
                {
                    ctrl->transferring_default_config = OS_TRUE;
                    ctrl->default_config_pos = 0;
                    ctrl->fdr_persistent_ok = OS_TRUE;
                }
                else
                {
                    ctrl->fdr_persistent = os_persistent_open(select, OS_NULL, OSAL_PERSISTENT_READ);
                    ctrl->fdr_persistent_ok = (ctrl->fdr_persistent != OS_NULL);
#if OSAL_DEBUG
                    if (ctrl->fdr_persistent == OS_NULL)
                    {
                        osal_debug_error_int("Reading persistent block failed", select);
                    }
                }
#endif

                /* If we are getting certificate chain, mark that we have it.
                 */
                if (select == OS_PBNR_CLIENT_CERT_CHAIN) {
                    osal_set_network_state_int(OSAL_NS_NO_CERT_CHAIN, 0, OS_FALSE);
                }

                os_get_timer(&ctrl->timer_ms);
            }
        }
    }

    if (ctrl->frd)
    {
        ioc_ctrl_stream_from_device(ctrl, params);
        s = OSAL_SUCCESS;
    }

    if (ctrl->tod  == OS_NULL)
    {
        cmd = (iocStreamerState)ioc_gets_int(params->tod.cmd, &state_bits, IOC_SIGNAL_DEFAULT);
        if (cmd == IOC_STREAM_RUNNING && (state_bits & OSAL_STATE_CONNECTED))
        {
            ctrl->tod = ioc_streamer_open(OS_NULL, params, OS_NULL, OSAL_STREAM_READ);

            if (ctrl->tod)
            {
                select = (osPersistentBlockNr)ioc_gets0_int(params->tod.select);
                ctrl->transferred_block_nr = select;
                ctrl->tod_persistent = os_persistent_open(select, OS_NULL, OSAL_PERSISTENT_WRITE);
#if OSAL_DEBUG
                if (ctrl->tod_persistent == OS_NULL)
                {
                    osal_debug_error_int("Writing persistent block failed", select);
                }
#endif
            }
        }
    }

    if (ctrl->tod)
    {
        ioc_ctrl_stream_to_device(ctrl, params);
        s = OSAL_SUCCESS;
    }

    return s;
}


/**
****************************************************************************************************

  @brief Move data from IO device tp controller.
  @anchor ioc_ctrl_stream_from_device

  This code is used in IO device. The function is called repeatedly to run data transfer
  from IO device to controller. The function reads data from persistent storage and
  writes it to stream buffer in memory block. When the data ends, the device will show
  IOC_STREAM_COMPLETED state.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
static void ioc_ctrl_stream_from_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    os_char buf[256];
    os_memsz rdnow, n_written, n_read, bytes;
    osalStatus s;

    if (ctrl->fdr_persistent || ctrl->transferring_default_config)
    {
        bytes = ioc_streamer_get_parameter(ctrl->frd, OSAL_STREAM_TX_AVAILABLE);
        while (OS_TRUE)
        {
            if (bytes <= 0)
            {
                if (!os_has_elapsed(&ctrl->timer_ms, IOC_STREAMER_TIMEOUT)) return;
                break;
            }
            os_get_timer(&ctrl->timer_ms);

            rdnow = bytes;
            if (rdnow > sizeof(buf)) rdnow = sizeof(buf);

            /* Get static default network congiguration.
             */
            if (ctrl->transferring_default_config)
            {
                if (params->default_config == OS_NULL)
                {
                    ctrl->fdr_persistent_ok = OS_FALSE;
                    break;
                }
                n_read = params->default_config_sz - ctrl->default_config_pos;
                if (rdnow < n_read) n_read = rdnow;
                os_memcpy(buf, params->default_config + ctrl->default_config_pos, n_read);
                ctrl->default_config_pos += (os_int)n_read;
            }

            /* Get actual persistent data.
             */
            else
            {
                n_read = os_persistent_read(ctrl->fdr_persistent, buf, rdnow);
            }

            if (n_read > 0)
            {
                s = ioc_streamer_write(ctrl->frd, buf, n_read, &n_written, OSAL_STREAM_DEFAULT);
                if (s) break;
                osal_debug_assert(n_written == n_read);
            }

            /* If all has been read?
             */
            if (n_read < rdnow)
            {
                if (n_read < 0) ctrl->fdr_persistent_ok = OS_FALSE;
                break;
            }
            bytes -= n_read;
        }

        os_persistent_close(ctrl->fdr_persistent, OSAL_PERSISTENT_DEFAULT);
        ctrl->fdr_persistent = OS_NULL;
    }

    /* Finalize any handshaking signal stuff.
     */
    s = ioc_streamer_flush(ctrl->frd, ctrl->fdr_persistent_ok
        ? OSAL_STREAM_FINAL_HANDSHAKE
        : OSAL_STREAM_FINAL_HANDSHAKE|OSAL_STREAM_INTERRUPT);

    if (s == OSAL_PENDING) return;

    /* Close the stream
     */
    ioc_streamer_close(ctrl->frd, OSAL_STREAM_DEFAULT);
    ctrl->frd = OS_NULL;
}


/**
****************************************************************************************************

  @brief Move data from controller to IO device.
  @anchor ioc_ctrl_stream_to_device

  This code is used in IO device. The function is called repeatedly to run data transfer
  from controller to IO device. The function reads data from persistent storage and
  writes it to stream buffer in memory block. When the data ends, the device will show
  IOC_STREAM_COMPLETED state. If transfer is interrupted (for example reading persistent
  storage fails, the IOC_STREAM_INTERRUPT state is set to memory block.

  @param   ctrl IO device control stream transfer state structure.
  @param   params Parameters for the streamer.
  @return  None.

****************************************************************************************************
*/
static void ioc_ctrl_stream_to_device(
    iocControlStreamState *ctrl,
    iocStreamerParams *params)
{
    os_char buf[256];
    os_memsz n_read;
    osalStatus s;
    os_int stream_flags;

    stream_flags = ctrl->tod_persistent ? OSAL_STREAM_DEFAULT : OSAL_STREAM_INTERRUPT;

    do
    {
        s = ioc_streamer_read(ctrl->tod, buf, sizeof(buf), &n_read, stream_flags);
        if (n_read == 0)
        {
            if (s == OSAL_SUCCESS) return;
            break;
        }
        if (ctrl->tod_persistent)
        {
            os_persistent_write(ctrl->tod_persistent, buf, n_read);
        }
    }
    while (s == OSAL_SUCCESS);

    if (s != OSAL_COMPLETED) stream_flags = OSAL_STREAM_INTERRUPT;

    if (ctrl->tod_persistent)
    {
        ctrl->transfer_status = IOC_BLOCK_WRITTEN;
        os_persistent_close(ctrl->tod_persistent, stream_flags);
        ctrl->tod_persistent = OS_NULL;
    }

    ioc_streamer_close(ctrl->tod, stream_flags);
    ctrl->tod = OS_NULL;
}
#endif


/** Stream interface for OSAL streamers. This is structure is filled with
    function pointers to memory block streamer implementation.
 */
const osalStreamInterface ioc_streamer_iface
 = {OSAL_STREAM_IFLAG_NONE,
    ioc_streamer_open,
    ioc_streamer_close,
    osal_stream_default_accept,
    ioc_streamer_flush,
    osal_stream_default_seek,
    ioc_streamer_write,
    ioc_streamer_read,
    osal_stream_default_write_value,
    osal_stream_default_read_value,
    ioc_streamer_get_parameter,
    osal_stream_default_set_parameter,
    osal_stream_default_select};

#endif
