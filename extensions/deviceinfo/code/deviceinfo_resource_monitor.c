/**

  @file    deviceinfo_resource_monitor.c
  @brief   Publish resource and performance counters.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "deviceinfo.h"
#if OSAL_RESOURCE_MONITOR

#define OSAL_RESOURCE_MONITOR_PERIOD 1200


/**
****************************************************************************************************

  @brief Initialize resource monitor state structure and store IO signal pointers.

  X

  This mu

  @param   X
  @return  X

****************************************************************************************************
*/
void dinfo_initialize_resource_monitor(
   dinfoResMonState *dinfo_rm,
   dinfoResMonSignals *sigs)
{
    os_memclear(dinfo_rm, sizeof(dinfoResMonState));
    os_memcpy(&dinfo_rm->sigs, sigs, sizeof(dinfoResMonSignals));
    os_get_timer(&dinfo_rm->update_timer);
}

/* Move changes to resource monitor data to signals. Must be called from application main loop.
 */
void dinfo_run_resource_monitor(
    dinfoResMonState *dinfo_rm,
    os_timer *ti)
{
    osalResourceMonitorState *rs;
    os_timer tmp_ti;
    os_int elapsed_ms;

    if (ti == OS_NULL) {
        os_get_timer(&tmp_ti);
        ti = &tmp_ti;
    }

    if (!os_has_elapsed_since(&dinfo_rm->update_timer, ti, OSAL_RESOURCE_MONITOR_PERIOD)) {
        return;
    }
    elapsed_ms = os_get_ms_elapsed(&dinfo_rm->update_timer, ti);
    dinfo_rm->update_timer = *ti;
    rs = &osal_global->resstate;


}

#endif
