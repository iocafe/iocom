/**

  @file    devicedir_dynamic.c
  @brief   List dynamic signals.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.11.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "devicedir.h"
#if IOC_DYNAMIC_MBLK_CODE


/* Forward referred static functions.
 */
static void devicedir_networks_dynamic_signals(
    iocDynamicNetwork *dnetwork,
    osalStream list,
    os_short flags,
    os_boolean *is_first);


/**
****************************************************************************************************

  @brief List end points connections of this node.

  The devicedir_dynamic_signals() function lists all dynamic signals.

  @param   root Pointer to the root structure.
  @param   list Steam handle into which to write connection list JSON
  @param   flags Reserved for future, set 0.
  @return  None.

****************************************************************************************************
*/
void devicedir_dynamic_signals(
    iocRoot *root,
    osalStream list,
    os_short flags)
{
    iocDynamicRoot *droot;
    iocDynamicNetwork *dnetwork;
    os_int i;
    os_boolean is_first;

    /* Check that root object is valid pointer.
     */
    osal_debug_assert(root->debug_id == 'R');

    /* Synchronize.
     */
    ioc_lock(root);

    droot = root->droot;
    if (droot == OS_NULL)
    {
        osal_stream_print_str(list, "Dynamic signal information not used by the application", 0);
        ioc_unlock(root);
        return;
    }

    osal_stream_print_str(list, "{\"signal\": [", 0);


    is_first = OS_TRUE;
    for (i = 0; i < IOC_DROOT_HASH_TAB_SZ; i++)
    {
        for (dnetwork = droot->hash[i];
             dnetwork;
             dnetwork = dnetwork->next)
        {
            devicedir_networks_dynamic_signals(dnetwork, list, flags, &is_first);
        }
    }

    /* End synchronization.
     */
    ioc_unlock(root);
    osal_stream_print_str(list, "\n]}\n", 0);
}


static void devicedir_networks_dynamic_signals(
    iocDynamicNetwork *dnetwork,
    osalStream list,
    os_short flags,
    os_boolean *is_first)
{
    iocDynamicSignal *dsignal;
    os_int i;

    for (i = 0; i < IOC_DNETWORK_HASH_TAB_SZ; i++)
    {
        for (dsignal = dnetwork->hash[i];
             dsignal;
             dsignal = dsignal->next)
        {
            if (!*is_first)
            {
                osal_stream_print_str(list, ",\n", 0);
            }
            *is_first = OS_FALSE;

            osal_stream_print_str(list, "{", 0);
            devicedir_append_str_param(list, "signal_name", dsignal->signal_name, OS_TRUE);
            devicedir_append_str_param(list, "mblk_name", dsignal->mblk_name, OS_FALSE);
            devicedir_append_str_param(list, "device_name", dsignal->device_name, OS_FALSE);
            devicedir_append_int_param(list, "device_nr", dsignal->device_nr, OS_FALSE);
            devicedir_append_str_param(list, "network_name", dnetwork->network_name, OS_FALSE);
            devicedir_append_int_param(list, "addr", dsignal->addr, OS_FALSE);
            devicedir_append_int_param(list, "n", dsignal->n, OS_FALSE);
            devicedir_append_str_param(list, "type",
                osal_typeid_to_name(dsignal->flags & OSAL_TYPEID_MASK), OS_FALSE);

            osal_stream_print_str(list, "}", 0);
        }
    }
}

#endif
