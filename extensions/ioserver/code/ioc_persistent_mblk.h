/**

  @file    ioc_persistent_mblk.h
  @brief   Load persistent data block as memory block content.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Load persistent data as memory block content.
 */
osalStatus ioc_load_persistent_into_mblk(
    iocHandle *handle,
    os_int select,
    const os_uchar *default_data,
    os_memsz default_data_sz);
