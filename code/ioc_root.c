/**

  @file    ioc_root.c
  @brief   Communication root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The communication root object...

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

/* Module name used by iocom library to report errors.
 */
const os_char iocom_mod[] = "iocom";


/**
****************************************************************************************************

  @brief Initialize root object.
  @anchor ioc_initialize_root

  The ioc_root initialize() function initializes the root object. The root object can always
  be allocated as global variable or by other means by application. It must exist until
  ioc_release_root() is called.

  @param   root  Pointer to root structure to be initialized.
  @param   flags Zero for default operation. IOC_USE_EOSAL_MUTEX specifies to use
           eosal mutex for synchronization. In more complex interaction, this can be used
           to avoid deadlock, with small performance penalty.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_root(
    iocRoot *root,
    os_char flags)
{
    os_memclear(root, sizeof(iocRoot));

#if OSAL_MULTITHREAD_SUPPORT
    root->init_flags = flags;
    if (flags & IOC_USE_EOSAL_MUTEX) {
        root->mutex = osal_global->system_mutex;
    }
    else {
        root->mutex = osal_mutex_create();
    }
#endif

    /* Start automatic device enumeration from 10001 and start unique memory block
       identifiers from 8.
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


#if OSAL_PROCESS_CLEANUP_SUPPORT
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

#if IOC_ROOT_CALLBACK_SUPPORT
    root->callback_func = OS_NULL;
#endif


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
    if ((root->init_flags & IOC_USE_EOSAL_MUTEX) == 0) {
        osal_mutex_delete(root->mutex);
    }
#endif

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    /* If we allocated pool (fixed size pool, but dynamically allocated,
       the release it now.
     */
    ioc_release_memory_pool(root);
#endif

    /* Mark that the root structure is no longer initialized (for debugging).
     */
    IOC_SET_DEBUG_ID(root, 0)
}
#endif

/**
****************************************************************************************************

  @brief Set network name for the IOCOM root.
  @anchor ioc_set_iodevice_id

  The ioc_set_iodevice_id() is called for an IO board, etc, to store up IO device identification
  for all communication. The information is stored within the root object.

  @param   root Pointer to root structure.
  @param   device_name Device name to set. OS_NULL to clear network name.
  @param   device_nr Device number to set. Zero to clear.
  @param   password password for the device. Set "*" to use generate password automatically.
  @param   network_name Network name to set. OS_NULL to clear network name.
  @return  None.

****************************************************************************************************
*/
void ioc_set_iodevice_id(
    iocRoot *root,
    const os_char *device_name,
    os_int device_nr,
    const os_char *password,
    const os_char *network_name)
{
    os_strncpy(root->device_name, device_name, IOC_NAME_SZ);
    root->device_nr = device_nr;
#if IOC_AUTHENTICATION_CODE
  #if OSAL_SECRET_SUPPORT
    if (os_strcmp(password, osal_str_empty) && os_strcmp(password, osal_str_asterisk))
    {
        os_strncpy(root->password, password, IOC_PASSWORD_SZ);
    }
    else
    {
        osal_get_auto_password(root->password, IOC_PASSWORD_SZ);
    }
  #else
    os_strncpy(root->password, password, IOC_PASSWORD_SZ);
  #endif
#endif
    os_strncpy(root->network_name, network_name, IOC_NETWORK_NAME_SZ);
    osal_set_network_state_str(OSAL_NS_IO_NETWORK_NAME, 0, network_name);
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
    iocEndPoint *epoint;
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


#if IOC_ROOT_CALLBACK_SUPPORT
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
    OSAL_UNUSED(context);

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
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
            network_name = mblk->network_name;
            device_name = mblk->device_name;
            device_nr = mblk->device_nr;
#else
            network_name = root->network_name;
            device_name = root->device_name;
            device_nr = root->device_nr;
#endif
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
#endif


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
    os_int id;
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
        id = (os_int)osal_rand(IOC_AUTO_DEVICE_NR + 1, 0x7FFFFFFFL);
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


/**
****************************************************************************************************

  @brief Copy root's network name to memory blocks without name.
  @anchor ioc_set_network_name

  Copy network name from iocRoot object to memory blocks which have no network name as "*".
  Called when root network name is changed afterwards (for now only by lighthouse).

  This works only for an IO device using static signal. Controllers, etc. using dynamic signals
  cannot change their network name on the fly.

  @param   root Pointer to the root object.
  @return  None.

****************************************************************************************************
*/
void ioc_set_network_name(
    iocRoot *root)
{
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    iocMemoryBlock *mblk;

    ioc_lock(root);

    for (mblk = root->mblk.first; mblk; mblk = mblk->link.next)
    {
        if (!os_strcmp(mblk->network_name, osal_str_asterisk) || mblk->network_name[0] == '\0')
        {
            os_strncpy(mblk->network_name, root->network_name, IOC_NETWORK_NAME_SZ);
        }
    }

    ioc_unlock(root);
#endif

    osal_set_network_state_str(OSAL_NS_IO_NETWORK_NAME, 0, root->network_name);
}


/**
****************************************************************************************************

  @brief Send data from all memory blocks synchronously.
  @anchor ioc_send_all

  The ioc_send_all() function pushes all writes to all memory blocks. This or ioc_send() function
  must be called from application.

  Call ioc_send_all() function repeatedly, for example in mictorontroller's main loop.
  Synchronous  sending causes all changes done in same main loop round to be transmitted together.

  It is possible to reduce data transmitted from noicy analog inputs by calling ioc_send()
  at low frequency. This assumes that analog inputs with same desired maximum update frequency
  are grouped into same memory block.

  @param   root Pointer to IOCOM root object.
  @return  None.

****************************************************************************************************
*/
void ioc_send_all(
    iocRoot *root)
{
    iocMemoryBlock *mblk;
    iocSourceBuffer *sbuf;
    if (root == OS_NULL) return;

    ioc_lock(root);
    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        for (sbuf = mblk->sbuf.first;
             sbuf;
             sbuf = sbuf->mlink.next)
        {
            ioc_sbuf_synchronize(sbuf);
        }
    }
    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Receive data synchronously for all memory blocks.
  @anchor ioc_receive_all

  The ioc_receive_all() function moves received data as snapshot to be available for reads for all
  memory blocks. This or ioc_receive() function must be called from application.

  @param   root Pointer to IOCOM root object.
  @return  None.

****************************************************************************************************
*/
void ioc_receive_all(
    iocRoot *root)
{
    iocMemoryBlock *mblk;

    ioc_lock(root);
    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        ioc_receive_nolock(mblk);
    }
    ioc_unlock(root);
}
