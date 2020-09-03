/**

  @file    application_cameras.cpp
  @brief   Application class'es camera support code.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    1.9.2020

  The camera object derived from abstract camera can be either run by application's main
  thread or as independent thread.
  - To run from other thread: Call add_mblks(), setup_camera() and turn_camera_on_or_off() to
    once to get everything setup, then call run() repeatedly to move data. Calling thread
    is responsivle for sending dexp and receiving dimp memory blocks.
  - To run as individual thread. Set up is the same: add_mblks(), setup_camera() and
    turn_camera_on_or_off(). But instead of run() call start_thread() function once.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iodevice.h"
#if PINS_CAMERA

using IoDevice::AbstractCamera;

/* Forward referred callback function.
 */
static void iocom_camera_callback(
    struct pinsPhoto *photo,
    void *context);

static void iocom_camera_command_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context);

static void iocom_camera_thread_starter(
    void *prm,
    osalEvent done);


AbstractCamera::AbstractCamera()
{
#if OSAL_MULTITHREAD_SUPPORT
    m_started = OS_FALSE;
    m_event = OS_NULL;
#endif
}

AbstractCamera::~AbstractCamera()
{
#if OSAL_MULTITHREAD_SUPPORT
    stop_thread();
#endif
}

/**
****************************************************************************************************

  @brief Create memory blocks for this camera.

  The mblks function creates "dexp" and "dimp", etc, memory blocks for camera data transfer
  and sets memory block handle for signals associated with camera data transfer.

  This function can and should be called after signal structure has been initialized
  (*_init_signal_struct()), but before calling setup_camera().

  @param   device_name Device name, like "buster".
  @param   device_number Number of the buster device.
  @param   network_name Network name the buster is connected to, like "iocafenet".
  @param   exp_mblk_name Name of the camera data transfer memory block to be exported by Buster,
           "dexp", etc.
  @param   exp_mblk_signal_hdr Pointer to signal header for "dexp" memory block. Handle
           is set for these signals.
  @param   exp_mblk_sz Exported memory block size in bytes.

  @param   imp_mblk_name Name of the camera data transfer memory block to be imported by Buster,
           "dimp", etc. This controls data transfer.
  @param   imp_mblk_signal_hdr Pointer to signal header for "dimp" memory block. Handle
           is set for these signals.
  @param   imp_mblk_sz Imported memory block size in bytes.

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
    blockprm.flags = IOC_MBLK_UP|IOC_FLOOR;
    ioc_initialize_memory_block(&m_dexp, OS_NULL, root, &blockprm);
    m_dexp.mblk->signal_hdr = exp_mblk_signal_hdr;

    blockprm.mblk_name = imp_mblk_name;
    blockprm.nbytes = imp_mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN|IOC_FLOOR;
    ioc_initialize_memory_block(&m_dimp, OS_NULL, root, &blockprm);
    m_dimp.mblk->signal_hdr = imp_mblk_signal_hdr;

    ioc_set_handle_to_signals(exp_mblk_signal_hdr, &m_dexp);
    ioc_set_handle_to_signals(imp_mblk_signal_hdr, &m_dimp);
}


/**
****************************************************************************************************

  @brief Set up cameras.

  The setup_camera function sets up camera data sturctures for use, but doesn't yet connect to
  actual camera.

  @param   iface Camera interface (pointer to structure with camera API function pointers).
  @param   sigs Structure which specifies signals for transferring camera data.
  @param   pin Camera pin. This can be used to pass hardware specific parameters from pins
           JSON configuration file to camera driver.
  @param   root Pointer to IOCOM root object.
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


/**
****************************************************************************************************

  @brief Set up cameras.

  The configure function should be owerridden by application to transfer camera parameters
  to camera. The function here is just placeholder to generate message if function is
  unimplemented.

  @return  None.

****************************************************************************************************
*/
void AbstractCamera::configure()
{
    osal_debug_error("configure_camera not overridden?");
}


/**
****************************************************************************************************

  @brief Run camera data transfer.

  The run function needs to be called repeatedy to keep camera data transfer alive,
  if own thread is nor created for the camera.

  @return  None.

****************************************************************************************************
*/
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

#if OSAL_MULTITHREAD_SUPPORT
        if (m_event) {
            osal_event_set(m_event);
        }
#endif
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

  @param   turn_on OS_TRUE to start the camera, OS_FALSE to stop it.
  @return  None.

****************************************************************************************************
*/
void AbstractCamera::turn_camera_on_or_off(
    os_boolean turn_on)
{
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


#if OSAL_MULTITHREAD_SUPPORT
void AbstractCamera::start_thread()
{
    if (m_started) return;

    m_event = osal_event_create();

    /* Start running test_sequence for this IO device network in own thread.
     */
    m_stop_thread = OS_FALSE;
    m_thread = osal_thread_create(iocom_camera_thread_starter, this,
        OS_NULL, OSAL_THREAD_ATTACHED);
    m_started = OS_TRUE;
}


/* Join worker thread to this thread.
 */
void AbstractCamera::stop_thread()
{
    if (!m_started) return;

    turn_camera_on_or_off(OS_FALSE);

    m_stop_thread = OS_TRUE;
    osal_event_set(m_event);
    osal_thread_join(m_thread);

    osal_event_delete(m_event);
    m_started = OS_FALSE;
}

void AbstractCamera::processing_thread(
    osalEvent done)
{
    osal_event_set(done);

    /* Call communication_callback() when data is received, etc..
     */
    ioc_add_callback(&m_dimp, iocom_camera_command_callback, this);

    osal_event_set(m_event);
//    os_timer ti;
    while (!m_stop_thread && osal_go())
    {
        ioc_receive(&m_dimp);
  //      os_get_timer(&ti);

        run();
        ioc_send(&m_dexp);
        os_timeslice();
    }


    ioc_remove_callback(&m_dimp, iocom_camera_command_callback, this);
}


void AbstractCamera::camera_command_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags)
{
    osal_event_set(m_event);
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


static void iocom_camera_command_callback(
    struct iocHandle *handle,
    os_int start_addr,
    os_int end_addr,
    os_ushort flags,
    void *context)
{
    /* Ignore other than data received callbackss.
     */
    if ((flags & IOC_MBLK_CALLBACK_RECEIVE) == 0) return;

    AbstractCamera *cam = (AbstractCamera*)context;
    cam->camera_command_callback(handle, start_addr, end_addr, flags);
}

static void iocom_camera_thread_starter(void *prm, osalEvent done)
{
    AbstractCamera *cam = (AbstractCamera*)prm;
    osal_thread_set_priority(OSAL_THREAD_PRIORITY_LOW);
    cam->processing_thread(done);
}

#endif


#endif
