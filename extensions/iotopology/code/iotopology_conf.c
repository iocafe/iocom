/**

  @file    iotopology_conf.c
  @brief   Data structures, defines and functions for managing network topology and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iotopology/iotopology.h"


/* Prototyped for forward referred static functions.
 */
static void iotopology_set_string(
    iotopologyNode *node,
    os_char **pstr,
    const os_char *x);

static void iotopology_get_string(
    iotopologyNode *node,
    os_char **pstr,
    os_char *buf,
    os_memsz buf_sz);

static void iotopology_release_string(
    os_char **pstr);




/**
****************************************************************************************************

  @brief Initialize node configuration structure.

  The iotopology_initialize_node_configuration() function initalizes iotopologyNode structure
  and creates mutex to synchronize access to node configuration information.

  @return  None.

****************************************************************************************************
*/
void iotopology_initialize_node_configuration(
    iotopologyNode *node)
{
    os_memclear(node, sizeof(iotopologyNode));

#if OSAL_MULTITHREAD_SUPPORT
    node->lock = osal_mutex_create();
#endif
}


/**
****************************************************************************************************

  @brief Release all memory allocated for node configuration structure.

  The iotopology_release_node_configuration() function releases all memory allocated for
  IO node configuration structure.

  @return  None.

****************************************************************************************************
*/
void iotopology_release_node_configuration(
    iotopologyNode *node)
{
#if OSAL_MULTITHREAD_SUPPORT
    osalMutex lock;
    lock = node->lock;
    osal_mutex_lock(lock);
#endif

    iotopology_release_string(&node->node_name);
    iotopology_release_string(&node->network_name);

    os_memclear(node, sizeof(iotopologyNode));

#if OSAL_MULTITHREAD_SUPPORT
    osal_mutex_unlock(lock);
    osal_mutex_delete(lock);
#endif
}


void iotopology_set_node_name(
    iotopologyNode *node,
    const os_char *node_name)
{
    iotopology_set_string(node, &node->node_name, node_name);
}


void iotopology_get_node_name(
    iotopologyNode *node,
    os_char *node_name,
    os_memsz node_name_sz)
{
    iotopology_get_string(node, &node->node_name, node_name, node_name_sz);
}


/**
****************************************************************************************************

  @brief Store copy of string in newly allocated memory.

  The iotopology_set_string() strores copy of string x and sets pstr to point it.

  @param   pstr Pointer to string pointer used to hold copy. This must be initialized before
           calling this function either to OS_NULL or set by earlier iotopology_set_string()
           function call.
  @param   x Pointer to new string value. If OS_NULL, the pstr pointer is set to OS_NULL.
  @return  None.

****************************************************************************************************
*/
static void iotopology_set_string(
    iotopologyNode *node,
    os_char **pstr,
    const os_char *x)
{
    os_memsz sz;

    iotopology_lock_node_configuration(node);

    /* If we have old string value, release memory allocated for it.
     */
    iotopology_release_string(pstr);

    /* If we have non empty string argument x, allocate memory for it and store a copy
       of string.
     */
    if (x) if (*x != '\0')
    {
        sz = os_strlen(x);

        *pstr = os_malloc(sz, OS_NULL);
        os_memcpy(*pstr, x, sz);
    }

    iotopology_unlock_node_configuration(node);
}


/**
****************************************************************************************************

  @brief Store copy of string in newly allocated memory.

  The iotopology_set_string() strores copy of string x and sets pstr to point it.

  @param   pstr Pointer to string pointer used to hold copy. This must be initialized before
           calling this function either to OS_NULL or set by earlier iotopology_set_string()
           function call.
  @param   x Pointer to new string value. If OS_NULL, the pstr pointer is set to OS_NULL.
  @return  None.

****************************************************************************************************
*/
static void iotopology_get_string(
    iotopologyNode *node,
    os_char **pstr,
    os_char *buf,
    os_memsz buf_sz)
{
    iotopology_lock_node_configuration(node);
    os_strncpy(buf, *pstr, buf_sz);
    iotopology_unlock_node_configuration(node);
}


/**
****************************************************************************************************

  @brief Release memory allocated for string.

  The iotopology_release_string() releases memoty allocated to hold a string, if any.

  @param   pstr Pointer to string pointer used to hold copy. This must be initialized before
           calling this function either to OS_NULL or set by earlier iotopology_set_string()
           function call. At return pstr will be OS_NULL.
  @return  None.

****************************************************************************************************
*/
static void iotopology_release_string(
    os_char **pstr)
{
    if (*pstr)
    {
        os_free(*pstr, os_strlen(*pstr));
        *pstr = OS_NULL;
    }
}
