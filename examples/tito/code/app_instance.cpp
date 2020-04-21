/**

  @file    app_instance.cpp
  @brief   IO controller application's base class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "app_main.h"

/* Forward referred static functions.
 */
static osalStatus app_gina1_photo_received(
    struct iocBrickBuffer *b,
    void *context);


/**
****************************************************************************************************

  @brief Constructor.

  Save IO device network topology related stuff and atart running application for this
  IO device network in own thread.

  @return  None.

****************************************************************************************************
*/
AppInstance::AppInstance()
{
}


/**
****************************************************************************************************

  @brief Virtual destructor.

  Join worker thread to this thread and clean up.

  @return  None.

****************************************************************************************************
*/
AppInstance::~AppInstance()
{
}


void AppInstance::start(const os_char *network_name, os_uint device_nr)
{
    // initialize(network_name, device_nr);
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);

    m_gina1_def = m_gina1.inititalize(m_network_name, 1);
    m_gina2_def = m_gina2.inititalize(m_network_name, 2);

    ioc_set_brick_received_callback(&m_gina1.m_camera_buffer, app_gina1_photo_received, this);
    ioc_brick_set_receive(&m_gina1.m_camera_buffer, OS_TRUE);

    m_test_seq1.start(this);
}

void AppInstance::stop()
{
    m_test_seq1.stop();
}

void AppInstance::run()
{
    ioc_receive(&m_gina1.m_gina_export);
    ioc_receive(&m_gina2.m_gina_export);
    ioc_run_brick_receive(&m_gina1.m_camera_buffer);
    ioc_send(&m_gina1.m_gina_export);
    ioc_send(&m_gina2.m_gina_export);
    ioc_send(&m_gina1.m_gina_import);
    ioc_send(&m_gina2.m_gina_import);
}



static osalStatus app_gina1_photo_received(
    struct iocBrickBuffer *b,
    void *context)
{

    return OSAL_SUCCESS;
}
