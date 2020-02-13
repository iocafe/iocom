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

/* Store memory block handle pointer within all signals in signal structure.
 */
void ioc_set_handle_to_signals(
    iocMblkSignalHdr *mblk_hdr,
    iocHandle *handle);

/* Get data from persistent block or from file.
 */
osalStatus osal_get_persistent_block_or_file(
    osPersistentBlockNr default_block_nr,
    const os_char *dir,
    const os_char *file_name,
    os_char **buf,
    os_memsz *n_read,
    os_int flags);
