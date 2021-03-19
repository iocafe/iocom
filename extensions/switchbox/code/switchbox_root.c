/**

  @file    switchbox_root.h
  @brief   Switchbox root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  The communication root object...

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "switchbox.h"


/**
****************************************************************************************************

  @brief Initialize root object.
  @anchor ioc_initialize_switchbox_root

  The ioc_root initialize() function initializes the root object. The root object can always
  be allocated as global variable or by other means by application. It must exist until
  ioc_release_switchbox_root() is called.

  @param   root  Pointer to root structure to be initialized.
  @param   flags Zero for default operation. IOC_USE_EOSAL_MUTEX specifies to use
           eosal mutex for synchronization. In more complex interaction, this can be used
           to avoid deadlock, with small performance penalty.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_switchbox_root(
    switchboxRoot *root,
    os_char flags)
{
    os_memclear(root, sizeof(switchboxRoot));

    root->mutex = osal_mutex_create();
}


/**
****************************************************************************************************

  @brief Release communication root object.
  @anchor ioc_release_switchbox_root

  The ioc_release_switchbox_root() function releases resources allocated for the root object.

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_release_switchbox_root(
    switchboxRoot *root)
{
    switchboxServiceConnection
        *con;

    osalStatus
        status;

    switchboxEndPoint
        *epoint;

    /* Synchronize, no more callbacks.
     */
    ioc_switchbox_lock(root);

    /* Terminate all end point worker threads.
     */
    while (OS_TRUE)
    {
        status = OSAL_SUCCESS;
        for (epoint = root->epoint.first;
             epoint;
             epoint = epoint->link.next)
        {
            status |= ioc_terminate_switchbox_end_point_thread(epoint);
        }
        if (status == OSAL_SUCCESS) break;

        ioc_switchbox_unlock(root);
        os_timeslice();
        ioc_switchbox_lock(root);
    }

    /* Terminate all connection worker threads.
     */
    while (OS_TRUE)
    {
        status = OSAL_SUCCESS;
        for (con = root->con.first;
             con;
             con = con->link.next)
        {
            status |= ioc_terminate_switchbox_service_connection_thread(con);
        }
        if (status == OSAL_SUCCESS) break;

        ioc_switchbox_unlock(root);
        os_timeslice();
        ioc_switchbox_lock(root);
    }

    /* Release all initialized end points.
     */
    while (root->epoint.first)
    {
        ioc_release_switchbox_end_point(root->epoint.first);
    }

    /* Release all initialized connections.
     */
    while (root->con.first)
    {
        ioc_release_switchbox_service_connection(root->con.first);
    }

    /* End syncronization.
     */
    ioc_switchbox_unlock(root);

    /* Delete synchronization mutex.
     */
    osal_mutex_delete(root->mutex);
}


/**
****************************************************************************************************

  @brief Lock the switchbox object hierarchy.
  @anchor ioc_switchbox_lock

  Lock functions are are used to lock object hierarchy for the root so it can be accessed only
  by one thread at the time. This is necessary for thread safety. Once the ioc_switchbox_lock() is
  called by one threads, other threads are paused when they ioc_switchbox_lock(), until the first
  thread calls ioc_switchbox_unlock().

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_switchbox_lock(
    switchboxRoot *root)
{
    osal_mutex_lock(root->mutex);
}


/**
****************************************************************************************************

  @brief Unlock the switchbox object hierarchy.
  @anchor ioc_switchbox_unlock

  See ioc_switchbox_lock for information.

  @param   root Pointer to the root structure.
  @return  None.

****************************************************************************************************
*/
void ioc_switchbox_unlock(
    switchboxRoot *root)
{
    osal_mutex_unlock(root->mutex);
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
#if 0
void ioc_run(
    switchboxRoot *root)
{
    switchboxServiceConnection
        *con,
        *next_con;

    osalStatus
        status;

    /* Synchronize.
     */
    ioc_switchbox_lock(root);

    /* Run the end points.
     */
    switchboxEndPoint *epoint;
    for (epoint = root->epoint.first;
         epoint;
         epoint = epoint->link.next)
    {
        if (!epoint->worker_thread_running &&
            !epoint->stop_worker_thread)
        {
            ioc_switchbox_run_endpoint(epoint);
        }
    }

    /* Run the connections.
     */
    for (con = root->con.first;
         con;
         con = next_con)
    {
        next_con = con->link.next;

        status = OSAL_SUCCESS;
        if (!con->worker.thread_running &&
            !con->worker.stop_thread)
        {
            status = ioc_run_switchbox_connection(con);
        }

        if (status && con->flags & IOC_CLOSE_CONNECTION_ON_ERROR)
        {
            ioc_release_switchbox_service_connection(con);
        }
    }

    /* End syncronization.
     */
    ioc_switchbox_unlock(root);
}
#endif
