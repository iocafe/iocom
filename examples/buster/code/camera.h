/**

  @file    camera.h
  @brief   Buster's camera class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.9.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_CAMERA_H_
#define IOC_CAMERA_H_
#include "buster.h"
#if PINS_CAMERA

class Camera : public AbstractCamera
{
public:
    virtual void configure(void);
};

#endif
#endif
