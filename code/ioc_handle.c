/**

  @file    ioc_handle.c
  @brief   Memory block handle object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    23.10.2019

  Handles are used instead of direct pointers to enable deleting memory blocks fron other thread
  than one using them. The same handle class could be used for other purposes.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/* Set up a memory block handle (synchronization lock must be on).
 */
void ioc_setup_handle(
    iocHandle *handle,
    struct iocRoot *root,
    iocMemoryBlock *mblk)
{
    handle->mblk = mblk;
    handle->root = root;

    if (mblk == OS_NULL)
    {
        handle->next = handle->prev = handle;
    }

    /* If we seting up the handle structure within memory block.
     */
    else if (handle == &mblk->handle)
    {
        handle->next = handle->prev = handle;
    }

    /* Otherwise setting up a chained handle
     */
    else
    {
        handle->prev = mblk->handle.prev;
        handle->next = &mblk->handle;
        mblk->handle.prev = handle;
        handle->prev->next = handle;
    }

    /* Mark handle structure for debugging.
     */
    IOC_SET_DEBUG_ID(handle, 'H')
}

/* Release a memory block handle (calls synchronization).
 */
void ioc_release_handle(
    iocHandle *handle)
{
    if (handle->root == OS_NULL) return;

    ioc_lock(handle->root);

    if (handle->next != handle->prev)
    {
        handle->prev->next = handle->next;
        handle->next->prev = handle->prev;
        handle->next = handle->prev = handle;
    }
    handle->mblk = OS_NULL;
    /* root must not be zeroed */

    ioc_unlock(handle->root);
}


/* Duplicate a memory block handle (calls synchronization).
 */
void ioc_duplicate_handle(
    iocHandle *handle,
    iocHandle *source_handle)
{
    iocRoot *root;

    root = source_handle->root;
    ioc_lock(root);
    ioc_setup_handle(handle, root, root ? source_handle->mblk : OS_NULL);
    ioc_unlock(root);
}


/* Called when memory block is deleted (synchronization lock must be on).
 */
void ioc_terminate_handles(
    iocHandle *handle)
{
    iocHandle *h, *nexth;

    h = handle;
    do {
        nexth = h->next;
        h->next = h->prev = h;
        h->mblk = OS_NULL;
        h->root = OS_NULL;
        h = nexth;
    }
    while (h != handle);
}


/* Get memory block pointer from handle and enter synchronization lock.
 * If memory block no longer exists, lock is left off.
 */
struct iocMemoryBlock *ioc_handle_lock_to_mblk(
    iocHandle *handle,
    struct iocRoot **proot)
{
    iocRoot *root;
    iocMemoryBlock *mblk;

    /* Check that mblk is valid pointer.
     */
    osal_debug_assert(handle->debug_id == 'H');

    /* Get root. Return root pointer if needed.
     */
    root = handle->root;
    if (root == OS_NULL)
    {
        return OS_NULL;
    }
    if (proot) *proot = root;

    /* Synchronize.
     */
    ioc_lock(root);

    /* Get memory block pointer. If none, unlock and return NULL to indicate failure.
     */
    mblk = handle->mblk;
    if (mblk == OS_NULL)
    {
        ioc_unlock(root);
        return OS_NULL;
    }

    /* Check that mblk is valid pointer.
     */
    osal_debug_assert(mblk->debug_id == 'M');
    return mblk;
}
