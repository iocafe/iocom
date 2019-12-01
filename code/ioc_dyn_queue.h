/**

  @file    ioc_dyn_queue.h
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.11.2019

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
#ifndef IOC_DYN_QUEUE_INCLUDED
#define IOC_DYN_QUEUE_INCLUDED
#if IOC_DYNAMIC_MBLK_CODE

/**
****************************************************************************************************
  Queued communication event
****************************************************************************************************
*/
typedef struct iocQueuedEvent
{
    /** Which event: IOC_NEW_MEMORY_BLOCK, IOC_NEW_NETWORK...
     */
    iocEvent event;

    /** Network name.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Device name and number.
     */
    os_char device_name[IOC_NAME_SZ];
    os_short device_nr;

    /** Memory block name.
     */
    os_char mblk_name[IOC_NAME_SZ];
    
    /* Pointer to next event in queue
     */
    struct iocQueuedEvent *next;
}
iocQueuedEvent;


/**
****************************************************************************************************
  Communication event queue structure
****************************************************************************************************
*/
typedef struct iocEventQueue
{
    /** Pointer to iocom root object.
     */
    iocRoot *root;

    /** First and last event in the queue. Last is the newest.
     */
    iocQueuedEvent *first, *last;

    /** Operating system event to set when new event is
        placed into queue.
     */
    osalEvent event;

    /** Which communication events we wish to queue, bits: IOC_MBLK_EVENTS,
        IOC_DEVICE_EVENTS, IOC_NETWORK_EVENTS.
     */
    os_int flags;
    
    /** Number of events queued at the moment.
     */
    os_int event_count;

    /** Maximum number of events to queue. This should be quite
        a big number which never is reached. Limit is there
        just to avoid allocating all memory of application doesn't
        process the events.
     */
    os_int max_nro_events;
}
iocEventQueue;


/**
****************************************************************************************************
  Communication event queue functions
****************************************************************************************************
*/

/* Flags for ioc_initialize_event_queue
 */
#define IOC_MBLK_EVENTS 1
#define IOC_DEVICE_EVENTS 2
#define IOC_NETWORK_EVENTS 4

/* Start queueing communication events for the application.
 */
osalStatus ioc_initialize_event_queue(
    iocRoot *root,
    osalEvent event,
    os_int max_nro_events,
    os_int flags);

/* Release any resources allocated for the event queue
   (doesn't free flat memory allocated for the queue structure)
 */
void ioc_release_event_queue(
    iocRoot *root);

/* Queue a communication event to inform application about it.
 */
osalStatus ioc_queue_event(
    iocRoot *root,
    iocEvent event,
    const os_char *network_name,
    const os_char *device_name,
    os_short device_nr,
    const os_char *mblk_name);

/* Return pointer to next event to be processed but do not
   remove it from queue.
 */
iocQueuedEvent *ioc_get_event(
    iocRoot *root);

/* Pop event away from queue. Called after processing
   event returned by ioc_get_event().
 */
void ioc_pop_event(
    iocRoot *root);

#endif
#endif
