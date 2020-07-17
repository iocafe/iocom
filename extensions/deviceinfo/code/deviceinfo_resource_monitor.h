/**

  @file    deviceinfo_resource_monitor.h
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

/** Enumeration of resource monitor items.
 */
typedef enum dinfoResMonSigEnum
{
    IOC_DINFO_RM_,

    IOC_DINFO_RM_NRO_SIGNALS
}
dinfoResMonSigEnum;

/** Structure holding resource monitor signal pointers.
 */
typedef struct dinfoResMonSignals
{
    const iocSignal
        *sig[IOC_DINFO_RM_NRO_SIGNALS];
}
dinfoResMonSignals;

/* Resource monitor state
 */
typedef struct dinfoResMonState
{
#if OSAL_RESOURCE_MONITOR
    dinfoResMonSignals sigs;
    os_timer update_timer;
#endif
}
dinfoResMonState;

#if OSAL_RESOURCE_MONITOR

/** Macro for easy set up of default system specification signals.
 */
#define DINFO_SET_COMMON_RESOURCE_MONITOR_SIGNALS(sigs, staticsigs)  \
    os_memclear(&sigs, sizeof(dinfoSystemSpecSigEnum)); \
    sigs.sig[IOC_DINFO_RM_] = &staticsigs.exp.si_package;

/* Initialize resource monitor state structure and store IO signal pointers.
 */
void dinfo_initialize_resource_monitor(
   dinfoResMonState *dinfo_rm,
   dinfoResMonSignals *sigs);

/* Move changes to resource monitor data to signals. Can be called from application main loop.
 */
void dinfo_run_resource_monitor(
    dinfoResMonState *dinfo_rm,
    os_timer *ti);

#else

#define DINFO_SET_COMMON_RESOURCE_MONITOR_SIGNALS(si, ss)
#define dinfo_initialize_resource_monitor(dinfo_rm,s)
#define dinfo_run_resource_monitor(dinfo_rm,ti)

#endif
