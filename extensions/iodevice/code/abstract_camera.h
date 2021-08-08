/**

  @file    abstract_camera.h
  @brief   Base class for camera connected to IO device.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    1.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_ABSTRACT_CAMERA_H_
#define IOC_ABSTRACT_CAMERA_H_
#include "iodevice.h"
#if PINS_CAMERA

namespace IoDevice
{
    /**
    ************************************************************************************************

      Abstract camera class.

    ************************************************************************************************
    */
    class AbstractCamera
    {
    public:
        AbstractCamera();
        ~AbstractCamera();

        /* Create memory blocks for this camera's data transfer.
         */
        void add_mblks(
            const os_char *device_name,
            os_int device_nr,
            const os_char *network_name,
            const os_char *exp_mblk_name,
            iocMblkSignalHdr *exp_mblk_signal_hdr,
            os_memsz exp_mblk_sz,
            const os_char *imp_mblk_name,
            iocMblkSignalHdr *imp_mblk_signal_hdr,
            os_memsz imp_mblk_sz,
            iocRoot *root);

        /* Set up camera data sturctures for use.
         */
        virtual void setup_camera(
            const pinsCameraInterface *iface,
            const iocStreamerSignals *sigs,
            const Pin *pin,
            iocRoot *root);

        /* Turn camera on or off.
         */
        virtual void turn_camera_on_or_off(
            os_boolean turn_on);

        /* Set camera parameters, this function need to be overridden.
         */
        virtual void configure();

        /* Run camera data transfer when camera doesn't have it's own processing thread.
         */
        virtual void run();

        /* "New frame received from camera" callback. Override to do image processing.
         */
        virtual void callback(pinsPhoto *photo);

        /* Stop and close the camera.
         */
        virtual void close();

        /* Set one camera parameter.
         */
        void set_camera_prm(
            pinsCameraParamIx ix,
            const iocSignal *sig);

        /* Get camera parameter from camera wrapper.
         */
        void get_camera_prm(
            pinsCameraParamIx ix,
            const iocSignal *sig);

#if OSAL_MULTITHREAD_SUPPORT
        /* Start thread to run camera processing independently.
         */
        void start_thread();

        /* Stop camera processing thread.
         */
        void stop_thread();

        /* Camera processing thread function. Override this function to do image
           processing, etc, within processing thread.
         */
        virtual void processing_thread(
            osalEvent done);

        /* Callback when vider request command has been received trough IOCOM.
         */
        virtual void command_callback(
            struct iocHandle *handle,
            os_int start_addr,
            os_int end_addr,
            os_ushort flags);
#endif

        /* Pins library camera state structure.
         */
        pinsCamera m_pins_camera;

        /* Pins library camera wrapper interface (pointer to structure of API function
           pointers for a specific camera implementation.
         */
        const pinsCameraInterface *m_iface;

        /* Buffer for video output.
         */
        iocBrickBuffer m_video_output;

        /* Camera control parameter has changed, camera on/off.
         */
        // os_boolean m_camera_on_or_off;
        os_boolean m_camera_is_on;

        /* Exported and imported memory blocks.
         */
        iocHandle m_dexp, m_dimp;

#if OSAL_MULTITHREAD_SUPPORT
        /* Signal to trig camera processing thread to immediate action.
         */
        osalEvent m_event;

        /* Thread pointer for join later.
         */
        osalThread *m_thread;

        /* Set OS_TRUE to stop processing thread.
         */
        os_boolean m_stop_thread;

        /* Flag indicating that camera has been started.
         */
        os_boolean m_started;

        /* Camera information chain.
         */
        pinsCameraInfo *m_camera_info;

        /* Motion detection.
         */
        DetectMotion m_motion;
        MotionDetectionParameters m_motion_prm;
        MotionDetectionResults m_motion_res;
#endif
    };
}

#endif
#endif
