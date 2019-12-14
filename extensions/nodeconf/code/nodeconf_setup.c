/**

  @file    nodeconf_setup.c
  @brief   Data structures, defines and functions for managing network node configuration and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/nodeconf/nodeconf.h"


/** Working state structure while parsing node configuration.
 */
typedef struct
{
    /** Pointer to node configuration beging set up.
     */
    iocNodeConf *node;


    /** Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL".
     */
//    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in
        context as device name, like "TEMPCTRL1".
     */
//    os_short device_nr;

    /** Current type as enumeration value, like OS_SHORT. This is set to default
        at beginning of memory block and modified by "type" tag.
     */
    osalTypeId current_type_id;

    /** Current address within memory while parsing. This is updated when a signal
     *  information is added by signal size, or set by "addr" tag.
     */
    os_int current_addr;

    /** Maximum address within memory block (first unused).
     */
    os_int max_addr;

    /** Latest information parsed from JSON
     */
    const os_char *tag;             /* Latest tag or key, '-' for array elements */
    const os_char *mblk_name;       /* The memory block we are now parsing */
    const os_char *group_name;      /* The group we are now parsing */
    const os_char *signal_name;     /* Name of the signal */
    const os_char *signal_type_str; /* Signal type specified in JSON, like "short" */
    os_int signal_addr;             /* Signal address specified in JSON */
    os_int signal_array_n;          /* Number of elements in array, 1 if not array */
    os_int ncolumns;                /* Number of columns when array holds matrix, 1 otherwise. */
}
iocNconfParseState;



static osalStatus ioc_nconf_process_block(
    iocNconfParseState *state,
    os_char *array_tag,
    osalJsonIndex *jindex);

static void ioc_nconf_setup_structure(
    iocNodeConf *node,
    const os_char *config,
    os_memsz config_sz);


/**
****************************************************************************************************

  @brief Load node's network node configuration from persistent storage.

  The ioc_load_node_config()

  @return  None.

****************************************************************************************************
*/
void ioc_load_node_config(
    iocNodeConf *node,
    const os_char *default_config,
    os_memsz default_config_sz)
{
    os_memclear(node, sizeof(iocNodeConf));

    ioc_nconf_setup_structure(node, default_config, default_config_sz);
}

/* void ioc_save_node_config(
    iocNodeConf *node)
{
}
*/

/**
****************************************************************************************************

  @brief Release all memory allocated for node configuration structure.

  The ioc_release_node_config() function releases all memory allocated for
  IO node configuration structure.

  @param   node Pointer to node's network node configuration to release.
  @return  None.

****************************************************************************************************
*/
#if 0
void ioc_release_node_config(
    iocNodeConf *node)
{
#if OSAL_MULTITHREAD_SUPPORT
    osalMutex lock;
    lock = node->lock;
    osal_mutex_lock(lock);
#endif

//    nodeconf_release_string(&node->node_name);
//    nodeconf_release_string(&node->network_name);

    os_memclear(node, sizeof(iocNodeConf));

#if OSAL_MULTITHREAD_SUPPORT
    osal_mutex_unlock(lock);
    osal_mutex_delete(lock);
#endif
}
#endif



/**
****************************************************************************************************

  @brief Processing packed JSON, handle arrays.

  The ioc_nconf_process_array() function is called to process array in packed JSON. General goal
  here is to move IO signals information from packed JSON to dynamic information structures,
  so this information can be seached quickly when needed. Synchronization ioc_lock() must be on
  when this function is called.

  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_nconf_process_array(
    iocNconfParseState *state,
    os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            return OSAL_STATUS_FAILED;
        }

        if (item.code == OSAL_JSON_END_ARRAY)
        {
            return OSAL_SUCCESS;
        }

        state->tag = item.tag_name;

        switch (item.code)
        {
            case OSAL_JSON_START_BLOCK:
                s = ioc_nconf_process_block(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                s = ioc_nconf_process_array(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
            case OSAL_JSON_VALUE_INTEGER:
            case OSAL_JSON_VALUE_FLOAT:
            case OSAL_JSON_VALUE_NULL:
            case OSAL_JSON_VALUE_TRUE:
            case OSAL_JSON_VALUE_FALSE:
                break;

            default:
                return OSAL_STATUS_FAILED;
        }
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Add IO signal to dynamic information.

  The ioc_new_signal_by_info() function adds a new IO signal to dynamic information. This
  function is called when parting packed JSON in info block. Synchronization ioc_lock()
  must be on when this function is called.

  @param   state Structure holding current JSON parsing state.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_new_signal_by_info(
    iocNconfParseState *state)
{
#if 0
    osalTypeId signal_type_id;
    os_int n, sz;

    if (state->signal_type_str)
    {
        signal_type_id = osal_typeid_from_name(state->signal_type_str);
        state->current_type_id = signal_type_id;
    }
    else
    {
        signal_type_id = state->current_type_id;
    }

    if (state->signal_addr > 0)
    {
        state->current_addr = state->signal_addr;
    }

    n = state->signal_array_n;
    if (n < 1) n = 1;

    ioc_add_dynamic_signal(state->dnetwork,
        state->signal_name,
        state->mblk_name,
        state->device_name,
        state->device_nr,
        state->current_addr,
        n,
        state->ncolumns,
        signal_type_id);

    if (signal_type_id == OS_BOOLEAN)
    {
        if (n == 1)
        {
            state->current_addr++;
        }
        else
        {
            sz = (n + 7)/8 + 1;
            state->current_addr += sz;
        }
    }
    else
    {
        sz = (os_int)osal_typeid_size(signal_type_id);
        state->current_addr += n * sz + 1;
    }

    /* Record first unused address to allow automatic resizing
     */
    if (state->current_addr > state->max_addr)
    {
        state->max_addr = state->current_addr;
    }
#endif

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Processing packed JSON, resize a memory block.

  The ioc_resize_memory_block_by_info() function resized a memory block (By making it bigger,
  if needed. Memory block will never be shrunk). This function is used at IO device to
  configure signals and memory block sizes by information in JSON. Synchronization ioc_lock()
  must be on when this function is called.

  @param   state Structure holding current JSON parsing state.
  @return  None.

****************************************************************************************************
*/
static void ioc_resize_memory_block_by_info(
    iocNconfParseState *state)
{
#if 0
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_char *newbuf;
    os_int sz;

    sz = state->max_addr;
    if (sz < IOC_MIN_MBLK_SZ) sz = IOC_MIN_MBLK_SZ;

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        if (mblk->device_nr != state->device_nr) continue;
        if (os_strcmp(mblk->mblk_name, state->mblk_name)) continue;
        if (os_strcmp(mblk->device_name, state->device_name)) continue;
        if (os_strcmp(mblk->network_name, mblk->network_name)) continue;

        if (sz > mblk->nbytes)
        {
            if (mblk->buf_allocated)
            {
                newbuf = ioc_malloc(root, sz, OS_NULL);
                if (newbuf == OS_NULL) return;
                os_memcpy(newbuf, mblk->buf, mblk->nbytes);
                ioc_free(root, mblk->buf, mblk->nbytes);
                mblk->buf = newbuf;
                mblk->nbytes = sz;
            }
#if OSAL_DEBUG
            else
            {
                osal_debug_error("Attempt to resize static memory block");
            }
#endif
        }
        break;
    }
#endif
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle {} blocks.

  The ioc_ioc_nconf_process_block() function is called to process a block in packed JSON. General
  goal here is to move IO signals information from packed JSON to dynamic information structures,
  so this information can be seached quickly when needed. Synchronization ioc_lock() must be on
  when this function is called.

  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_nconf_process_block(
    iocNconfParseState *state,
    os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;
    os_boolean is_signal_block, is_mblk_block;
    os_char array_tag_buf[16];
    iocNodeConf *node;
    node = state->node;

    /* If this is beginning of signal block.
     */
    is_signal_block = OS_FALSE;
    is_mblk_block = OS_FALSE;
    if (!os_strcmp(state->tag, "-"))
    {
        if (!os_strcmp(array_tag, "signals"))
        {
            is_signal_block = OS_TRUE;
            state->signal_addr = -1;
            state->signal_array_n = 1;
            state->ncolumns = 1;
            state->signal_type_str = OS_NULL;
            state->signal_name = OS_NULL;
        }
        else if (!os_strcmp(array_tag, "mblk"))
        {
            is_mblk_block = OS_TRUE;
            state->current_addr = 0;
            state->max_addr = 0;
            state->current_type_id = OS_USHORT;
        }
    }

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            /* If end of signal block, generate the signal
             */
            /* if (is_signal_block)
            {
                return ioc_new_signal_by_info(state);
            }
            if (is_mblk_block && state->resize_mblks)
            {
                ioc_resize_memory_block_by_info(state);
            } */
            return OSAL_SUCCESS;
        }

        if (item.code == OSAL_JSON_END_ARRAY)
        {
            return OSAL_STATUS_FAILED;
        }

        state->tag = item.tag_name;
        switch (item.code)
        {
            case OSAL_JSON_START_BLOCK:
                s = ioc_nconf_process_block(state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                os_strncpy(array_tag_buf, state->tag, sizeof(array_tag_buf));
                s = ioc_nconf_process_array(state, array_tag_buf, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
                if (array_tag[0] == '\0')
                {
                    if (!os_strcmp(state->tag, "device_name"))
                    {
                        node->device_id.device_name = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "device_nr"))
                    {
                        node->device_id.device_nr = (os_int)osal_str_to_int(item.value.s, OS_NULL);
                    }
                    else if (!os_strcmp(state->tag, "network_name"))
                    {
                        node->device_id.network_name = item.value.s;
                    }
                    else if (!os_strcmp(state->tag, "password"))
                    {
                        node->device_id.password = item.value.s;
                    }
                }
                break;

            case OSAL_JSON_VALUE_INTEGER:
                if (array_tag[0] == '\0')
                {
                    if (!os_strcmp(state->tag, "device_nr"))
                    {
                        node->device_id.device_nr = (os_int)item.value.l;
                    }
                }
                if (!os_strcmp(array_tag, "signals"))
                {
                    if (!os_strcmp(state->tag, "addr"))
                    {
                        state->signal_addr = (os_int)item.value.l;
                    }
                    else if (!os_strcmp(state->tag, "array"))
                    {
                        state->signal_array_n = (os_int)item.value.l;
                    }
                    else if (!os_strcmp(state->tag, "ncolumns"))
                    {
                        state->ncolumns = (os_int)item.value.l;
                    }
                }
                break;

            case OSAL_JSON_VALUE_FLOAT:
            case OSAL_JSON_VALUE_NULL:
            case OSAL_JSON_VALUE_TRUE:
            case OSAL_JSON_VALUE_FALSE:
                break;

            default:
                return OSAL_STATUS_FAILED;
        }
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Add information in packed "info" JSON to searchable dynamic IO information structure.

  X...
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static void ioc_nconf_setup_structure(
    iocNodeConf *node,
    const os_char *config,
    os_memsz config_sz)
{
    osalJsonIndex jindex;
    iocNconfParseState state;
    osalStatus s;

    os_memclear(&state, sizeof(state));
    state.node = node;

    s = osal_create_json_indexer(&jindex, config, config_sz, 0);
    if (!s)
    {
        s = ioc_nconf_process_block(&state, "", &jindex);
    }

    if (s)
    {
        osal_debug_error_int("parsing node configuration failed:", s);
    }
}
