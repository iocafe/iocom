/**

  @file    ioc_server_util.c
  @brief   Server side helper functions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "ioserver.h"

/**
****************************************************************************************************

  @brief Store memory block handle pointer within all signals in signal structure.
  @anchor ioc_set_handle_to_signals

  The ioc_set_handle_to_signals() function stores memory block handle pointer for all signals
  in signals sturcture.

  @param   mblk_hdr Header of signal structure for the memory block.
  @param   handle Memory block handle pointer to store.
  @return  None.

****************************************************************************************************
*/
void ioc_set_handle_to_signals(
    iocMblkSignalHdr *mblk_hdr,
    iocHandle *handle)
{
    iocSignal *sig;
    os_int count;

    mblk_hdr->handle = handle;

    count = mblk_hdr->n_signals;
    sig = mblk_hdr->first_signal;

    while (count--)
    {
        sig->handle = handle;
        sig++;
    }
}


/**
****************************************************************************************************

  @brief Get data from persistent block or from file.
  @anchor osal_get_persistent_block_or_file

  The osal_get_persistent_block_or_file() function reads either a file (if we have file system
  support) or persistent memory block, depending on file_name.

  This is used by basic server to get client certificate chain to set up and IO device.

  Buffer may be allocated by os_malloc and must be released by calling os_free(buf, n_read) if
  the function returns OSAL_MEMORY_ALLOCATED.

  @param   default_block_nr If reading from persistent storage, this is default block
           number for the case when file name doesn't specify one.
  @param   dir Directory from where files are read, if using file system.
  @param   file_name Specifies file name or persistent block number.
  @param   buf Where to store pointer to data buffer.
  @param   n_read Number of data bytes in buffer.
  @param   flags OS_FILE_NULL_CHAR to terminate buffer with NULL character.
           OS_FILE_DEFAULT for default operation.
  @return  OSAL_SUCCESS or OSAL_MEMORY_ALLOCATED if data was loaded. OSAL_MEMORY_ALLOCATED
           return code means that memory was allocated and must be released by os_free().
           Other return values indicate that it was not loaded (missing or error).

****************************************************************************************************
*/
osalStatus osal_get_persistent_block_or_file(
    osPersistentBlockNr default_block_nr,
    const os_char *dir,
    const os_char *file_name,
    os_char **buf,
    os_memsz *n_read,
    os_int flags)
{
    osPersistentBlockNr block_nr;
    osalStatus s;
    os_char *block;

#if OSAL_FILESYS_SUPPORT
    os_char path[OSAL_PATH_SZ];

    *buf = OS_NULL;

    /* If we have file name which doesn't start with number, we will read from file.
     */
    if (file_name) if (!osal_char_isdigit(*file_name) && *file_name != '\0')
    {
        if (dir == OS_NULL)
        {
            dir = OSAL_FS_ROOT "coderoot/eosal/extensions/tls/keys-and-certs/";
        }

        os_strncpy(path, dir, sizeof(path));
        os_strncat(path, file_name, sizeof(path));

        *buf = os_read_file_alloc(path, n_read, flags);
        if (*buf) return OSAL_MEMORY_ALLOCATED;

        osal_debug_error_str("bserver: reading file failed ", path);
        return OSAL_STATUS_FAILED;
    }
#endif

    block_nr = (osPersistentBlockNr)osal_str_to_int(file_name, OS_NULL);
    if (block_nr == 0) block_nr = default_block_nr;

    s = os_load_persistent_malloc(block_nr, &block, n_read);
    if (s != OSAL_SUCCESS && s != OSAL_MEMORY_ALLOCATED)
    {
        osal_debug_error_int("os_load_persistent_malloc failed ", block_nr);
        return OSAL_STATUS_FAILED;
    }
    *buf = block;
    return s;
}
