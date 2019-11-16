/**

  @file    ioc_handle.h
  @brief   Memory block handle object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    23.10.2019

  Handles are used instead of direct pointers to enable deleting memory blocks fron other thread
  than one using them. The same handle class could be used for other purposes.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_HANDLE_INCLUDED
#define IOC_HANDLE_INCLUDED

struct iocRoot;
struct iocMemoryBlock;


/**
****************************************************************************************************
    Memory block handle structure.
****************************************************************************************************
*/
typedef struct iocHandle
{
    /** Debug identifier must be first item in the object structure. It is used to verify
        that a function argument is pointer to correct initialized object.
     */
    IOC_DEBUG_ID

    /** Memory block flags, like IOC_TARGET, IOC_SOURCE, IOC_AUTO_SYNC, IOC_ALLOW_RESIZE
     *  or IOC_STATIC 32
     */
    os_short flags;

    struct iocRoot *root;
    struct iocMemoryBlock *mblk;
    struct iocHandle *next, *prev;
} 
iocHandle;


/**
****************************************************************************************************
    Handle functions.
****************************************************************************************************
*/

/* Set up a memory block handle (synchronization lock must be on).
 */
void ioc_setup_handle(
    iocHandle *handle,
    struct iocRoot *root,
    struct iocMemoryBlock *mblk);

/* Release a memory block handle (calls synchronization).
 */
void ioc_release_handle(
    iocHandle *handle);

/* Duplicate a memory block handle (calls synchronization).
 */
void ioc_duplicate_handle(
    iocHandle *handle,
    iocHandle *source_handle);

/* Called when memory block is deleted (synchronization lock must be on).
 */
void ioc_terminate_handles(
    iocHandle *handle);

/* Get memory block pointer from handle and enter synchronization lock.
 */
struct iocMemoryBlock *ioc_handle_lock_to_mblk(
    iocHandle *handle,
    struct iocRoot **proot);

#endif
