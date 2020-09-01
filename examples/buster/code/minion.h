/**

  @file    minion.h
  @brief   Wrapper representing Minion IO device interface.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_MINION_CTRL_H_
#define IOC_MINION_CTRL_H_
#include "buster.h"

/**
****************************************************************************************************
  IO device interface wrapper class.
****************************************************************************************************
*/
class Minion : public AbstractSlaveDevice
{
public:
    /* Constructor and virtual destructor.
     */
    Minion();
    virtual ~Minion();

    minion_t *inititalize(const os_char *network_name, os_uint device_nr);
    virtual void release();

    os_boolean
        m_initialized;

    /* Memory block handles.
     */
    iocHandle
        m_gina_export,
        m_gina_import,
        m_gina_conf_export,
        m_gina_conf_import;

    /* Gina IO definition structure.
     */
    minion_t
        m_minion_def;

    /* Buffer for incoming camera photo.
     */
    iocBrickBuffer
        m_camera_buffer;
};

#endif
