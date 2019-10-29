/**

  @file    ioc_memory.h
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

/* Setup buffer to use as memory pool.
*/
void ioc_set_memory_pool(
    iocRoot *root,
    os_char *buf,
    os_int bufsz);

/* Allocate a block of memory.
 */
os_char *ioc_malloc(
    iocRoot *root,
    os_memsz request_bytes,
    os_memsz *allocated_bytes);

/* Release a block of memory.
 */
void ioc_free(
    iocRoot *root,
    void *memory_block,
    os_memsz bytes);
