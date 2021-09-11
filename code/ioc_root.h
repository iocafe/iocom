/**

  @file    ioc_root.h
  @brief   Communication root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Root object acts as the root of communication object hierarchy. It holds first and last
  object pointers for memory block list and for connection list.

  If the library is compiled to support multi-threading, the root object holds also the mutex
  to synchronize access to communication object hierarchy.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_ROOT_H_
#define IOC_ROOT_H_
#include "iocom.h"

/* Structures which are defined later in headers.
 */
struct iocRoot;
struct iocMemoryBlock;
struct iocConnection;
struct iocEndPoint;
struct iocSourceBuffer;
struct iocTargetBuffer;


/** Source buffer internal work buffer item type.
 */
typedef os_ushort ioc_sbuf_item;
typedef os_ushort ioc_tbuf_item;

struct iocFreeBlk;
struct iocDeviceHdr;
struct iocDynamicRoot;
struct iocDynamicNetwork;
struct iocEventQueue;

/* Module name used by iocom library to report errors.
 */
const extern os_char iocom_mod[];

/** Start automatically given device numbers from IOC_AUTO_DEVICE_NR + 1. This can be changed by
    compiler define, but communicating devices using automatic device numbers
    must use the same define.
 */
#ifndef IOC_AUTO_DEVICE_NR
#define IOC_AUTO_DEVICE_NR 9000
#endif
#define IOC_TO_AUTO_DEVICE_NR (IOC_AUTO_DEVICE_NR-1)

/** Flag for ioc_initialize_root() to use EOSAL system mutex for synchronization, instead
    of creating own one.
 */
#define IOC_USE_EOSAL_MUTEX 1
#define IOC_CREATE_OWN_MUTEX 0


/**
****************************************************************************************************
    Linked list of root's memory blocks
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the first memory block in linked list.
     */
    struct iocMemoryBlock *first;

    /** Pointer to the last memory block in linked list.
     */
    struct iocMemoryBlock *last;
}
iocRootsMemoryBlockList;


/**
****************************************************************************************************
    Linked list of root's connections
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the first connection in linked list.
     */
    struct iocConnection *first;

    /** Pointer to the last connection in linked list.
     */
    struct iocConnection *last;
}
iocRootsConnectionList;


/**
****************************************************************************************************
    Linked list of root's end points
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the first end point in linked list.
     */
    struct iocEndPoint *first;

    /** Pointer to the last end point in linked list.
     */
    struct iocEndPoint *last;
}
iocRootsEndPointList;


/**
****************************************************************************************************
    Root callback event enumeration, reason why the callback?
****************************************************************************************************
*/
typedef enum
{
    IOC_NEW_MEMORY_BLOCK,
    IOC_MBLK_CONNECTED_AS_SOURCE,
    IOC_MBLK_CONNECTED_AS_TARGET,
    IOC_MEMORY_BLOCK_DELETED,

    IOC_NEW_NETWORK,
    IOC_NETWORK_DISCONNECTED,

    IOC_NEW_DEVICE,
    IOC_DEVICE_DISCONNECTED
}
iocEvent;


/**
****************************************************************************************************
    Root callback function type (network and device connect/disconnect, etc).
****************************************************************************************************
*/
typedef void ioc_root_callback(
    struct iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,
    void *context);


/**
****************************************************************************************************

  @name Root object structure.

  The iocRoot is the root of communication object hierarchy. It maintains lists of memory
  buffers and connections, and if multithreading is supported, mutex to synchronize access
  to the commmunication object hierarcy.

****************************************************************************************************
*/
typedef struct iocRoot
{
    /** Debug identifier must be first item in the object structure. It is used to verify
        that a function argument is pointer to correct initialized object.
     */
    IOC_DEBUG_ID

    /** Linked list of root's memory blocks.
     */
    iocRootsMemoryBlockList mblk;

    /** Linked list of root's connections.
     */
    iocRootsConnectionList con;

#if OSAL_SOCKET_SUPPORT
    /** Linked list of root's end points.
     */
    iocRootsEndPointList epoint;
#endif

    /** IO device only: Device name, if this is single IO device. Empty if not set.
     */
    os_char device_name[IOC_NAME_SZ];

    /** IO device only: Device number, if this is single IO device. Zero if not set.
     */
    os_int device_nr;

    /** IO device only: Network name. Empty if not set.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

#if IOC_AUTHENTICATION_CODE
    /** Default password for the device.
     */
    os_char password[IOC_PASSWORD_SZ];
#endif

    /** Pointer to static memory pool, OS_NULL is not used.
        Pool size is in bytes.
     */
    os_char *pool;
    os_int poolsz;
    os_int poolpos;
    struct iocFreeBlk *poolfree;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_boolean pool_alllocated;
#endif

#if OSAL_MULTITHREAD_SUPPORT
    /** Flags given to ioc_initialize_root, bit fields. Flag IOC_USE_EOSAL_MUTEX (1)
        indicates that iocom used EOSAL system mutex for thread synchronization.
        IOC_CREATE_OWN_MUTEX is defines as 0 and can be used to mark that own
        mutex is created.
     */
    os_char init_flags;

    /** Mutex to synchronize access to communication object hierarchy.
     */
    osalMutex mutex;
#endif

#if IOC_ROOT_CALLBACK_SUPPORT
    /** Callback function pointer. OS_NULL if not used.
     */
    ioc_root_callback *callback_func;

    /** Callback context for callback function. OS_NULL if not used.
     */
    void *callback_context;
#endif

    /** Automatic device number, used if device number is 0
     */
    os_uint auto_device_nr;

    /** Next unique memory block identifier to reserve.
     */
    os_uint next_unique_mblk_id;

#if IOC_DYNAMIC_MBLK_CODE
    /** Pointer to dynamic IO network configuration, if any.
     */
    struct iocDynamicRoot *droot;

    /** Pointer to communication event queue. The application processess
        these events to know about connected/disconnected device IO
        networks, devices, etc.
     */
    struct iocEventQueue *event_queue;
#endif

#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
    ioc_authorize_user_func *authorization_func;
    void *authorization_context;
#endif

#if IOC_DYNAMIC_MBLK_CODE
    /** Flag for basic server (iocBServer). Check for missing certificate chain and
        flash program versions. This is optimization flag for automatic uploader.
     */
    os_boolean check_cert_chain_etc;
#endif
}
iocRoot;


/**
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_root() function initializes or allocates new communication root objects,
  and ioc_release_root() releases resources associated with it. Memory allocated for the root
  object is freed, if it was allocated by ioc_initialize_root().

****************************************************************************************************
 */
/*@{*/

/* Initialize communication root object.
 */
void ioc_initialize_root(
    iocRoot *root,
    os_char flags);

/* Release communication root object.
 */
#if OSAL_PROCESS_CLEANUP_SUPPORT
    void ioc_release_root(
        iocRoot *root);
#else
    #define ioc_release_root(r)
#endif

/* Set network name for the IOCOM root.
 */
void ioc_set_iodevice_id(
    iocRoot *root,
    const os_char *device_name,
    os_int device_nr,
    const os_char *password,
    const os_char *network_name);

/* Run the communication.
 */
void ioc_run(
    iocRoot *root);

/* Run the communication in single tread build only.
   Define evaluates to empty in multithread mode.
 */
#if OSAL_MULTITHREAD_SUPPORT
    #define ioc_single_thread_run(r)
#else
    #define ioc_single_thread_run(r) ioc_run(r)
#endif

/* Set callback function for iocRoot object.
 */
#if IOC_ROOT_CALLBACK_SUPPORT
    void ioc_set_root_callback(
        iocRoot *root,
        ioc_root_callback func,
        void *context);
#endif

/* Inform application about a communication event.
 */
#if IOC_ROOT_CALLBACK_SUPPORT
    void ioc_new_root_event(
        iocRoot *root,
        iocEvent event,
        struct iocDynamicNetwork *dnetwork,
        struct iocMemoryBlock *mblk,
        void *context);
#else
    #define ioc_new_root_event(r,e,d,m,c)
#endif


/* Create unique identifier for device.
 */
os_uint ioc_get_unique_device_id(
    iocRoot *root,
    os_uchar *unique_id_bin);

/* Copy root's network name to memory blocks without name.
 */
void ioc_set_network_name(
    iocRoot *root);

/* Send data from all memory blocks synchronously.
 */
void ioc_send_all(
    iocRoot *root);

/* Receive data synchronously for all memory blocks.
 */
void ioc_receive_all(
    iocRoot *root);

/*@}*/

#if OSAL_MULTITHREAD_SUPPORT
/** Lock functions. These are used to lock object hierarchy under the root so it can be accessed
    only by one thread at the time. This is necessary for thread safety.
*/
/*@{*/

/* Lock the communication object hierarchy.
 */
void ioc_lock(
    iocRoot *root);

/* Unlock the communication object hierarchy.
 */
void ioc_unlock(
    iocRoot *root);

/* Declare iocRoot pointer for thread syncronization only
 */
#define IOC_MT_ROOT_PTR iocRoot *root

/* Macro to set root pointer when multithreading is supported.
 */
#define ioc_set_mt_root(r,p) r=p

/*@}*/
#else

/** Empty event macros. If OSAL_MULTITHREAD_SUPPORT flag is zero, these macros will replace
    functions and do not generate any code. This allows to compile code which has calls
    to lock functions without multithreading support.
*/
/*@{*/
  #define ioc_lock(x)
  #define ioc_unlock(x)
  #define IOC_MT_ROOT_PTR
  #define ioc_set_mt_root(r,p)
/*@}*/

#endif
#endif
