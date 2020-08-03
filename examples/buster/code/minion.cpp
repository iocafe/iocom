/**

  @file    minion.cpp
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
#include "buster.h"


/**
****************************************************************************************************

  @brief Constructor.

  Set IO device name and mark this object uninitialized.

  @return  None.

****************************************************************************************************
*/
Minion::Minion() : AbstractSlaveDevice()
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
Minion::~Minion()
{
    release();
}


minion_t *Minion::inititalize(
    const os_char *network_name,
    os_uint device_nr)
{
#if 0
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
    blockprm.flags = IOC_MBLK_UP;
    ioc_initialize_memory_block(&m_gina_export, OS_NULL, &iocom_root, &blockprm);

    blockprm.mblk_name = m_gina_def.imp.hdr.mblk_name;
    blockprm.nbytes = m_gina_def.imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN;
    ioc_initialize_memory_block(&m_gina_import, OS_NULL, &iocom_root, &blockprm);

    blockprm.mblk_name = m_gina_def.conf_exp.hdr.mblk_name;
    blockprm.nbytes = m_gina_def.conf_exp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_UP;
    ioc_initialize_memory_block(&m_gina_conf_export, OS_NULL, &iocom_root, &blockprm);

    blockprm.mblk_name = m_gina_def.conf_imp.hdr.mblk_name;
    blockprm.nbytes = m_gina_def.conf_imp.hdr.mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN;
    ioc_initialize_memory_block(&m_gina_conf_import, OS_NULL, &iocom_root, &blockprm);

    /* These do store memory block handle for signals. Without this signals will
       not work from this program.
     */
    ioc_set_handle_to_signals(&m_gina_def.imp.hdr, &m_gina_import);
    ioc_set_handle_to_signals(&m_gina_def.exp.hdr, &m_gina_export);
    ioc_set_handle_to_signals(&m_gina_def.conf_imp.hdr, &m_gina_conf_import);
    ioc_set_handle_to_signals(&m_gina_def.conf_exp.hdr, &m_gina_conf_export);

#if IOC_DYNAMIC_MBLK_CODE
    /* These will store signal header pointer in memory block. This is necessary
       to clear OSAL_STATE_CONNECTED status bit when upper level, like I spy disconnects.
       See ioc_tbuf_disconnect_signals() function.
     */
    /* mblk_set_signal_header(&m_gina_import, &m_gina_def.imp.hdr);
    mblk_set_signal_header(&m_gina_export, &m_gina_def.exp.hdr);
    mblk_set_signal_header(&m_gina_conf_import, &m_gina_def.conf_imp.hdr);
    mblk_set_signal_header(&m_gina_conf_export, &m_gina_def.conf_exp.hdr); */
#endif

    /* Set up buffer for incoming camera photo
     */
    ioc_initialize_brick_buffer(&m_camera_buffer, &m_gina_def.camera,
        &iocom_root, -1, IOC_BRICK_CONTROLLER);

    /* Set callback to detect received data and connection status changes.
     */
    // ioc_add_callback(&ctx.inputs, iocontroller_callback, &ctx);

    m_initialized = OS_TRUE;
#endif

    return &m_minion_def;
}


void Minion::release()
{
    if (!m_initialized) return;
    ioc_release_brick_buffer(&m_camera_buffer);
    ioc_release_memory_block(&m_gina_export);
    ioc_release_memory_block(&m_gina_import);
    ioc_release_memory_block(&m_gina_conf_export);
    ioc_release_memory_block(&m_gina_conf_import);

    m_initialized = OS_FALSE;
}
