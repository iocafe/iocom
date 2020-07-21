/**

  @file    deviceinfo_resource_monitor.c
  @brief   Publish resource and performance counters.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Publish main loop timing and resource counters collected by EOSAL as IO signals.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "deviceinfo.h"
#if OSAL_RESOURCE_MONITOR

#define OSAL_RESOURCE_MONITOR_PERIOD 1200

typedef struct dinfoResMonMapItem
{
    os_char dst, src;
}
dinfoResMonMapItem;

static OS_FLASH_MEM dinfoResMonMapItem dinfo_rm_map[] =
{
     {IOC_DINFO_RM_MALLOC, OSAL_RMON_SYSTEM_MEMORY_ALLOCATION}
    ,{IOC_DINFO_RM_MUSE, OSAL_RMON_SYSTEM_MEMORY_USE}
#if OSAL_MULTITHREAD_SUPPORT
    ,{IOC_DINFO_RM_THREADS ,OSAL_RMON_THREAD_COUNT}
    ,{IOC_DINFO_RM_EVENTS, OSAL_RMON_EVENT_COUNT}
    ,{IOC_DINFO_RM_MUTEXES, OSAL_RMON_MUTEX_COUNT}
#endif
#if OSAL_FILESYS_SUPPORT
    ,{IOC_DINFO_RM_FILE_HANDLES, OSAL_RMON_FILE_HANDLE_COUNT}
#endif
#if OSAL_SOCKET_SUPPORT
    ,{IOC_DINFO_RM_SOCKETS, OSAL_RMON_SOCKET_COUNT}
    ,{IOC_DINFO_RM_CONNECTS, OSAL_RMON_SOCKET_CONNECT_COUNT}
    ,{IOC_DINFO_RM_TX_TCP, OSAL_RMON_TX_TCP}
    ,{IOC_DINFO_RM_RX_TCP, OSAL_RMON_RX_TCP}
    ,{IOC_DINFO_RM_TX_UDP, OSAL_RMON_TX_UDP}
    ,{IOC_DINFO_RM_RX_UDP, OSAL_RMON_RX_UDP}
#endif
#if OSAL_SERIAL_SUPPORT
    ,{IOC_DINFO_RM_TX_SERIAL, OSAL_RMON_TX_SERIAL}
    ,{IOC_DINFO_RM_RX_SERIAL, OSAL_RMON_RX_SERIAL}
#endif
};

#define DINFO_RM_MAP_LEN (sizeof( dinfo_rm_map)/sizeof(dinfoResMonMapItem))


/**
****************************************************************************************************

  @brief Initialize resource monitor state structure and store IO signal pointers.

  Called from initialization code during program startup.

  @param   dinfo_rm Pointer to published resource monitor state structure.
           This pointer is used as "handle".
  @param   sigs Structure containing signal pointers to set. Macros like
           DINFO_SET_COMMON_RESOURCE_MONITOR_SIGNALS can be used to initialize typical
           signal pointers.
  @return  None

****************************************************************************************************
*/
void dinfo_initialize_resource_monitor(
   dinfoResMonState *dinfo_rm,
   dinfoResMonSignals *sigs)
{
    os_memclear(dinfo_rm, sizeof(dinfoResMonState));
    os_memcpy(&dinfo_rm->sigs, sigs, sizeof(dinfoResMonSignals));
}


/**
****************************************************************************************************

  @brief Check if we need to save or reboot
  @anchor dinfo_run_node_conf

  The dinfo_run_resource_monitor() function is called repeatedly to move changes to resource
  monitor data to signals. This must be be called from application main loop, on every loop
  since the function handles main loop timing.

  @param   dinfo_rm Pointer to published resource monitor state structure.
           This pointer is used as "handle".
  @param   ti Current timer value, If OS_NULL timer is read by function call.
  @return  None.

****************************************************************************************************
*/
void dinfo_run_resource_monitor(
    dinfoResMonState *dinfo_rm,
    os_timer *ti)
{
    osalResourceMonitorState *rs;
    const iocSignal *sig;
    os_timer tmp_ti;
    os_int elapsed_ms, loop_ms, i, si, di;
    os_int loop_period_100us, maxloop_ms, minutes_since_boot;

    if (ti == OS_NULL) {
        os_get_timer(&tmp_ti);
        ti = &tmp_ti;
    }

    if (!dinfo_rm->initialized)
    {
        dinfo_rm->boot_timer = *ti;
        dinfo_rm->loop_timer = *ti;
        dinfo_rm->update_timer = *ti;
        dinfo_rm->initialized = OS_TRUE;
        return;
    }

    loop_ms = (os_int)os_get_ms_elapsed(&dinfo_rm->loop_timer, ti);
    if (loop_ms > dinfo_rm->maxloop_ms) {
        dinfo_rm->maxloop_ms = loop_ms;
    }
    dinfo_rm->loop_timer = *ti;
    dinfo_rm->loop_count++;

    if (!os_has_elapsed_since(&dinfo_rm->update_timer, ti, OSAL_RESOURCE_MONITOR_PERIOD)) {
        return;
    }
    elapsed_ms = (os_int)os_get_ms_elapsed(&dinfo_rm->update_timer, ti);
    dinfo_rm->update_timer = *ti;

    rs = &osal_global->resstate;
    if (rs->updated)
    {
        rs->updated = OS_FALSE;

        for (i = 0; i < (os_int)DINFO_RM_MAP_LEN; i++) {
            si = dinfo_rm_map[i].src;

            if (rs->changed[si]) {
                di = dinfo_rm_map[i].dst;
                sig = dinfo_rm->sigs.sig[di];
                if (sig) ioc_set(sig, rs->current[si]);
                rs->changed[si] = OS_FALSE;
            }
        }
    }

    if (dinfo_rm->loop_count > 0) {
        loop_period_100us = (os_int)(10 * (os_long)elapsed_ms / dinfo_rm->loop_count);
    }
    else {
        loop_period_100us = -1;
    }

    sig = dinfo_rm->sigs.sig[IOC_DINFO_RM_AVELOOP];
    if (sig && loop_period_100us != dinfo_rm->prev_loop_period_100us)
    {
        ioc_set_double(sig, 0.1 * loop_period_100us);
        dinfo_rm->prev_loop_period_100us = loop_period_100us;
    }

    maxloop_ms = dinfo_rm->maxloop_ms;
    sig = dinfo_rm->sigs.sig[IOC_DINFO_RM_MAXLOOP];
    if (sig && maxloop_ms != dinfo_rm->prev_maxloop_ms)
    {
        ioc_set(sig, maxloop_ms);
        dinfo_rm->prev_maxloop_ms = maxloop_ms;
    }

    elapsed_ms = (os_int)os_get_ms_elapsed(&dinfo_rm->boot_timer, ti);
    minutes_since_boot = elapsed_ms / (60 * 1000);
    sig = dinfo_rm->sigs.sig[IOC_DINFO_RM_BOOTTIME];
    if (sig && minutes_since_boot != dinfo_rm->minutes_since_boot)
    {
        ioc_set(sig, minutes_since_boot);
        dinfo_rm->minutes_since_boot = minutes_since_boot;
    }

    dinfo_rm->loop_count = 0;
    dinfo_rm->maxloop_ms = 0;
}

#endif
