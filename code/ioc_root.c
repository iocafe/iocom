/**

  @file    ioc_root.c
  @brief   Communication root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.7.2018

  The communication root object...

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/**
****************************************************************************************************

  @brief Initialize root object.
  @anchor ioc_initialize_root

  The ioc_root initialize() function initializes the root object. The root object can always
  be allocated as global variable or by other means by application. It must exist until
  ioc_release_root() is called.

  @param   root Pointer to root structure to be initialized.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_root(
    iocRoot *root)
{
    os_memclear(root, sizeof(iocRoot));

#if OSAL_MULTITHREAD_SUPPORT
    root->mutex = osal_mutex_create();
#endif

    /* Start automatic device enumeration from 10001 and start unique memory block
       identifiers from 8
     */
    root->auto_device_nr = IOC_AUTO_DEVICE_NR + 1;
    root->next_unique_mblk_id = IOC_MIN_UNIQUE_ID;

    /* Mark root structure as initialized (for debugging).
     */
    IOC_SET_DEBUG_ID(root, 'R')

#if OSAL_CHECKSUM_TEST
    /* Test the checksum code.
     */
    osal_test_checksum();
#endif
}


/**
****************************************************************************************************

  @brief Release communication root object.
  @anchor ioc_release_root

  The ioc_release_root() function releases resources allocated for the root object.

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_release_root(
    iocRoot *root)
{
#if OSAL_MULTITHREAD_SUPPORT
    iocConnection
        *con;

    osalStatus
        status;

#if OSAL_SOCKET_SUPPORT
    iocEndPoint
        *epoint;
#endif
#endif

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    /* Synchronize, no more callbacks.
     */
    ioc_lock(root);
    root->callback_func = OS_NULL;


#if OSAL_MULTITHREAD_SUPPORT
#if OSAL_SOCKET_SUPPORT
    /* Terminate all end point worker threads.
     */
    while (OS_TRUE)
    {
        status = OSAL_SUCCESS;
        for (epoint = root->epoint.first;
             epoint;
             epoint = epoint->link.next)
        {
            status |= ioc_terminate_end_point_thread(epoint);
        }
        if (status == OSAL_SUCCESS) break;

        ioc_unlock(root);
        os_timeslice();
        ioc_lock(root);
    }
#endif

    /* Terminate all connection worker threads.
     */
    while (OS_TRUE)
    {
        status = OSAL_SUCCESS;
        for (con = root->con.first;
             con;
             con = con->link.next)
        {
            status |= ioc_terminate_connection_thread(con);
        }
        if (status == OSAL_SUCCESS) break;

        ioc_unlock(root);
        os_timeslice();
        ioc_lock(root);
    }
#endif

#if IOC_DYNAMIC_MBLK_CODE
    /* If we have dynamic IO network configuration, release it.
     */
    ioc_release_dynamic_root(root->droot);
    root->droot = OS_NULL;

    ioc_release_event_queue(root);
#endif

    /* Release all initialized end points.
     */
#if OSAL_SOCKET_SUPPORT
    while (root->epoint.first)
    {
        ioc_release_end_point(root->epoint.first);
    }
#endif

    /* Release all initialized connections.
     */
    while (root->con.first)
    {
        ioc_release_connection(root->con.first);
    }

    /* Release all initialized memory blocks.
     */
    while (root->mblk.first)
    {
        ioc_release_memory_block(&root->mblk.first->handle);
    }

    /* End syncronization.
     */
    ioc_unlock(root);

#if OSAL_MULTITHREAD_SUPPORT
    /* Delete synchronization mutex.
     */
    osal_mutex_delete(root->mutex);
#endif

    /* Mark that the root structure is no longer initialized (for debugging).
     */
    IOC_SET_DEBUG_ID(root, 0)
}


/**
****************************************************************************************************

  @brief Set network name for the IOCOM root.
  @anchor ioc_set_network_name

  The ioc_set_network_name() is called for an IO board, etc, to set up network name for all
  communication.

  @param   root Pointer to root structure.
  @param   network_name Network name to set. OS_NULL to clear network name.
  @return  None.

****************************************************************************************************
*/
void ioc_set_network_name(
    iocRoot *root,
    const os_char *network_name)
{
    os_strncpy(root->network_name, network_name, IOC_NETWORK_NAME_SZ);
}


/**
****************************************************************************************************

  @brief Run the communication.
  @anchor ioc_run

  The ioc_run() function is what actually makes communication to do anything. It needs to be
  called repeatedly.

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_run(
    iocRoot *root)
{
    iocEndPoint
        *epoint;

    iocConnection
        *con,
        *next_con;

    osalStatus
        status;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    /* Synchronize.
     */
    ioc_lock(root);

#if OSAL_SOCKET_SUPPORT
    /* Run the end points.
     */
    for (epoint = root->epoint.first;
         epoint;
         epoint = epoint->link.next)
    {
#if OSAL_MULTITHREAD_SUPPORT
        if (!epoint->worker_thread_running &&
            !epoint->stop_worker_thread)
        {
            ioc_run_endpoint(epoint);
        }
#else
        ioc_run_endpoint(epoint);
#endif
    }
#endif

    /* Run the connections.
     */
    for (con = root->con.first;
         con;
         con = next_con)
    {
        next_con = con->link.next;

#if OSAL_MULTITHREAD_SUPPORT
        status = OSAL_SUCCESS;
        if (!con->worker.thread_running &&
            !con->worker.stop_thread)
        {
            status = ioc_run_connection(con);
        }
#else
        status = ioc_run_connection(con);
#endif

        if (status && con->flags & IOC_CLOSE_CONNECTION_ON_ERROR)
        {
            ioc_release_connection(con);
        }
    }

    /* End syncronization.
     */
    ioc_unlock(root);
}


#if OSAL_MULTITHREAD_SUPPORT
/**
****************************************************************************************************

  @brief Lock the communication object hierarchy.
  @anchor ioc_lock

  Lock functions are are used to lock object hierarchy for the root so it can be accessed only
  by one thread at the time. This is necessary for thread safety. Once the ioc_lock() is
  called by one threads, other threads are paused when they ioc_lock(), until the first
  thread calls ioc_unlock().

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_lock(
    iocRoot *root)
{
    osal_debug_assert(root->debug_id == 'R');
    osal_mutex_lock(root->mutex);
}
#endif

#if OSAL_MULTITHREAD_SUPPORT
/**
****************************************************************************************************

  @brief Unlock the communication object hierarchy.
  @anchor ioc_unlock

  See ioc_lock for information.

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_unlock(
    iocRoot *root)
{
    osal_debug_assert(root->debug_id == 'R');
    osal_mutex_unlock(root->mutex);
}
#endif


/**
****************************************************************************************************

  @brief Set callback function for iocRoot object.
  @anchor ioc_set_connection_callback

  The ioc_set_connection_callback function sets callback function for the iocRoot object. This
  allows application to get information about global events, like new dynamically allocated
  memory blocks.

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_set_root_callback(
    iocRoot *root,
    ioc_root_callback func,
    void *context)
{
    root->callback_func = func;
    root->callback_context = context;
}


/**
****************************************************************************************************

  @brief Inform application about a communication event.
  @anchor ioc_new_root_event

  The TOCOM library can inform application about new/deleted memory blocks, connected IO networks
  and devices. This is much more efficient than polling for changes, especially in large IO
  device networks (IoT applications). 

  This function generates the event, using method selected for the application:

  Application can be informed either by callback qunction, or queueing the event information
  and setting operating system event to trigger the application. These methods are alternatives:
  Callbacks are generally better suited for signle thread model, while queues are typically 
  better choice in complex multithread environments. Event queues are exclusively used with 
  Python API.

  ioc_lock must be on when calling this function.

  @param   root Pointer to the root structure.
  @param   event Which event: IOC_NEW_MEMORY_BLOCK, IOC_NEW_NETWORK...
  @param   dnetwork Dynamic network structure, OS_NULL if not available.
  @param   mblk Memory block structure, OS_NULL if not available.
  @param   context Application callback context.
  @return  None.

****************************************************************************************************
*/
void ioc_new_root_event(
    iocRoot *root,
    iocEvent event,
    struct iocDynamicNetwork *dnetwork,
    struct iocMemoryBlock *mblk,    
    void *context)
{
#if IOC_DYNAMIC_MBLK_CODE
    const os_char *network_name;
    const os_char *device_name;
    const os_char *mblk_name;
    os_uint device_nr;
#endif

    if (root->callback_func)
    {
        root->callback_func(root, event, dnetwork, mblk, root->callback_context);
    }

#if IOC_DYNAMIC_MBLK_CODE
    if (root->event_queue)
    {
        network_name = OS_NULL;
        if (dnetwork)
        {
            network_name = dnetwork->network_name;
        }
        if (mblk)
        {
            network_name = mblk->network_name;
            device_name = mblk->device_name;
            device_nr = mblk->device_nr;
            mblk_name = mblk->mblk_name;
        }
        else
        {
            device_name = OS_NULL;
            device_nr = 0;
            mblk_name = OS_NULL;
        }

        ioc_queue_event(root, event, network_name,
            device_name, device_nr, mblk_name);
    }
#endif
}


/**
****************************************************************************************************

  @brief Create unique identifier for device.
  @anchor ioc_get_unique_mblk_id

  Some devices, like UI clients, games, etc, may not have device number associate with them
  and return IOC_AUTO_DEVICE_NR as device number to controller. Controller uses this
  function to assign unique device ID for the device.

  ioc_lock() must be on before calling this function.

  @param   root Pointer to the root object.
  @return  Unique device identifier IOC_AUTO_DEVICE_NR + 1 .. 0xFFFFFFFF.

****************************************************************************************************
*/
os_uint ioc_get_unique_device_id(
    iocRoot *root)
{
    iocConnection *con;
    os_uint id;
    os_int count;

    /* Just return next number
     */
    if (root->auto_device_nr)
    {
        return root->auto_device_nr++;
    }

    /* We run out of numbers. Strange, this can be possible only if special effort is
       made for this to happen. Handle anyhow.
     */
    count = 100000;
    while (count--)
    {
        id = osal_rand(IOC_AUTO_DEVICE_NR + 1, 0xFFFFFFFFL);
        for (con = root->con.first;
             con;
             con = con->link.next)
        {
            if (id == con->auto_device_nr) break;
        }
        if (con == OS_NULL) return id;
    }

    osal_debug_error("Out of numbers (devices)");
    return 1;
}
