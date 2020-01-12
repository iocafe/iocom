/**

  @file    ioc_server_util.h
  @brief   Server side helper functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#if IOC_SERVER_EXTENSIONS

/* Store memory block handle pointer within all signals in signal structure.
 */
void ioc_set_handle_to_signals(
    iocMblkSignalHdr *mblk_hdr,
    iocHandle *handle);

#endif
