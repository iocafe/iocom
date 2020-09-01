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

        virtual void setup_camera(
            const pinsCameraInterface *iface,
            const iocStreamerSignals *sigs,
            const Pin *pin,
            iocRoot *root);

        virtual void configure();
        virtual void start();
        virtual void run();
        virtual void callback(pinsPhoto *photo);

        void set_camera_prm(
            pinsCameraParamIx ix,
            const iocSignal *sig);

        void get_camera_prm(
            pinsCameraParamIx ix,
            const iocSignal *sig);

        virtual void turn_camera_on_or_off(void);

        /* Camera state and camera output */
        pinsCamera m_pins_camera;
        iocBrickBuffer m_video_output;

        /* Camera control parameter has changed, camera on/off */
        os_boolean m_camera_on_or_off;
        os_boolean m_camera_is_on;

        const pinsCameraInterface *m_iface;

        iocHandle m_dexp, m_dimp;
    };
}

#endif
#endif
