/**

  @file    gina_io_device.cpp
  @brief   Wrapper representing Gina IO device interface.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.11.2019

  Copyright 2012 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "tito.h"


/**
****************************************************************************************************

  @brief Constructor.

  Set IO device name and mark this object uninitialized.

  @return  None.

****************************************************************************************************
*/
TitoGinaIoDevice::TitoGinaIoDevice() : TitoIoDevice()
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
TitoGinaIoDevice::~TitoGinaIoDevice()
{
    release();
}


void doit(iocMblkSignalHdr *mblk_hdr, iocHandle *handle)
{
    iocSignal *sig;
    os_int count;

    // mblk_hdr->handle = handle;

    count = mblk_hdr->n_signals;
    sig = mblk_hdr->first_signal;

    while (count--)
    {
        sig->handle = handle;
        sig++;
    }
}


gina_t *TitoGinaIoDevice::inititalize(const os_char *network_name, os_short device_nr)
{
    iocMemoryBlockParams blockprm;

    if (m_initialized) return &m_gina_def;

    m_device_nr = device_nr;

    /* Setup initial Gina IO board definition structure.
     */
    gina_init_signal_struct(&m_gina_def);


    /* Dynamic conf: put data from json in
     */

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = m_device_name;
    blockprm.device_nr = m_device_nr;
    blockprm.network_name = network_name;

    blockprm.mblk_nr = IOC_DEV_EXPORT_MBLK;
    blockprm.mblk_name = m_gina_def.exp.hdr.mblk_name;
    blockprm.nbytes = m_gina_def.exp.hdr.mblk_sz;
    blockprm.flags = IOC_TARGET|IOC_AUTO_SYNC /* |IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_gina_export, OS_NULL, &tito_root, &blockprm);

    blockprm.mblk_nr = IOC_DEV_IMPORT_MBLK;
    blockprm.mblk_name = m_gina_def.imp.hdr.mblk_name;
    blockprm.nbytes = m_gina_def.imp.hdr.mblk_sz;
    blockprm.flags = IOC_SOURCE|IOC_AUTO_SYNC /* |IOC_ALLOW_RESIZE */;
    ioc_initialize_memory_block(&m_gina_import, OS_NULL, &tito_root, &blockprm);


doit(&m_gina_def.imp.hdr, &m_gina_import);
doit(&m_gina_def.exp.hdr, &m_gina_export);

    /* Set callback to detect received data and connection status changes.
     */
    // ioc_add_callback(&ctx.inputs, iocontroller_callback, &ctx);

    m_initialized = OS_TRUE;

    return &m_gina_def;
}


void TitoGinaIoDevice::release()
{
    if (!m_initialized) return;
    ioc_release_memory_block(&m_gina_export);
    ioc_release_memory_block(&m_gina_import);

    m_initialized = OS_FALSE;
}
