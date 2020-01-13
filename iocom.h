/**

  @file    iocom.h
  @brief   Main iocom header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Main iocom library base header file. If further includes rest of base iocom headers.
  If dynamic extensions for server side or Python API are needed, include "iocomx.h" header
  instead of this file.

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
   There are cases when there is reason to do otherwise, so this
   can be overridden by compiler define.
 */
#ifndef IOC_DYNAMIC_MBLK_CODE
  #if OSAL_MICROCONTROLLER
    #define IOC_DYNAMIC_MBLK_CODE 0
  #else
    #define IOC_DYNAMIC_MBLK_CODE OSAL_DYNAMIC_MEMORY_ALLOCATION
  #endif
#endif

/* If we are using dynamic memory allocation, include code to
 * resize memory blocks.
 */
#ifndef IOC_RESIZE_MBLK_CODE
#define IOC_RESIZE_MBLK_CODE OSAL_DYNAMIC_MEMORY_ALLOCATION
#endif

/* Authentication support level defines.
 */
#define IOC_NO_AUTHENTICATION 0
#define IOC_DEVICE_AUTHENTICATION 1
#define IOC_FULL_AUTHENTICATION 2

/* Default authentication support for the platform.
 */
#ifndef IOC_AUTHENTICATION_CODE
  #if OSAL_MICROCONTROLLER
    #define IOC_AUTHENTICATION_CODE IOC_DEVICE_AUTHENTICATION
  #else
    #define IOC_AUTHENTICATION_CODE IOC_FULL_AUTHENTICATION
  #endif
#endif

/* Support for bidirectional memory blocks.
 */
#ifndef IOC_BIDIRECTIONAL_MBLK_CODE
  #if OSAL_MICROCONTROLLER
    #define IOC_BIDIRECTIONAL_MBLK_CODE 0
  #else
    #define IOC_BIDIRECTIONAL_MBLK_CODE 1
  #endif
#endif

/* Include all base iocom headers.
 */
#include "code/ioc_timing.h"
#include "code/ioc_debug.h"
#include "code/ioc_handle.h"
#include "code/ioc_memory_block_info.h"
#include "code/ioc_authentication.h"
#include "code/ioc_root.h"
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
#include "code/ioc_streamer.h"
#include "code/ioc_ioboard.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

/* Be sure to include iocomx.h, if dynamic configuration code is used.
 */
#if IOC_DYNAMIC_MBLK_CODE
#ifndef IOCOMX_INCLUDED
#include "iocomx.h"
#endif
#endif

#endif
