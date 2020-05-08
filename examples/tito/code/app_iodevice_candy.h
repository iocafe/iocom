/**

  @file    app_iodevice_candy.g
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

/**
****************************************************************************************************
  IO device interface wrapper class.
****************************************************************************************************
*/
class CandyIoDevice : public AppIoDevice
{
public:
    /* Constructor and virtual destructor.
     */
    CandyIoDevice();
    virtual ~CandyIoDevice();

    candy_t *inititalize(const os_char *network_name, os_uint device_nr);
    virtual void release();

    os_boolean
        m_initialized;

    /* Memory block handles.
     */
    iocHandle
        m_candy_export,
        m_candy_import,
        m_candy_conf_export,
        m_candy_conf_import;

    /* Candy IO definition structure.
     */
    candy_t
        m_candy_def;

    /* Buffer for incoming camera photo.
     */
    iocBrickBuffer
        m_camera_buffer;
};
