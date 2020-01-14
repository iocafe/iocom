/**

  @file    ioc_persistent_mblk.c
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
#include "ioserver.h"

/**
****************************************************************************************************

  @brief Load persistent data as memory block content.

  The ioc_load_persistent_into_mblk() function loads persistent data....
  Resizes memory block

  - Memory block must have IOC_ALLOW_RESIZE flag.

  @param   node Node (IO device) configuration to set up.
  @param   select Persisent block number to load.
  @param   default_data Default content to store to memory block if loading fails.
  @param   default_data_sz Default content size in bytes.
  @return  OSAL_STATUS_COMPLETED: The memory block succesfully loaded from persistent storage.
           OSAL_SUCCESS: Default data given as argument was used.
           Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_load_persistent_into_mblk(
    iocHandle *handle,
    os_int select,
    const os_char *default_data,
    os_memsz default_data_sz)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    const os_char *block = OS_NULL;
    os_memsz block_sz, n_read = -1;
    osalStatus s;
    osPersistentHandle *h = OS_NULL;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(handle, &root);
    if (mblk == OS_NULL) return OSAL_STATUS_FAILED;

    /* If persistant storage is in micro-controller's flash, we can just get pointer to data block
       and data size.
     */
    s = os_persistent_get_ptr(select, &block, &block_sz);
    if (s != OSAL_SUCCESS)
    {
        /* No success with direct pointer to flash, try loading from persisten storage.
         */
        h = os_persistent_open(OS_PBNR_CONFIG, &block_sz, OSAL_STREAM_READ);

        /* If no success with persistent storage.
         */
        if (h == OS_NULL || block_sz <= 0)
        {
            h = OS_NULL;
            block = default_data;
            block_sz = default_data_sz;
        }
    }

    /* CHECK WHAT TO DO WITH OPEN SOURCE/TARGET BUFFERS IF BLOCK SIZE IS INCREASED.
     */
    s = ioc_resize_mblk(mblk, (os_int)block_sz, IOC_DISCONNECT_MBLK_ON_RESIZE);
    if (s != OSAL_SUCCESS && s != OSAL_STATUS_COMPLETED)
    {
        osal_debug_error("resizing memory block failed");
        goto getout;
    }

    if (h)
    {
        n_read = os_persistent_read(h, mblk->buf, block_sz);
        os_persistent_close(h, OSAL_STREAM_DEFAULT);
    }

    /* If we didn't read from persistent storage using handle, copy either from
       direct pointer to flash or default data.
     */
    if (n_read != block_sz)
    {
        os_memcpy(mblk->buf, block, block_sz);
    }

    /* If we need to transfer the changes?
     */
    ioc_mblk_invalidate(mblk, 0, block_sz-1);

getout:
    /* End syncronization.
     */
    ioc_unlock(root);
    return s;
}
