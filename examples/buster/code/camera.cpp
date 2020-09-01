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
#include "buster.h"
#if PINS_CAMERA

/**
****************************************************************************************************

  @brief Configure camera.

  The configure_camera function sets all camera parameters from signals in
  "exp" memory block to camera API.
  @return  None.

****************************************************************************************************
*/
void Camera::configure(void)
{
#ifdef BUSTER_EXP_CAM_NR
    set_camera_prm(PINS_CAM_NR, &buster.exp.cam_nr);
#endif
#ifdef BUSTER_EXP_IMG_WIDTH
    set_camera_prm(PINS_CAM_IMG_WIDTH, &buster.exp.img_width);
    get_camera_prm(PINS_CAM_IMG_WIDTH, &buster.exp.img_width);
    get_camera_prm(PINS_CAM_IMG_HEIGHT, &buster.exp.img_height);
#endif
#ifdef BUSTER_EXP_IMG_HEIGHT
    set_camera_prm(PINS_CAM_IMG_HEIGHT, &buster.exp.img_height);
    get_camera_prm(PINS_CAM_IMG_WIDTH, &buster.exp.img_width);
    get_camera_prm(PINS_CAM_IMG_HEIGHT, &buster.exp.img_height);
#endif
#ifdef BUSTER_EXP_FRAMERATE
    set_camera_prm(PINS_CAM_FRAMERATE, &buster.exp.framerate);
#endif

#ifdef BUSTER_EXP_BRIGHTNESS
    set_camera_prm(PINS_CAM_BRIGHTNESS, &buster.exp.brightness);
#endif
#ifdef BUSTER_EXP_SATURATION
    set_camera_prm(PINS_CAM_SATURATION, &buster.exp.saturation);
#endif
}

#endif
