/**

  @file    app_iodevice_gina.cpp
  @brief   Wrapper representing Gina IO device interface.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "app_main.h"


/**
****************************************************************************************************

  @brief Constructor.

  Set IO device name and mark this object uninitialized.

  @return  None.

****************************************************************************************************
*/
GinaIoDevice::GinaIoDevice() : AppIoDevice()
{
    os_strncpy(m_device_name, "gina", IOC_NAME_SZ);
    m_initialized = OS_FALSE;
}


/**
****************************************************************************************************

  @brief Release any resources allocated for this object.

  X...

  @return  None.

****************************************************************************************************
*/
GinaIoDevice::~GinaIoDevice()
{
    release();
}


gina_t *GinaIoDevice::inititalize(const os_char *network_name, os_uint device_nr)
{
    iocMemoryBlockParams blockprm;

    if (m_initialized) return &m_gina_def;

    m_device_nr = device_nr;

    /* Setup initial Gina IO board definition structure.
     */
    gina_init_signal_struct(&m_gina_def);

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = m_device_name;
    blockprm.device_nr = m_device_nr;
    blockprm.network_name = network_name;

    blockprm.mblk_name = m_gina_def.exp.hdr.mblk_name;
    blockprm.nbytes = m_gina_def.exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP /* |IOC_AUTO_SYNC|IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_gina_export, OS_NULL, &app_iocom_root, &blockprm);

    blockprm.mblk_name = m_gina_def.imp.hdr.mblk_name;
    blockprm.nbytes = m_gina_def.imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN /* |IOC_AUTO_SYNC|IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_gina_import, OS_NULL, &app_iocom_root, &blockprm);

    ioc_set_handle_to_signals(&m_gina_def.imp.hdr, &m_gina_import);
    ioc_set_handle_to_signals(&m_gina_def.exp.hdr, &m_gina_export);

    /* Set up buffer for incoming camera photo
     */
    ioc_initialize_brick_buffer(&m_camera_buffer, &m_gina_def.ccd,
        &app_iocom_root, -1, IOC_BRICK_CONTROLLER);

    /* Set callback to detect received data and connection status changes.
     */
    // ioc_add_callback(&ctx.inputs, iocontroller_callback, &ctx);

    m_initialized = OS_TRUE;

    return &m_gina_def;
}


void GinaIoDevice::release()
{
    if (!m_initialized) return;
    ioc_release_memory_block(&m_gina_export);
    ioc_release_memory_block(&m_gina_import);

    m_initialized = OS_FALSE;
}
