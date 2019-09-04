/**

  @file    ioc_root.h
  @brief   Communication root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.7.2018

  Root object acts as the root of communication object hierarchy. It holds first and last
  object pointers for memory block list and for connection list.

  If the library is compiled to support multi-threading, the root object holds also the mutex
  to synchronize access to communication object hierarchy.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_ROOT_INCLUDED
#define IOC_ROOT_INCLUDED

/* Structures defined later in headers.
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
    IOC_NEW_DYNAMIC_MBLK
    /* IOC_MBLK_CONNECTED */
}
iocRootCallbackEvent;


/**
****************************************************************************************************
    Root callback function type.
****************************************************************************************************
*/
typedef void ioc_root_callback(
    struct iocRoot *root,
    struct iocConnection *con,
    struct iocMemoryBlock *mblk,
    iocRootCallbackEvent event,
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

    /** Pointer to static memory pool, OS_NULL is not used.
        Pool size is in bytes.
     */
    os_uchar *pool;
    os_int poolsz;
    os_int poolpos;
    struct iocFreeBlk *poolfree;

#if OSAL_MULTITHREAD_SUPPORT
    /** Mutex to synchronize access to communication object hierarchy.
     */
    osalMutex mutex;
#endif

    /** How many times stream (socket, etc) has been dropped.
        Global diagnostics counter.
     */
    os_int drop_count;

    /** Callback function pointer. OS_NULL if not used.
     */
    ioc_root_callback *callback_func;

    /** Callback context for callback function. OS_NULL if not used.
     */
    void *callback_context;
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
    iocRoot *root);

/* Release communication root object.
 */
void ioc_release_root(
    iocRoot *root);

/* Run the communication.
 */
void ioc_run(
    iocRoot *root);

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

/* Set callback function for iocRoot object.
 */
void ioc_set_root_callback(
    iocRoot *root,
    ioc_root_callback func,
    void *context);

/*@}*/

#endif
