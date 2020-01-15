/**

  @file    app_iodevice_gina.g
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

/**
****************************************************************************************************
  IO device interface wrapper class.
****************************************************************************************************
*/
class GinaIoDevice : public AppIoDevice
{
public:
    /* Constructor and virtual destructor.
     */
    GinaIoDevice();
    virtual ~GinaIoDevice();

    gina_t *inititalize(const os_char *network_name, os_uint device_nr);
    virtual void release();

    os_boolean
        m_initialized;

    /* Memory block handles.
     */
    iocHandle
        m_gina_export,
        m_gina_import;

    /* Gina IO definition structure.
     */
    gina_t
        m_gina_def;
};
