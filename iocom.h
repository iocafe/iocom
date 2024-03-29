/**

  @file    iocom.h
  @brief   Main iocom header file.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

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
#ifndef IOCOM_H_
#define IOCOM_H_

/* Include operating system abstraction layer with extension headers.
 */
#include "eosalx.h"

/* If C++ compilation, all functions, etc. from this point on included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* IOCOM library version number.
 */
#ifndef IOCOM_VERSION
#define IOCOM_VERSION "210424"
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

/* Controller side automatic device numbering support. By default disabled in
   microcontrollers and enabled in bigger computers.
 */
#ifndef IOC_AUTO_DEVICE_NR_SUPPORT
  #define IOC_AUTO_DEVICE_NR_SUPPORT (OSAL_MICROCONTROLLER==0)
#endif

/* Support for switchbox socket to connect up and for switchbox authentication.
 * Switchbox is TLS based, default to OSAL_TLS_SUPPORT.
 */
#ifndef IOC_SWITCHBOX_SUPPORT
#define IOC_SWITCHBOX_SUPPORT OSAL_TLS_SUPPORT
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
#define IOC_DEVICE_PARAMETER_SUPPORT (OSAL_PERSISTENT_SUPPORT && OSAL_MINIMALISTIC == 0)
#endif

/* Support gettings signal number ranges at callback, and dynamic configuration stuff.
   Not used in minimal configuration to save memory.
 */
#ifndef IOC_SIGNAL_RANGE_SUPPORT
#define IOC_SIGNAL_RANGE_SUPPORT (OSAL_MINIMALISTIC == 0)
#endif

/* Support for bidirectional memory blocks.
 */
#ifndef IOC_BIDIRECTIONAL_MBLK_CODE
  #if OSAL_MICROCONTROLLER || OSAL_MINIMALISTIC
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
   By default iocom define IOC_RELAX_SECURITY follows OSAL_RELAX_SECURITY in eosal.h.
 */
#ifndef IOC_RELAX_SECURITY
#define IOC_RELAX_SECURITY OSAL_RELAX_SECURITY
#endif

/* Decide wether to include nick name generator.
 */
#ifndef IOC_NICKGEN_SUPPORT
    #define IOC_NICKGEN_SUPPORT OSAL_NICKNAME_SUPPORT
#endif

/* Support root event callback unless this is minimalistic build
 */
#ifndef IOC_ROOT_CALLBACK_SUPPORT
#define IOC_ROOT_CALLBACK_SUPPORT (OSAL_MINIMALISTIC == 0)
#endif

/* Decide wether to include nick name generator
 */
#ifndef IOC_MBLK_SPECIFIC_DEVICE_NAME
  #define IOC_MBLK_SPECIFIC_DEVICE_NAME (OSAL_MINIMALISTIC == 0)
#endif


#if OSAL_MINIMALISTIC
    typedef os_short ioc_addr;
    typedef os_char ioc_sig_addr;
#else
    typedef os_int ioc_addr;
    typedef os_int ioc_sig_addr;
#endif


/* Include all base iocom headers and some extension headers needed early.
 */
#include "code/ioc_timing.h"
#include "code/ioc_debug.h"
#include "code/ioc_handle.h"
#include "code/ioc_memory_block_info.h"
#include "code/ioc_authentication.h"
#include "code/ioc_auto_device_nr.h"
#include "code/ioc_root.h"
#include "code/ioc_memory_block.h"
#include "code/ioc_signal.h"
#include "code/ioc_signal_addr.h"
#include "code/ioc_streamer.h"
#if IOC_DYNAMIC_MBLK_CODE
  #include "extensions/dynamicio/ioc_remove_mblk_list.h"
#endif
#include "code/ioc_handshake.h"
#include "code/ioc_handshake_iocom.h"
#include "code/ioc_connection.h"
#include "code/ioc_end_point.h"
#include "code/ioc_source_buffer.h"
#include "code/ioc_target_buffer.h"
#include "code/ioc_compress.h"
#include "code/ioc_memory.h"
#include "code/ioc_brick.h"
#include "code/ioc_parameters.h"
#include "code/ioc_ioboard.h"
#if IOC_NICKGEN_SUPPORT
#include "code/ioc_nickgen.h"
#endif
#include "code/ioc_switchbox_auth_frame.h"
#include "code/ioc_switchbox_socket.h"
#include "code/ioc_switchbox_util.h"

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
