/**

  @file    ioc_dyn_queue.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Queue network and device connect/disconnects and other events for application. Once
  application calls ioc_initialize_event_queue to start processing queued data, it must
  process queued events periodically by calling ioc_get_event()/ioc_pop_event() functions.

  Queue + event is used (instead of callbacks) to pass information to calling application
  to avoid challenges with application being called by different thread.
  with callbacks.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/**
****************************************************************************************************

  @brief Allocate and initialize event queue.

  The ioc_initialize_event_queue() function is called by application if it wants to process
  communication events. If this function is called, the events must be processed.
  This function sets up queue for the communication events for the IOCOM root structure.

  @param   root Pointer to IOCOM root object.
  @param   event Application event to set when a new event is queued. OS_NULL if not needed.
  @param   max_events Maximum number of events to queue. This shoud be big number which can
           never be exceed, and is there just to avoid running out of memory in case
           application error prevents processing events. Set 0 to use default.
  @param   flags Which communication events we wish to queue, bits: IOC_MBLK_EVENTS,
           IOC_DEVICE_EVENTS, IOC_NETWORK_EVENTS.
  @return  OSAL_SUCCESS to indicate success or OSAL_STATUS_MEMORY_ALLOCATION_FAILED if
           memory allocation has failed.

****************************************************************************************************
*/
osalStatus ioc_initialize_event_queue(
    iocRoot *root,
    osalEvent event,
    os_int max_nro_events,
    os_int flags)
{
    iocEventQueue *queue;

    ioc_lock(root);

    ioc_release_event_queue(root);

    queue = (iocEventQueue*)os_malloc(sizeof(iocEventQueue), OS_NULL);
    if (queue == OS_NULL)
    {
        ioc_unlock(root);
        return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    }
    os_memclear(queue, sizeof(iocEventQueue));
    queue->root = root;
    queue->event = event;
    queue->flags = flags;
    queue->max_nro_events = max_nro_events ? max_nro_events : 1000;
    root->event_queue = queue;

    ioc_unlock(root);

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Release any resources allocated for the event queue.

  The ioc_release_event_queue() function doesn't need to be called by application, it is
  called once IOCOM root stucture is released.

  @param   root Pointer to IOCOM root object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_event_queue(
    iocRoot *root)
{
    iocEventQueue *queue;

    ioc_lock(root);
    queue = root->event_queue;
    if (queue == OS_NULL)
    {
        ioc_unlock(root);
        return;
    }

    while (queue->first)
    {
        ioc_pop_event(root);
    }

    os_free(queue, sizeof(iocEventQueue));
    root->event_queue = OS_NULL;

    ioc_unlock(root);
}


/**
****************************************************************************************************

  @brief Queue a communication event to inform application about it.

  The ioc_queue_event() function creates a new communication event into queue and sets
  application defined OS event (if not NULL) to trigger the application.

  ioc_lock must be on when calling this function.

  @param   root Pointer to IOCOM root object.
  @param   event  Which event: IOC_NEW_MEMORY_BLOCK, IOC_NEW_NETWORK...
  @param   network_name Network name.
  @param   device_name Device name.
  @param   device_nr Device number.
  @param   mblk_name Memory block name.
  @return  OSAL_SUCCESS if successful. Other values indicate an error (out of memory or
           queue overflow).

****************************************************************************************************
*/
osalStatus ioc_queue_event(
    iocRoot *root,
    iocEvent event,
    const os_char *network_name,
    const os_char *device_name,
    os_uint device_nr,
    const os_char *mblk_name)
{
    iocEventQueue *queue;
    iocQueuedEvent *e;

    queue = root->event_queue;

    switch (event)
    {
        case IOC_NEW_MEMORY_BLOCK:
            if ((queue->flags & (IOC_NEW_MBLK_EVENTS|IOC_ALL_MBLK_EVENTS)) == 0) return OSAL_SUCCESS;
            break;

        case IOC_MBLK_CONNECTED_AS_SOURCE:
        case IOC_MBLK_CONNECTED_AS_TARGET:
            if ((queue->flags & (IOC_MBLK_CONNECT_EVENTS|IOC_ALL_MBLK_EVENTS)) == 0) return OSAL_SUCCESS;
            break;

        default:
        case IOC_MEMORY_BLOCK_DELETED:
            if ((queue->flags & IOC_ALL_MBLK_EVENTS) == 0) return OSAL_SUCCESS;
            break;

        case IOC_NEW_NETWORK:
        case IOC_NETWORK_DISCONNECTED:
            if ((queue->flags & IOC_NETWORK_EVENTS) == 0) return OSAL_SUCCESS;
            break;

        case IOC_NEW_DEVICE:
        case IOC_DEVICE_DISCONNECTED:
            if ((queue->flags & IOC_DEVICE_EVENTS) == 0) return OSAL_SUCCESS;
            break;
    }

    if (queue->event_count >= queue->max_nro_events)
    {
        osal_debug_error("Communication event queue overflow.");
        return OSAL_STATUS_FAILED;
    }
    queue->event_count++;

    /* Allocate new event structure and fill it.
     */
    e = (iocQueuedEvent*)os_malloc(sizeof(iocQueuedEvent), OS_NULL);
    if (e == OS_NULL)
    {
        ioc_unlock(root);
        return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    }
    os_memclear(e, sizeof(iocQueuedEvent));
    e->event = event;
    os_strncpy(e->network_name, network_name, IOC_NETWORK_NAME_SZ);
    os_strncpy(e->device_name, device_name, IOC_NAME_SZ);
    e->device_nr = device_nr;
    os_strncpy(e->mblk_name, mblk_name, IOC_NAME_SZ);

    if (queue->last)
    {
        queue->last->next = e;
    }
    else
    {
        queue->first = e;
    }
    queue->last = e;

    if (queue->event)
    {
        osal_event_set(queue->event);
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get oldest communication event in queue.

  The ioc_release_event_queue() function returns pointer to next event to be processed but
  does not remove it from the queue.

  @param   root Pointer to IOCOM root object.
  @return  Pointer to communication event structure, OS_NULL if no events in queue.

****************************************************************************************************
*/
iocQueuedEvent *ioc_get_event(
    iocRoot *root)
{
    iocEventQueue *queue;
    iocQueuedEvent *e;

    ioc_lock(root);
    queue = root->event_queue;

    if (queue == OS_NULL)
    {
        ioc_unlock(root);
        return OS_NULL;
    }
    e = queue->first;
    ioc_unlock(root);
    return e;
}


/**
****************************************************************************************************

  @brief Remove oldest communication event in queue.

  The ioc_release_event_queue() function pops event away from queue. Called after processing
  event returned by ioc_get_event().

  @param   root Pointer to IOCOM root object.
  @return  OS_TRUE if queue became empty, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean ioc_pop_event(
    iocRoot *root)
{
    iocEventQueue *queue;
    iocQueuedEvent *e;
    os_boolean is_empty;

    ioc_lock(root);
    queue = root->event_queue;

    e = queue->first;
    osal_debug_assert(e != OS_NULL);
    queue->first = e->next;
    if (e->next == OS_NULL)
    {
        queue->last = OS_NULL;
        is_empty = OS_TRUE;
    }
    else
    {
        is_empty = OS_FALSE;
    }

    os_free(e, sizeof(iocQueuedEvent));
    queue->event_count--;

    if (queue->event)
    {
        osal_event_set(queue->event);
    }

    ioc_unlock(root);

    return is_empty;
}

#endif
