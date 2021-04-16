/**

  @file    switchbox_root.h
  @brief   Switchbox root object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef SWITCHBOX_ROOT_H_
#define SWITCHBOX_ROOT_H_
#include "extensions/switchbox/switchbox.h"

/* Structures which are defined later in headers.
 */
struct switchboxConnection;
struct switchboxEndPoint;


/**
****************************************************************************************************
    Linked list of root's connections
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the first connection in linked list.
     */
    struct switchboxConnection *first;

    /** Pointer to the last connection in linked list.
     */
    struct switchboxConnection *last;
}
switchboxConnectionList;


/**
****************************************************************************************************
    Linked list of root's end points
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the first end point in linked list.
     */
    struct switchboxEndPoint *first;

    /** Pointer to the last end point in linked list.
     */
    struct switchboxEndPoint *last;
}
switchboxEndPointList;


/**
****************************************************************************************************

  @name Root object structure.

  The switchboxRoot is the root of communication object hierarchy. It maintains lists of memory
  buffers and connections, and if multithreading is supported, mutex to synchronize access
  to the commmunication object hierarcy.

****************************************************************************************************
*/
typedef struct switchboxRoot
{
    /** Mutex to synchronize access to communication object hierarchy.
     */
    osalMutex mutex;

    /** Counter for new client identifiers.
     */
    os_ushort current_client_id;

    /** Linked list of root's connections.
     */
    switchboxConnectionList con;

    /** Linked list of root's end points.
     */
    switchboxEndPointList epoint;
}
switchboxRoot;


/**
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_switchbox_root() function initializes or allocates new communication root objects,
  and ioc_release_switchbox_root() releases resources associated with it. Memory allocated for the root
  object is freed, if it was allocated by ioc_initialize_switchbox_root().

****************************************************************************************************
 */
/*@{*/

/* Initialize communication root object.
 */
void ioc_initialize_switchbox_root(
    switchboxRoot *root,
    os_char flags);

/* Release communication root object.
 */
void ioc_release_switchbox_root(
    switchboxRoot *root);

/* Lock the communication object hierarchy.
 */
void ioc_switchbox_lock(
    switchboxRoot *root);

/* Unlock the communication object hierarchy.
 */
void ioc_switchbox_unlock(
    switchboxRoot *root);

/* Find service connection by network name.
 */
struct switchboxConnection *ioc_switchbox_find_service_connection(
    switchboxRoot *root,
    const os_char *network_name,
    struct switchboxConnection *exclude_con);

/* Get new unique client id.
 */
os_ushort ioc_new_switchbox_client_id(
    switchboxRoot *root);

/*@}*/


#endif
