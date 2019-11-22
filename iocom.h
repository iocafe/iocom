/**

  @file    iocom.h
  @brief   Main iocom header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    29.7.2018

  iocom library main header file. If further includes rest of base iocom headers. 

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOCOM_INCLUDED
#define IOCOM_INCLUDED

/* Include operating system abstraction layer with extension headers.
 */
#include "eosalx.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Enable/disable dynamic allocation of memory blocks and resizing
   the memory blocks, if dynamic memory allocation is supported.
   There may be cases when there is reason to do otherwise, so this
   can be overridden by compiler define.
 */
#ifndef IOC_DYNAMIC_MBLK_CODE
#define IOC_DYNAMIC_MBLK_CODE OSAL_DYNAMIC_MEMORY_ALLOCATION
#endif

#ifndef IOC_RESIZE_MBLK_CODE
#define IOC_RESIZE_MBLK_CODE OSAL_DYNAMIC_MEMORY_ALLOCATION
#endif


/* Include all base iocom headers.
 */
#include "code/ioc_timing.h"
#include "code/ioc_debug.h"
#include "code/ioc_handle.h"
#include "code/ioc_root.h"
#include "code/ioc_com_status.h"
#include "code/ioc_memory_block_info.h"
#include "code/ioc_memory_block.h"
#include "code/ioc_signal.h"
#include "code/ioc_connection.h"
#if OSAL_SOCKET_SUPPORT
#include "code/ioc_end_point.h"
#endif
#include "code/ioc_source_buffer.h"
#include "code/ioc_target_buffer.h"
#include "code/ioc_compress.h"
#include "code/ioc_memory.h"
#include "code/ioc_identifiers.h"
#include "code/ioc_dyn_signal.h"
#include "code/ioc_dyn_network.h"
#include "code/ioc_dyn_root.h"
#include "code/ioc_dyn_mblk_list.h"
#include "code/ioc_ioboard.h"
#include "code/ioc_poolsize.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
