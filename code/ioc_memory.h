/**

  @file    ioc_memory.h
  @brief   Memory allocation.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  A static memory buffer can be as memory pool for the ioal library. The ioc_set_memory_pool()
  function stores buffer pointer within the iocRoot structure.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_MEMORY_H_
#define IOC_MEMORY_H_
#include "iocom.h"

/* FLags for ioc_malloc and ioc_free.
 */
#define IOC_DEFAULT_ALLOC 0
#define IOC_PREFER_PSRAM 1

/* Setup buffer to use as memory pool.
*/
void ioc_set_memory_pool(
    iocRoot *root,
    os_char *buf,
    os_memsz bufsz);

/* If pool was allocated by ioc_set_memory_pool(), then release it.
 */
void ioc_release_memory_pool(
    iocRoot *root);

/* Allocate a block of memory.
 */
os_char *ioc_malloc(
    iocRoot *root,
    os_memsz request_bytes,
    os_memsz *allocated_bytes,
    os_int flags);

/* Release a block of memory.
 */
void ioc_free(
    iocRoot *root,
    void *memory_block,
    os_memsz bytes,
    os_int flags);

#endif
