/**

  @file    tito_io_device.cpp
  @brief   Wrapper representing IO device interface.
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

  X...

  @return  None.

****************************************************************************************************
*/
TitoIoDevice::TitoIoDevice(const os_char *device_name, os_short device_nr)
{
    /* Save IO device network topology related stuff.
     */
    // os_strncpy(m_device_name, "tito", IOC_NAME_SZ);
    // os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);
    // m_device_nr = device_nr;


    /* Set up static IO boards, for now two gina boards 1 and 2
     */

    gina_init_prm_t gina_prm;

    os_memclear(&gina_prm, sizeof(gina_prm));
    // gina_prm.up = ?;

    m_io_device_hdr = (iocDeviceHdr*)os_malloc(sizeof(gina_t), OS_NULL);
    gina_init_signal_struct((gina_t*)m_io_device_hdr, &gina_prm);
    // Dynamic: merge received JSON in.

    /* Generate memory blocks
     */

    /* Put memory blocks handle into signals
     */



}


/**
****************************************************************************************************

  @brief Virtual destructor.

  X...

  @return  None.

****************************************************************************************************
*/
TitoIoDevice::~TitoIoDevice()
{

    os_free(m_io_device_hdr, sizeof(gina_t));
}


