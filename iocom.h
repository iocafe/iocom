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
#pragma once
#ifndef IOCOM_INCLUDED
#define IOCOM_INCLUDED

/* Include operating system abstraction layer with extension headers.
 */
#include "eosalx.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* IOCOM library version number.
 */
#ifndef IOCOM_VERSION
#define IOCOM_VERSION "200712"
#endif

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

/* Do we want to enable JPEG compression for brick data transfers.
 */
#ifndef IOC_USE_JPEG_COMPRESSION
  #if OSAL_MICROCONTROLLER
    #define IOC_USE_JPEG_COMPRESSION 0
  #else
    #define IOC_USE_JPEG_COMPRESSION 1
  #endif
#endif

/* If we need streamer support?
 */
#ifndef IOC_DEVICE_STREAMER
#define IOC_DEVICE_STREAMER OSAL_PERSISTENT_SUPPORT
#endif
#ifndef IOC_CONTROLLER_STREAMER
#define IOC_CONTROLLER_STREAMER 1
#endif

#define IOC_STREAMER_SUPPORT (IOC_DEVICE_STREAMER || IOC_CONTROLLER_STREAMER)

/* Do we need support ring buffer transfer for video, etc.
 */
#ifndef IOC_BRICK_RING_BUFFER_SUPPORT
  #if OSAL_MICROCONTROLLER == 0 && IOC_STREAMER_SUPPORT
    #define IOC_BRICK_RING_BUFFER_SUPPORT 1
#else
    #define IOC_BRICK_RING_BUFFER_SUPPORT 0
  #endif
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

/* Enable/disable support for IO device parameters.
 */
#ifndef IOC_DEVICE_PARAMETER_SUPPORT
#define IOC_DEVICE_PARAMETER_SUPPORT OSAL_PERSISTENT_SUPPORT
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

/* Support server <-> cloud server connections. This enables passing user account
   data "backwards" to cloud and strips off unwanted configuration, etc. memory
   blocks from transfers between local and cloud servers.
 */
#ifndef IOC_SERVER2CLOUD_CODE
  #if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
    #define IOC_SERVER2CLOUD_CODE 1
  #else
    #define IOC_SERVER2CLOUD_CODE 0
  #endif
#endif

/* Security and testing is difficult with security on, define to turn much of it off.
   By default iocom's define follows EOSAL_RELAX_SECURITY in eosal.h.
 */
#ifndef IOC_RELAX_SECURITY
#define IOC_RELAX_SECURITY EOSAL_RELAX_SECURITY
#endif


/* Include all base iocom headers and some extension headers needed early.
 */
#include "code/ioc_timing.h"
#include "code/ioc_debug.h"
#include "code/ioc_handle.h"
#include "code/ioc_memory_block_info.h"
#include "code/ioc_authentication.h"
#include "code/ioc_root.h"
#include "code/ioc_memory_block.h"
#include "code/ioc_signal.h"
#include "code/ioc_signal_addr.h"
#if IOC_DYNAMIC_MBLK_CODE
  #include "extensions/dynamicio/ioc_remove_mblk_list.h"
#endif
#include "code/ioc_connection.h"
#include "code/ioc_end_point.h"
#include "code/ioc_source_buffer.h"
#include "code/ioc_target_buffer.h"
#include "code/ioc_compress.h"
#include "code/ioc_memory.h"
#include "code/ioc_streamer.h"
#include "code/ioc_brick.h"
#include "code/ioc_parameters.h"
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
