/**

  @file    ioc_memory.c
  @brief   Memory allocation.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.7.2018

  A static memory buffer can be as memory pool for the ioal library. The ioc_set_memory_pool() 
  function stores buffer pointer within the iocRoot structure.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


typedef struct iocFreeBlk
{
    struct iocFreeBlk *next_samesz;
    struct iocFreeBlk *next_diffsz;
    os_int sz;
}
iocFreeBlk;


/**
****************************************************************************************************

  @brief Setup buffer to use as memory pool.
  @anchor ioc_set_memory_pool

  The ioc_set_memory_pool() function stores memory pool information into iocRoot sturture.
  Once pool is set, all memory is allocated from it and dynamic heap based memory allocation
  is not used.
  The ioc_set_memory_pool must be called soon after ioc_initialize(), before any memory allocation
  is done. Once set, pool cannot be modified.


  @param   root Pointer to iocom root structure.
  @param   buf Pointer to static buffer to use as pool.
  @param   bufsz BUffer size in bytes.
  @return  None.

****************************************************************************************************
*/
void ioc_set_memory_pool(
    iocRoot *root,
    os_uchar *buf,
    os_int bufsz)
{
    root->pool = buf;
    root->poolsz = bufsz;
    root->poolpos = 0;
    root->poolfree  = OS_NULL;
}


/**
****************************************************************************************************

  @brief Allocate a block of memory.
  @anchor ioc_malloc

  The ioc_malloc() allocated memory from either eosal library (OS abstraction layer) or from
  static memory pool (set by ioc_set_memory_pool() function).

  Mutex must be locked when calling this function.

  @param   root Pointer to iocom root structure.
  @param   request_bytes The function allocates at least the amount of memory requested by this
           argument.
  @param   allocated_bytes Pointer to long integer into which to store the actual size of the
           allocated block (in bytes). The actual size is greater or equal to requested size.
           If actual size is not needed, this parameter can be set to OS_NULL.

  @return  Pointer to the allocated memory block.

****************************************************************************************************
*/
os_uchar *ioc_malloc(
    iocRoot *root,
    os_memsz request_bytes,
    os_memsz *allocated_bytes)
{
    iocFreeBlk *b, *prevb, *r;

    /* We cannot allocate smaller memory blocks than free block size.
     */
    osal_debug_assert(request_bytes >= sizeof(iocFreeBlk));

    /* If no static pool, use default memory allocation function.
     */
    if (root->pool == OS_NULL)
    {
        return (os_uchar*)osal_memory_allocate(request_bytes, allocated_bytes);
    }

    prevb = OS_NULL;
    for (b = root->poolfree; b; b = b->next_diffsz)
    {
        if (b->sz == request_bytes)
        {
            if (b->next_samesz)
            {
                r = b->next_samesz;
                b->next_samesz = r->next_samesz;
            }
            else
            {
                r = b;
                if (prevb) prevb->next_diffsz = b->next_diffsz;
                else root->poolfree = b->next_diffsz;
            }
            goto alldone;
        }
        prevb = b;
    }

    if (root->poolsz - root->poolpos >= request_bytes)
    {
        r = (iocFreeBlk*)(root->pool + root->poolpos);
        root->poolpos += (os_int)request_bytes;
        goto alldone;
    } 

    osal_debug_error("iocom out of memory pool");   
    if (allocated_bytes) *allocated_bytes = 0;
    return OS_NULL;

alldone:
    if (allocated_bytes) *allocated_bytes = request_bytes;
    return (os_uchar*)r;
}


/**
****************************************************************************************************

  @brief Release a block of memory.
  @anchor ioc_free

  The ioc_free() function releases a block of memory allocated from operating system
  by ioc_malloc() function.

  Mutex must be locked when calling this function.

  @param   root Pointer to iocom root structure.
  @param   memory_block Pointer to memory block to release. If this pointer is OS_NULL, then
           the function does nothing.
  @param   bytes Size of memory block, either request_bytes given as argument or allocated_bytes
           returned by osal_memory_allocate() function.

  @return  None.

****************************************************************************************************
*/
void ioc_free(
    iocRoot *root,
    void *memory_block,
    os_memsz bytes)
{
    iocFreeBlk *b, *r;

    /* If no static pool, use default memory allocation.
     */
    if (root->pool == OS_NULL) 
    {
        osal_memory_free(memory_block, bytes);
        return;
    }

    /* Make sure that free block information can fit. This limits minimum allocation size.
     */
    osal_debug_assert(bytes >= sizeof(iocFreeBlk));

    /* Join the block to structure of free blocks. Try first block of same size.
     */
    r = (iocFreeBlk *)memory_block;
    r->sz = (os_int)bytes;
    r->next_diffsz = OS_NULL;
    r->next_samesz = OS_NULL;
    for (b = root->poolfree; b; b = b->next_diffsz)
    {
        if (b->sz == bytes) 
        {
            r->next_samesz = b->next_samesz;
            b->next_samesz = r;
            return;
        }
    }

    /* No free block of same size found, add block with new size.
     */
    r->next_diffsz = root->poolfree;
    root->poolfree = r;
}
