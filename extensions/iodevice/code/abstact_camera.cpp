/**

  @file    application_cameras.cpp
  @brief   Application class'es camera support code.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    1.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iodevice.h"
#if PINS_CAMERA

using IoDevice::AbstractCamera;


static void iocom_camera_callback(
    struct pinsPhoto *photo,
    void *context);


/**
****************************************************************************************************

  @brief Create memory blocks for this camera.

  The mblks function...
  @return  None.

****************************************************************************************************
*/
void AbstractCamera::add_mblks(
    const os_char *device_name,
    os_int device_nr,
    const os_char *network_name,
    const os_char *exp_mblk_name,
    iocMblkSignalHdr *exp_mblk_signal_hdr,
    os_memsz exp_mblk_sz,
    const os_char *imp_mblk_name,
    iocMblkSignalHdr *imp_mblk_signal_hdr,
    os_memsz imp_mblk_sz,
    iocRoot *root)
{
    iocMemoryBlockParams blockprm;
    os_memclear(&blockprm, sizeof(blockprm));

    blockprm.device_name = device_name;
    blockprm.device_nr = device_nr;
    blockprm.network_name = network_name;

    blockprm.mblk_name = exp_mblk_name;
    blockprm.nbytes = exp_mblk_sz;
    blockprm.flags = IOC_MBLK_UP;
    ioc_initialize_memory_block(&m_dexp, OS_NULL, root, &blockprm);
    m_dexp.mblk->signal_hdr = exp_mblk_signal_hdr;

    blockprm.mblk_name = imp_mblk_name;
    blockprm.nbytes = imp_mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN;
    ioc_initialize_memory_block(&m_dimp, OS_NULL, root, &blockprm);
    m_dimp.mblk->signal_hdr = imp_mblk_signal_hdr;
}


/**
****************************************************************************************************

  @brief Set up cameras.

  The camera_callback function is called when a camera frame is captured.
  If video transfer buffer is empty and vido output stream is open, the camera data is  moved
  to video outout buffer. Othervise camera data is dropped.

  @param   photo Pointer to a frame captured by camera.
  @return  None.

****************************************************************************************************
*/
void AbstractCamera::setup_camera(
    const pinsCameraInterface *iface,
    const iocStreamerSignals *sigs,
    const Pin *pin,
    iocRoot *root)
{
    m_iface = iface;

    /* Set up video output stream and the camera
     */
    ioc_initialize_brick_buffer(&m_video_output, sigs,
        root, 4000, IOC_BRICK_DEVICE);

    pinsCameraParams camera_prm;
    iface->initialize();
    os_memclear(&camera_prm, sizeof(camera_prm));
    camera_prm.camera_pin = pin;
    camera_prm.callback_func = iocom_camera_callback;
    camera_prm.callback_context = this;
    iface->open(&m_pins_camera, &camera_prm);
    configure();

    m_camera_on_or_off = m_camera_is_on = OS_FALSE;
}


void AbstractCamera::configure()
{
    osal_debug_error("configure_camera not overridden?");
}


void AbstractCamera::start()
{
    turn_camera_on_or_off();
}

void AbstractCamera::run()
{
    ioc_run_brick_send(&m_video_output);
}


/**
****************************************************************************************************

  @brief "New frame from camera" callback.

  The callback function is called when a camera frame is captured.
  If video transfer buffer is empty and vido output stream is open, the camera data is  moved
  to video outout buffer. Othervise camera data is dropped.

  @param   photo Pointer to a frame captured by camera.
  @return  None.

****************************************************************************************************
*/
void AbstractCamera::callback(
    pinsPhoto *photo)
{
    if (ioc_ready_for_new_brick(&m_video_output) && ioc_is_brick_connected(&m_video_output))
    {
        pins_store_photo_as_brick(photo, &m_video_output, IOC_DEFAULT_COMPRESSION);
    }
}


/**
****************************************************************************************************

  @brief Configure one camera parameter.

  The set_camera_prm function sets a camera parameter to camera API wrapper. The
  value to set is taken from a signal in "exp" memory block.

  @param   ix Camera parameter index, like PINS_CAM_BRIGHTNESS.
  @param   sig Pointer to signal in "exp" memory block.
  @return  None.

****************************************************************************************************
*/
void AbstractCamera::set_camera_prm(
    pinsCameraParamIx ix,
    const iocSignal *sig)
{
    os_long x;
    os_char state_bits;

    x = ioc_get_ext(sig, &state_bits, IOC_SIGNAL_NO_TBUF_CHECK);
    if (state_bits & OSAL_STATE_CONNECTED) {
        m_iface->set_parameter(&m_pins_camera, ix, x);
    }
}


/**
****************************************************************************************************

  @brief Get camera parameter from camera driver.

  The get_camera_prm function reads a camera parameter from camera wrapper and
  stores the value in signal in "exp" memory block.

  @param   ix Camera parameter index, like PINS_CAM_BRIGHTNESS.
  @param   sig Pointer to signal in "exp" memory block.
  @return  None.

****************************************************************************************************
*/
void AbstractCamera::get_camera_prm(
    pinsCameraParamIx ix,
    const iocSignal *sig)
{
    os_long x;
    x = m_iface->get_parameter(&m_pins_camera, ix);
    ioc_set(sig, x);
}


/**
****************************************************************************************************

  @brief Turn camera on/off.

  The ioapp_turn_camera_on_or_off function calls pins library to start or stop the camera.
  @return  None.

****************************************************************************************************
*/
void AbstractCamera::turn_camera_on_or_off(void)
{
    os_boolean turn_on;

    // turn_on = (os_boolean)ioc_get(&buster.exp.on);
turn_on = 1;
    if (turn_on != m_camera_is_on) {
        if (turn_on) {
            m_iface->start(&m_pins_camera);
        }
        else {
            m_iface->stop(&m_pins_camera);
        }
        m_camera_is_on = turn_on;
    }
}


/**
****************************************************************************************************

  @brief "New frame from camera" callback.

  The iocom_application_camera_callback_1 function is called when a camera frame is captured.
  If video transfer buffer is empty and vido output stream is open, the camera data is  moved
  to video outout buffer. Othervise camera data is dropped.

  @param   photo Pointer to a frame captured by camera.
  @param   context Application context, not used (NULL).
  @return  None.

****************************************************************************************************
*/
static void iocom_camera_callback(
    struct pinsPhoto *photo,
    void *context)
{
    AbstractCamera *cam;
    cam = (AbstractCamera*)context;
    cam->callback(photo);
}

#endif
