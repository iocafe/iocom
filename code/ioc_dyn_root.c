/**

  @file    ioc_dyn_root.c
  @brief   Dynamically maintain IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.11.2019

  The dynamic root holds data structure to managet information about IO networks and signals.
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


typedef struct
{
    /* Pointer to dynamic IO network beging configured.
     */
    iocDynamicNetwork *dnetwork;

    /* TRUE if new dynamic network was created and application callback is needed.
     */
    os_boolean new_network;

    osalTypeId current_type_id;
    os_int current_addr;


    const os_char *tag;
    const os_char *array_tag;
    const os_char *block_tag;

    const os_char *mblk_name;
    const os_char *group_name;
    const os_char *signal_name;
    const os_char *signal_type_str;
    os_int signal_addr;
    os_int signal_array_n;
}
iocAddDinfoState;


static osalStatus ioc_dinfo_process_block(
    iocDynamicRoot *droot,
    iocAddDinfoState *state,
    osalJsonIndex *jindex);


/* Allocate and initialize dynamic root object.
 */
iocDynamicRoot *ioc_initialize_dynamic_root(
    void)
{
    iocDynamicRoot *droot;

    droot = (iocDynamicRoot*)os_malloc(sizeof(iocDynamicRoot), OS_NULL);
    os_memclear(droot, sizeof(iocDynamicRoot));
    return droot;
}

/* Release dynamic root structure.
   LOCK MUST BE ON.
 */
void ioc_release_dynamic_root(
    iocDynamicRoot *droot)
{
    iocDynamicNetwork *dnetwork, *next_dnetwork;
    os_int i;

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

    os_free(droot, sizeof(iocDynamicRoot));
}


/* Set callback function for iocRoot object to inform application about IO network
   connect and disconnects.
   LOCK SHOULD BE ON IF COMMUNICATION IS RUNNING
 */
void ioc_set_dnetwork_callback(
    iocDynamicRoot *droot,
    ioc_dnetwork_callback func,
    void *context)
{
    droot->func = func;
    droot->context = context;
}


/* Add a dynamic network.
 * LOCK must be on.
 */
iocDynamicNetwork *ioc_add_dynamic_network(
    iocDynamicRoot *droot,
    const os_char *network_name)
{
    iocDynamicNetwork *dnetwork, *prev_dnetwork;
    os_uint hash_ix;

    /* If we have existing IO network with this name,
       just return pointer to it.

THIS IS NOT NEEDED, FIND IS USED TO CHECK IF NETWORK EXISTS BEFRE ADDING
     */
    hash_ix = ioc_hash(network_name) % IOC_DROOT_HASH_TAB_SZ;
    prev_dnetwork = OS_NULL;
    for (dnetwork = droot->hash[hash_ix];
         dnetwork;
         dnetwork = dnetwork->next)
    {
        if (!os_strcmp(network_name, dnetwork->network_name))
        {
            return dnetwork;
        }
        prev_dnetwork = dnetwork;
    }

    /* Allocate and initialize a new IO network object.
     */
    dnetwork = ioc_initialize_dynamic_network();
    os_strncpy(dnetwork->network_name, network_name, IOC_NETWORK_NAME_SZ);

    /* Join it as last to linked list for the hash index.
     */
    if (prev_dnetwork)
    {
        prev_dnetwork->next = dnetwork;
    }
    else
    {
        droot->hash[hash_ix] = dnetwork;
    }

    return dnetwork;
}

/* Remove a dynamic network.
   LOCK MUST BE ON.
 */
void ioc_remove_dynamic_network(
    iocDynamicRoot *droot,
    iocDynamicNetwork *dnetwork)
{
    iocDynamicNetwork *dn, *prev_dn;
    os_uint hash_ix;

    /* If we have application callback function, call it.
     */
    if (droot->func)
    {
        droot->func(droot, dnetwork, IOC_DYNAMIC_NETWORK_REMOVED, droot->context);
    }

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


/* Find a dynamic network.
   LOCK MUST BE ON.
 */
iocDynamicNetwork *ioc_find_dynamic_network(
    iocDynamicRoot *droot,
    const os_char *network_name)
{
    iocDynamicNetwork *dnetwork;
    os_uint hash_ix;

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


/* Process a packed JSON array.
 */
static osalStatus ioc_dinfo_process_array(
    iocDynamicRoot *droot,
    iocAddDinfoState *state,
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
                state->block_tag = state->tag;
                s = ioc_dinfo_process_block(droot, state, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                state->array_tag = state->tag;
                s = ioc_dinfo_process_array(droot, state, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
                // item.value.s;
                break;

            case OSAL_JSON_VALUE_INTEGER:
                // item.value.l
                break;

            case OSAL_JSON_VALUE_FLOAT:
                // item.value.d;
                break;

            /* Handling OSAL_JSON_VALUE_NULL, OSAL_JSON_VALUE_TRUE, and
               OSAL_JSON_VALUE_FALSE is needed only if compressed with
               OSAL_JSON_KEEP_QUIRKS flag.
             */
            case OSAL_JSON_VALUE_NULL:
                // p = "null";
                break;

            case OSAL_JSON_VALUE_TRUE:
                //p = "true";
                break;

            case OSAL_JSON_VALUE_FALSE:
                //p = "false";
                break;

            default:
                return OSAL_STATUS_FAILED;
        }
    }

    return OSAL_SUCCESS;
}

static osalStatus ioc_new_signal_by_info(
    iocAddDinfoState *state)
{
    osalTypeId signal_type_id;
    os_memsz sz;
    os_int n;

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
        state->current_addr,
        n,
        signal_type_id,
        OS_NULL);

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
        sz = osal_typeid_size(signal_type_id);
        state->current_addr += n * sz + 1;
    }

    return OSAL_SUCCESS;
}


/* Process a block of packed JSON.
 */
static osalStatus ioc_dinfo_process_block(
    iocDynamicRoot *droot,
    iocAddDinfoState *state,
    osalJsonIndex *jindex)
{
    osalJsonItem item;
    osalStatus s;
    os_boolean is_signal_block;

    /* If this is beginning of signal block.
     */
    is_signal_block = OS_FALSE;
    if (!os_strcmp(state->block_tag, "-"))
    {
        if (!os_strcmp(state->array_tag, "signals"))
        {
            is_signal_block = OS_TRUE;
            state->signal_addr = -1;
            state->signal_array_n = 1;
            state->signal_type_str = OS_NULL;
            state->signal_name = OS_NULL;
        }
        else if (!os_strcmp(state->array_tag, "mblk"))
        {
            state->current_addr = 0;
            state->current_type_id = OS_USHORT;
        }
    }

    while (!(s = osal_get_json_item(jindex, &item)))
    {
        if (item.code == OSAL_JSON_END_BLOCK)
        {
            /* If end of signal block, generate the signal
             */
            if (is_signal_block)
            {
                return ioc_new_signal_by_info(state);
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
                state->block_tag = state->tag;
                s = ioc_dinfo_process_block(droot, state, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_START_ARRAY:
                state->array_tag = state->tag;
                s = ioc_dinfo_process_array(droot, state, jindex);
                if (s) return s;
                break;

            case OSAL_JSON_VALUE_STRING:
                if (!os_strcmp(state->tag, "name"))
                {
                    if (!os_strcmp(state->array_tag, "mblk"))
                    {
                        state->mblk_name = item.value.s;
                    }

                    else if (!os_strcmp(state->array_tag, "groups"))
                    {
                        state->group_name = item.value.s;
                        if (!os_strcmp(state->group_name, "inputs") ||
                            !os_strcmp(state->group_name, "outputs"))
                        {
                            state->current_type_id = OS_BOOLEAN;
                        }
                    }

                    else if (!os_strcmp(state->array_tag, "signals"))
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
                if (!os_strcmp(state->array_tag, "signals"))
                {
                    if (!os_strcmp(state->tag, "addr"))
                    {
                        state->signal_addr = (os_int)item.value.l;
                    }
                    else if (!os_strcmp(state->tag, "array"))
                    {
                        state->signal_array_n = (os_int)item.value.l;
                    }

                }
                // item.value.l
                break;

            case OSAL_JSON_VALUE_FLOAT:
                // item.value.d;
                break;

            /* Handling OSAL_JSON_VALUE_NULL, OSAL_JSON_VALUE_TRUE, and
               OSAL_JSON_VALUE_FALSE is needed only if compressed with
               OSAL_JSON_KEEP_QUIRKS flag.
             */
            case OSAL_JSON_VALUE_NULL:
                // p = "null";
                break;

            case OSAL_JSON_VALUE_TRUE:
                //p = "true";
                break;

            case OSAL_JSON_VALUE_FALSE:
                //p = "false";
                break;

            default:
                return OSAL_STATUS_FAILED;
        }
    }

    return OSAL_SUCCESS;
}


/* Add dynamic memory block/signal information.
 */
osalStatus ioc_add_dynamic_info(
    iocDynamicRoot *droot,
    iocHandle *mblk_handle)
{
    iocRoot *root;
    iocMemoryBlock *mblk;
    osalJsonIndex jindex;
    osalStatus s;
    iocAddDinfoState state;

    /* Get memory block pointer and start synchronization.
     */
    mblk = ioc_handle_lock_to_mblk(mblk_handle, &root);
    if (mblk == OS_NULL) return OSAL_STATUS_FAILED;

    os_memclear(&state, sizeof(state));

    s = osal_create_json_indexer(&jindex, mblk->buf, mblk->nbytes, 0);
    if (s) goto getout;

    /* Make sure that we have network with this name.
     */
    state.dnetwork = ioc_find_dynamic_network(droot, mblk->network_name);
    if (state.dnetwork == OS_NULL)
    {
        state.dnetwork = ioc_add_dynamic_network(droot, mblk->network_name);
        state.new_network = OS_TRUE;
    }

    // ioc_add_mblk_to_network(); ???

    s = ioc_dinfo_process_block(droot, &state, &jindex);
    if (s) goto getout;

    /* If new network was created and we have application callback function.
     */
    if (state.new_network && droot->func)
    {
        droot->func(droot, state.dnetwork, IOC_NEW_DYNAMIC_NETWORK, droot->context);
    }

    /* End syncronization and return.
     */
getout:
    ioc_unlock(root);
    return s;
}


/* Called by ioc_release_memory_block(): memory block is being deleted, remove any references
   to it from dynamic configuration.
 */
void ioc_droot_mblk_is_deleted(
    iocDynamicRoot *droot,
    iocMemoryBlock *mblk)
{
    iocDynamicNetwork *dnetwork;

    dnetwork = ioc_find_dynamic_network(droot, mblk->network_name);
    if (dnetwork)
    {
        ioc_network_mblk_is_deleted(dnetwork, mblk);
    }
}


/* Calculate hash index for the key
 */
os_uint ioc_hash(
    const os_char *key_str)
{
    const os_uint primes[] = {47, 2, 43, 3, 41, 5, 37, 7, 31, 11, 29, 13, 23, 17, 19};
    const os_int n_primes = sizeof(primes) / sizeof(os_uint);
    os_ulong hash_sum;
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
