/**

  @file    ioc_dyn_root.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  The dynamic root holds data structure to manage information about IO networks and signals.
  It is used to convert io path (signal name, memory block name, device name and number, network
  name) to IO signal object pointers, or to memory block pointers.

  An IO path can be split to individual identifiers by ioc_iopath_to_identifiers() function.
  The network name and signal name are used as hash keys, since these are known explisitely
  by application and are efficient for the purpose.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE

/** Working state structure while adding signals to dynamic incormation.
 */
typedef struct
{
    /** Pointer to iocom root object
     */
    iocRoot *root;

    /** Pointer to dynamic IO network beging configured.
     */
    iocDynamicNetwork *dnetwork;

    /** Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL".
     */
    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in
        context as device name, like "TEMPCTRL1".
     */
    os_uint device_nr;

    /** Resize memory blocks while parsing flag.
     */
    os_boolean resize_mblks;

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

    /** Trick to get memory block name before processing signals. "groups" position
        is stored here to return to signals after memory block name has been received.
     */
    osalJsonIndex mblk_groups_jindex;
    os_boolean mblk_groups_jindex_set;
}
iocAddDinfoState;


/* Forward referred static functions.
 */
static osalStatus ioc_dinfo_process_block(
    iocDynamicRoot *droot,
    iocAddDinfoState *state,
    const os_char *array_tag,
    osalJsonIndex *jindex);


/**
****************************************************************************************************

  @brief Allocate and initialize dynamic root object.

  The ioc_initialize_dynamic_root() function allocated and initializes root structure for
  storing dynamic signal, memory block and network information in quickly searcable format.
  The allocated structure is bound to IOCOM root. If dynamic information is to be used, this
  function should be called right after initializing IOCOM root structure with ioc_initialize_root().

  @param   root Pointer to IOCOM root object.
  @return  Pointer to dynamic information root structure, or OS_NULL if memory allocation failed.

****************************************************************************************************
*/
iocDynamicRoot *ioc_initialize_dynamic_root(
    iocRoot *root)
{
    iocDynamicRoot *droot;

    droot = (iocDynamicRoot*)os_malloc(sizeof(iocDynamicRoot), OS_NULL);
    if (droot == OS_NULL) return OS_NULL;
    os_memclear(droot, sizeof(iocDynamicRoot));
    droot->root = root;
    root->droot = droot;
    return droot;
}


/**
****************************************************************************************************

  @brief Release dynamic root structure.

  The ioc_release_dynamic_root() function releases dynamic information root and all substructures
  allocated for storing dynamic information. This function is called by ioc_release_root()
  and must not be called directly from application. Synchronization ioc_lock() must be on
  when this function is called.

  @param   droot Pointer to dynamic information root structure to release. If OS_NULL, the
           function does nothing.
  @return  None.

****************************************************************************************************
*/
void ioc_release_dynamic_root(
    iocDynamicRoot *droot)
{
    iocDynamicNetwork *dnetwork, *next_dnetwork;
    os_int i;

    if (droot == OS_NULL) return;

    for (i = 0; i < IOC_DROOT_HASH_TAB_SZ; i++)
    {
        for (dnetwork = droot->hash[i];
             dnetwork;
             dnetwork = next_dnetwork)
        {
            next_dnetwork = dnetwork->next;
            ioc_release_dynamic_network(dnetwork);
        }
    }

    if (droot->root)
    {
        droot->root->droot = OS_NULL;
    }

    os_free(droot, sizeof(iocDynamicRoot));
}


/**
****************************************************************************************************

  @brief Add an IO device network to dynamic information.

  The ioc_add_dynamic_network() function adds a IO device network structure with specified
  network name to dynamic information. Synchronization ioc_lock() must be on when this
  function is called.

  @param   droot Pointer to dynamic information root structure. The network information is
           stored "within" this root.
  @param   network_name Name of IO device network.
  @return  Pointer to dynamic network information structure, or OS_NULL if memory allocation
           failed.

****************************************************************************************************
*/
iocDynamicNetwork *ioc_add_dynamic_network(
    iocDynamicRoot *droot,
    const os_char *network_name)
{
    iocDynamicNetwork *dnetwork;
    os_uint hash_ix;

    /* If we already have network with this name.
     */
    dnetwork = ioc_find_dynamic_network(droot, network_name);
    if (dnetwork) return dnetwork;

    /* If we have existing IO network with this name,
       just return pointer to it.
     */
    hash_ix = ioc_hash(network_name) % IOC_DROOT_HASH_TAB_SZ;

    /* Allocate and initialize a new IO network object.
     */
    dnetwork = ioc_initialize_dynamic_network();
    if (dnetwork == OS_NULL) return OS_NULL;
    os_strncpy(dnetwork->network_name, network_name, IOC_NETWORK_NAME_SZ);
    dnetwork->new_network = OS_TRUE;

    /* Join it as last to linked list for the hash index.
     */
    dnetwork->next = droot->hash[hash_ix];
    droot->hash[hash_ix] = dnetwork;

    return dnetwork;
}


/**
****************************************************************************************************

  @brief Remove an IO device network from dynamic information.

  The ioc_remove_dynamic_network() function removes an IO device network structure from dynamic
  information. This gets called by ioc_network_mblk_is_deleted() function when the last memory
  block of a network is released. Synchronization ioc_lock() must be on when this
  function is called.

  @param   droot Pointer to dynamic information root structure.
  @param   dnetwork Pointer to dynamic network structure to remove.
  @return  None.

****************************************************************************************************
*/
void ioc_remove_dynamic_network(
    iocDynamicRoot *droot,
    iocDynamicNetwork *dnetwork)
{
    iocDynamicNetwork *dn, *prev_dn;
    os_uint hash_ix;

    /* If we application needs to be informed?
     */
    ioc_new_root_event(droot->root, IOC_NETWORK_DISCONNECTED, dnetwork,
        OS_NULL, droot->root->callback_context);

    /* Fond out who has pointer to dnetwork in prev_dn.
     * If none, dnetwork is first in list and prev_dn is OS_NULL.
     */
    hash_ix = ioc_hash(dnetwork->network_name) % IOC_DROOT_HASH_TAB_SZ;
    prev_dn = OS_NULL;
    for (dn = droot->hash[hash_ix];
         dn && dn != dnetwork;
         dn = dn->next)
    {
        prev_dn = dn;
    }

    /* Remove from linked list.
     */
    if (prev_dn)
    {
        prev_dn->next = dnetwork->next;
    }
    else
    {
        droot->hash[hash_ix] = dnetwork->next;
    }

    /* Release the dynamic network object.
     */
    ioc_release_dynamic_network(dnetwork);
}


/**
****************************************************************************************************

  @brief Find dynamic IO device network information.

  The ioc_find_dynamic_network() function searches for an IO device network by name from
  dynamic information. Synchronization ioc_lock() must be on when this function is called.

  @param   droot Pointer to dynamic information root structure.
  @param   network_name Network name to search for.
  @return  Pointer to device IO network information struture, if one is found. OS_NULL if none
           was found.

****************************************************************************************************
*/
iocDynamicNetwork *ioc_find_dynamic_network(
    iocDynamicRoot *droot,
    const os_char *network_name)
{
    iocDynamicNetwork *dnetwork;
    os_uint hash_ix;

    if (droot == OS_NULL) return OS_NULL;

    hash_ix = ioc_hash(network_name) % IOC_DROOT_HASH_TAB_SZ;
    for (dnetwork = droot->hash[hash_ix];
         dnetwork;
         dnetwork = dnetwork->next)
    {
        if (!os_strcmp(network_name, dnetwork->network_name))
        {
            return dnetwork;
        }
    }

    return OS_NULL;
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle arrays.

  The ioc_dinfo_process_array() function is called to process array in packed JSON. General goal
  here is to move IO signals information from packed JSON to dynamic information structures,
  so this information can be seached quickly when needed. Synchronization ioc_lock() must be on
  when this function is called.

  @param   droot Pointer to dynamic information root structure.
  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_dinfo_process_array(
    iocDynamicRoot *droot,
    iocAddDinfoState *state,
    const os_char *array_tag,
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

        if (!os_strcmp(array_tag, "mblk"))
        {
            state->mblk_name = OS_NULL;
            state->mblk_groups_jindex_set = OS_FALSE;
        }

        state->tag = item.tag_name;

        switch (item.code)
        {
            case OSAL_JSON_START_BLOCK:
                s = ioc_dinfo_process_block(droot, state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                s = ioc_dinfo_process_array(droot, state, array_tag, jindex);
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
    iocAddDinfoState *state)
{
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

    /* We must accept address 0 as valid setting mark unspecified address with -1 */
    if (state->signal_addr >= 0)
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

    switch(signal_type_id)
    {
        case OS_BOOLEAN:
            if (n == 1)
            {
                state->current_addr++;
            }
            else
            {
                sz = (n + 7)/8 + 1;
                state->current_addr += sz;
            }
            break;

        case OS_STR:
            state->current_addr += n + 1;
            break;

        default:
            sz = (os_int)osal_type_size(signal_type_id);
            state->current_addr += n * sz + 1;
            break;
    }

    /* Record first unused address to allow automatic resizing
     */
    if (state->current_addr > state->max_addr)
    {
        state->max_addr = state->current_addr;
    }

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
    iocAddDinfoState *state)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    os_char *newbuf;
    os_int sz;

    root = state->root;
    sz = state->max_addr;
    if (sz < IOC_MIN_MBLK_SZ) sz = IOC_MIN_MBLK_SZ;

#if IOC_MBLK_SPECIFIC_DEVICE_NAME==0
    if (root->device_nr != state->device_nr) return;
    if (os_strcmp(root->device_name, state->device_name)) return;
    /* if (os_strcmp(root->network_name, state->network_name)) return; */
#endif

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
        if (mblk->device_nr != state->device_nr) continue;
        if (os_strcmp(mblk->device_name, state->device_name)) continue;
        /* if (os_strcmp(mblk->network_name, state->network_name)) continue;  */
#endif
        if (os_strcmp(mblk->mblk_name, state->mblk_name)) continue;

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
}


/**
****************************************************************************************************

  @brief Processing packed JSON, handle {} blocks.

  The ioc_dinfo_process_block() function is called to process a block in packed JSON. General
  goal here is to move IO signals information from packed JSON to dynamic information structures,
  so this information can be seached quickly when needed. Synchronization ioc_lock() must be on
  when this function is called.

  @param   droot Pointer to dynamic information root structure.
  @param   state Structure holding current JSON parsing state.
  @param   array_tag Name of array from upper level of JSON structure.
  @param   jindex Current packed JSON parsing position.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_dinfo_process_block(
    iocDynamicRoot *droot,
    iocAddDinfoState *state,
    const os_char *array_tag,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;
    os_boolean is_signal_block, is_mblk_block;
    os_char array_tag_buf[16];

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
            /* If end of signal block and we got memory block name, generate the signal
             */
            if (is_signal_block)
            {
                if (state->mblk_name == OS_NULL) {
                    return OSAL_SUCCESS;
                }
                return ioc_new_signal_by_info(state);
            }
            if (is_mblk_block && state->resize_mblks)
            {
                ioc_resize_memory_block_by_info(state);
            }
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
                s = ioc_dinfo_process_block(droot, state, array_tag, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                os_strncpy(array_tag_buf, state->tag, sizeof(array_tag_buf));
                if (!os_strcmp(array_tag_buf, "groups") && state->mblk_name == OS_NULL) {
                    state->mblk_groups_jindex = *jindex;
                    state->mblk_groups_jindex_set = OS_TRUE;
                }
                s = ioc_dinfo_process_array(droot, state, array_tag_buf, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
                if (!os_strcmp(state->tag, "name"))
                {
                    if (!os_strcmp(array_tag, "mblk"))
                    {
                        state->mblk_name = item.value.s;

                        if (state->mblk_groups_jindex_set) {
                            s = ioc_dinfo_process_array(droot, state, "groups", &state->mblk_groups_jindex);
                            if (s) return s;
                        }
                    }

                    else if (!os_strcmp(array_tag, "groups"))
                    {
                        state->group_name = item.value.s;
                        if (!os_strcmp(state->group_name, "inputs") ||
                            !os_strcmp(state->group_name, "outputs"))
                        {
                            state->current_type_id = OS_BOOLEAN;
                        }
                    }

                    else if (!os_strcmp(array_tag, "signals"))
                    {
                        state->signal_name = item.value.s;
                    }
                }

                if (!os_strcmp(state->tag, "type"))
                {
                    state->signal_type_str = item.value.s;
                }
                break;

            case OSAL_JSON_VALUE_INTEGER:
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

  The ioc_add_dynamic_info() function adds memory block and signal information for
  an IO device to searchable dynamic structures. In server, this is called when "info" memory
  block is received from IO device, and in dynamically implemented IO device this can
  used to publish information in JSON.

  @param   mblk_handle "info" memory block handle.
  @param   resize_mblk OS_TRUE to resize memory blocks. This should be OS_TRUE for dynampically
           implmented IO device and OS_FALSE for server end.
  @return  OSAL_SUCCESS if all is fine, other values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_add_dynamic_info(
    iocHandle *mblk_handle,
    os_boolean resize_mblks)
{
    iocRoot *root;
    iocDynamicRoot *droot;
    iocMemoryBlock *mblk;
    osalJsonIndex jindex;
    osalStatus s;
    iocAddDinfoState state;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(mblk_handle, &root);
    if (mblk == OS_NULL) return OSAL_STATUS_FAILED;
    droot = root->droot;

    os_memclear(&state, sizeof(state));
    state.root = root;
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    os_strncpy(state.device_name, mblk->device_name, IOC_NAME_SZ);
    state.device_nr = mblk->device_nr;
#else
    os_strncpy(state.device_name, root->device_name, IOC_NAME_SZ);
    state.device_nr = root->device_nr;
#endif
    state.resize_mblks = resize_mblks;

    s = osal_create_json_indexer(&jindex, mblk->buf, mblk->nbytes, 0);
    if (s) goto getout;

    /* Make sure that we have network with this name.
     */
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    state.dnetwork = ioc_add_dynamic_network(droot, mblk->network_name);
#else
    state.dnetwork = ioc_add_dynamic_network(droot, root->network_name);
#endif
    if (state.dnetwork == OS_NULL)
    {
        s = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
        goto getout;
    }

    s = ioc_dinfo_process_block(droot, &state, osal_str_empty, &jindex);
    if (s) goto getout;

    /* Add info block to dynamic shortcuts (if not somehow already there)
     */
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    if (ioc_find_mblk_shortcut(state.dnetwork, mblk->mblk_name,
        mblk->device_name, mblk->device_nr) == OS_NULL)
#else
    if (ioc_find_mblk_shortcut(state.dnetwork, mblk->mblk_name,
        root->device_name, root->device_nr) == OS_NULL)
#endif
    {
        ioc_add_mblk_shortcut(state.dnetwork, mblk);
    }

    /* Informn application about new networks and devices.
     */
    if (state.dnetwork->new_network)
    {
        ioc_new_root_event(root, IOC_NEW_NETWORK, state.dnetwork, OS_NULL, root->callback_context);
        state.dnetwork->new_network = OS_FALSE;
    }
    ioc_new_root_event(root, IOC_NEW_DEVICE, state.dnetwork, mblk, root->callback_context);

    /* Flag for basic server (iocBServer). Check for missing certificate chain and
       flash program versions.
     */
    root->check_cert_chain_etc = OS_TRUE;

    /* End syncronization and return.
     */
getout:
    ioc_unlock(root);
    return s;
}


/**
****************************************************************************************************

  @brief Delete all dynamic signal information related to a memory block.
  @anchor ioc_dynamic_mblk_is_deleted

  The ioc_dynamic_mblk_is_deleted() is called when a memory block is about to be deleted from
  the IO device network by ioc_release_memory_block() function. All dynamic signal information
  related to the memory block is deleted.

  Root lock must be on when calling this function.

  @param   droot Pointer to dynamic root object.
  @param   mblk Pointer to memory block object being deleted.
  @return  None.

****************************************************************************************************
*/
void ioc_dynamic_mblk_is_deleted(
    iocDynamicRoot *droot,
    iocMemoryBlock *mblk)
{
    iocDynamicNetwork *dnetwork;

    if (droot == OS_NULL) return;

#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    dnetwork = ioc_find_dynamic_network(droot, mblk->network_name);
#else
    dnetwork = ioc_find_dynamic_network(droot, mblk->link.root->network_name);
#endif
    if (dnetwork)
    {
        ioc_network_mblk_is_deleted(dnetwork, mblk);
    }
}


/**
****************************************************************************************************

  @brief Calculate hash index for the key

  The ioc_hash function() calculates hash sum from string key given as argument. Both IO
  device networks and signal do use hash table to speed up seaching dynamic information.

  @param   key_str Key string, has sum is caluclated from this.
  @return  Hash sum, final has index is reminder of dividing this value by hash table size.

****************************************************************************************************
*/
os_uint ioc_hash(
    const os_char *key_str)
{
    const os_uint primes[] = {47, 2, 43, 3, 41, 5, 37, 7, 31, 11, 29, 13, 23, 17, 19};
    const os_int n_primes = sizeof(primes) / sizeof(os_uint);
    os_uint hash_sum;
    os_int prime_ix;
    os_uchar c;

    hash_sum = 0;
    prime_ix = 0;
    while ((c = *(key_str++)))
    {
        hash_sum += c * primes[prime_ix];
        if (++prime_ix >= n_primes) prime_ix = 0;
    }

    return hash_sum;
}

#endif
