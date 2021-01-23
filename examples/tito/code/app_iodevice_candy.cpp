/**

  @file    app_iodevice_candy.cpp
  @brief   Wrapper representing Candy IO device interface.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "controller_main.h"


/**
****************************************************************************************************

  @brief Constructor.

  Set IO device name and mark this object uninitialized.

  @return  None.

****************************************************************************************************
*/
CandyIoDevice::CandyIoDevice() : AppIoDevice()
{
    os_strncpy(m_device_name, "candy", IOC_NAME_SZ);
    m_initialized = OS_FALSE;
}


/**
****************************************************************************************************

  @brief Release any resources allocated for this object.

  X...

  @return  None.

****************************************************************************************************
*/
CandyIoDevice::~CandyIoDevice()
{
    release();
}


candy_t *CandyIoDevice::inititalize(
    const os_char *network_name,
    os_uint device_nr)
{
    iocMemoryBlockParams blockprm;

    if (m_initialized) return &m_candy_def;

    m_device_nr = device_nr;

    /* Setup initial Candy IO board definition structure.
     */
    candy_init_signal_struct(&m_candy_def);

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    blockprm.device_name = m_device_name;
    blockprm.device_nr = m_device_nr;
    blockprm.network_name = network_name;
#endif

    blockprm.mblk_name = m_candy_def.exp.hdr.mblk_name;
    blockprm.nbytes = m_candy_def.exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP;
    ioc_initialize_memory_block(&m_candy_export, OS_NULL, &iocom_root, &blockprm);

    blockprm.mblk_name = m_candy_def.imp.hdr.mblk_name;
    blockprm.nbytes = m_candy_def.imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN;
    ioc_initialize_memory_block(&m_candy_import, OS_NULL, &iocom_root, &blockprm);

    blockprm.mblk_name = m_candy_def.conf_exp.hdr.mblk_name;
    blockprm.nbytes = m_candy_def.conf_exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP;
    ioc_initialize_memory_block(&m_candy_conf_export, OS_NULL, &iocom_root, &blockprm);

    blockprm.mblk_name = m_candy_def.conf_imp.hdr.mblk_name;
    blockprm.nbytes = m_candy_def.conf_imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN;
    ioc_initialize_memory_block(&m_candy_conf_import, OS_NULL, &iocom_root, &blockprm);

    /* These do store memory block handle for signals. Without this signals will
       not work from this program.
     */
    ioc_set_handle_to_signals(&m_candy_def.imp.hdr, &m_candy_import);
    ioc_set_handle_to_signals(&m_candy_def.exp.hdr, &m_candy_export);
    ioc_set_handle_to_signals(&m_candy_def.conf_imp.hdr, &m_candy_conf_import);
    ioc_set_handle_to_signals(&m_candy_def.conf_exp.hdr, &m_candy_conf_export);

#if IOC_DYNAMIC_MBLK_CODE
    /* These will store signal header pointer in memory block. This is necessary
       to clear OSAL_STATE_CONNECTED status bit when upper level, like I spy disconnects.
       See ioc_tbuf_disconnect_signals() function.
     */
    /* mblk_set_signal_header(&m_candy_import, &m_candy_def.imp.hdr);
    mblk_set_signal_header(&m_candy_export, &m_candy_def.exp.hdr);
    mblk_set_signal_header(&m_candy_conf_import, &m_candy_def.conf_imp.hdr);
    mblk_set_signal_header(&m_candy_conf_export, &m_candy_def.conf_exp.hdr); */
#endif

    /* Set up buffer for incoming camera photo
     */
    ioc_initialize_brick_buffer(&m_camera_buffer, &m_candy_def.camera,
        &iocom_root, -1, IOC_BRICK_CONTROLLER);

    /* Set callback to detect received data and connection status changes.
     */
    // ioc_add_callback(&ctx.inputs, iocontroller_callback, &ctx);

    m_initialized = OS_TRUE;

    return &m_candy_def;
}


void CandyIoDevice::release()
{
    if (!m_initialized) return;
    ioc_release_brick_buffer(&m_camera_buffer);
    ioc_release_memory_block(&m_candy_export);
    ioc_release_memory_block(&m_candy_import);
    ioc_release_memory_block(&m_candy_conf_export);
    ioc_release_memory_block(&m_candy_conf_import);

    m_initialized = OS_FALSE;
}
