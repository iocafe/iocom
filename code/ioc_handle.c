/**

  @file    ioc_handle.c
  @brief   Memory block handle object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Handles are used instead of direct pointers to enable deleting memory blocks fron other thread
  than one using them. The same handle class could be used for other purposes.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

/* For testing
 */
#define IOC_VALIDATE_HANDLE 0
#if IOC_VALIDATE_HANDLE
    static void ioc_validate_handle(iocHandle *handle);
#else
    #define ioc_validate_handle(h)
#endif

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
        handle->flags = 0;
        handle->next = handle->prev = handle;
        IOC_SET_DEBUG_ID(handle, 'H')
        return;
    }

    /* If we seting up the handle structure within memory block.
     */
    if (handle == &mblk->handle)
    {
        handle->next = handle->prev = handle;
    }

    /* Otherwise setting up a chained handle
     */
    else
    {
        ioc_validate_handle(&mblk->handle);
        handle->prev = mblk->handle.prev;
        handle->next = &mblk->handle;
        mblk->handle.prev = handle;
        handle->prev->next = handle;
    }

    handle->flags = mblk->flags;
    osal_debug_assert(handle->flags != 0);

    IOC_SET_DEBUG_ID(handle, 'H')
    ioc_validate_handle(handle);
}

/* Release a memory block handle (calls synchronization).
 */
void ioc_release_handle(
    iocHandle *handle)
{
    if (handle->root == OS_NULL) return;

    ioc_lock(handle->root);
    ioc_validate_handle(handle);

    if (handle->next != handle)
    {
        handle->prev->next = handle->next;
        handle->next->prev = handle->prev;
        handle->next = handle->prev = handle;
    }
    handle->mblk = OS_NULL;
    /* root must not be zeroed */

    ioc_unlock(handle->root);

    /* Mark handle structure finished with small h
     */
    IOC_SET_DEBUG_ID(handle, 'h')
}


/* Duplicate a memory block handle (calls synchronization).
 */
void ioc_duplicate_handle(
    iocHandle *handle,
    iocHandle *source_handle)
{
    iocRoot *root;

    /* Check that mblk is valid pointer.
     */
    osal_debug_assert(source_handle->debug_id == 'H');

    ioc_validate_handle(handle);
    root = source_handle->root;
    if (root)
    {
        ioc_lock(root);
        ioc_setup_handle(handle, root, source_handle->mblk);
        ioc_unlock(root);
    }
}


#if IOC_VALIDATE_HANDLE
/* Called when memory block is deleted (synchronization lock must be on).
 */
void ioc_validate_handle(
    iocHandle *handle)
{
    iocHandle *h;

    osal_debug_assert(handle->debug_id == 'H');

    h = handle;
    while (OS_TRUE) {
        osal_debug_assert(h->debug_id == 'H');
        osal_debug_assert(h->next->debug_id == 'H');
        osal_debug_assert(h->prev->debug_id == 'H');
        if (h->next == handle) break;
        osal_debug_assert(h->prev->next == h);
        osal_debug_assert(h->next->prev == h);
        h = h->next;
    }
}
#endif

/* Called when memory block is deleted (synchronization lock must be on).
 */
void ioc_terminate_handles(
    iocHandle *handle)
{
    iocHandle *h, *nexth;

    /* Check that mblk is valid pointer.
     */
    ioc_validate_handle(handle);
    osal_debug_assert(handle->debug_id == 'H');

    h = handle;
    do {
        nexth = h->next;
        h->next = h->prev = h;
        h->mblk = OS_NULL;
        h = nexth;
    }
    while (h != handle);

    /* Mark handle structure finished with small h
     */
    IOC_SET_DEBUG_ID(handle, 'h')
}


/* Get memory block pointer from handle and enter synchronization lock.
 * If memory block no longer exists, lock is left off and proot pointer is set to NULL.
 */
struct iocMemoryBlock *ioc_handle_lock_to_mblk(
    iocHandle *handle,
    struct iocRoot **proot)
{
    iocRoot *root;
    iocMemoryBlock *mblk;

    /* Get root. Return root pointer if needed.
     */
    root = handle->root;
    if (proot) *proot = root;
    if (root == OS_NULL)
    {
        return OS_NULL;
    }

    /* Check that mblk is valid pointer.
     */
    osal_debug_assert(handle->debug_id == 'H');

    /* Synchronize.
     */
    ioc_lock(root);
    ioc_validate_handle(handle);

    /* Get memory block pointer. If none, unlock and return NULL to indicate failure.
     */
    mblk = handle->mblk;
    if (mblk == OS_NULL)
    {
        ioc_unlock(root);
        if (proot) *proot = OS_NULL;
        return OS_NULL;
    }

    /* Check that mblk is valid pointer.
     */
    osal_debug_assert(mblk->debug_id == 'M');
    return mblk;
}
