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
    IOC_DINFO_RM_MALLOC,
    IOC_DINFO_RM_MUSE,
    IOC_DINFO_RM_THREADS,
    IOC_DINFO_RM_EVENTS,
    IOC_DINFO_RM_MUTEXES,
    IOC_DINFO_RM_SOCKETS,
    IOC_DINFO_RM_CONNECTS,
    IOC_DINFO_RM_TX_TCP,
    IOC_DINFO_RM_RX_TCP,
    IOC_DINFO_RM_TX_UDP,
    IOC_DINFO_RM_RX_UDP,
    IOC_DINFO_RM_TX_SERIAL,
    IOC_DINFO_RM_RX_SERIAL,
    IOC_DINFO_RM_AVELOOP,
    IOC_DINFO_RM_MAXLOOP,
    IOC_DINFO_RM_BOOTTIME,

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
    os_timer boot_timer;
    os_timer update_timer;

    os_timer loop_timer;
    os_int maxloop_ms;
    os_int loop_count;
    os_int prev_loop_period_100us;
    os_int prev_maxloop_ms;
    os_int minutes_since_boot;
    os_boolean initialized;
#endif
}
dinfoResMonState;

#if OSAL_RESOURCE_MONITOR

/** Macro for easy set up of default system specification signals.
 */
#define DINFO_SET_COMMON_RESOURCE_MONITOR_SIGNALS(sigs, staticsigs)  \
    os_memclear(&sigs, sizeof(dinfoSystemSpecSigEnum)); \
    sigs.sig[IOC_DINFO_RM_MALLOC] = &staticsigs.exp.rm_malloc; \
    sigs.sig[IOC_DINFO_RM_MUSE] = &staticsigs.exp.rm_muse; \
    sigs.sig[IOC_DINFO_RM_THREADS] = &staticsigs.exp.rm_threads; \
    sigs.sig[IOC_DINFO_RM_EVENTS] = &staticsigs.exp.rm_events; \
    sigs.sig[IOC_DINFO_RM_MUTEXES] = &staticsigs.exp.rm_mutexes; \
    sigs.sig[IOC_DINFO_RM_SOCKETS] = &staticsigs.exp.rm_sockets; \
    sigs.sig[IOC_DINFO_RM_CONNECTS] = &staticsigs.exp.rm_connects; \
    sigs.sig[IOC_DINFO_RM_TX_TCP] = &staticsigs.exp.rm_tx_tcp; \
    sigs.sig[IOC_DINFO_RM_RX_TCP] = &staticsigs.exp.rm_rx_tcp; \
    sigs.sig[IOC_DINFO_RM_TX_UDP] = &staticsigs.exp.rm_tx_udp; \
    sigs.sig[IOC_DINFO_RM_RX_UDP] = &staticsigs.exp.rm_rx_udp; \
    sigs.sig[IOC_DINFO_RM_AVELOOP] = &staticsigs.exp.rm_aveloop; \
    sigs.sig[IOC_DINFO_RM_MAXLOOP] = &staticsigs.exp.rm_maxloop; \
    sigs.sig[IOC_DINFO_RM_BOOTTIME] = &staticsigs.exp.rm_boottime;


/* Initialize resource monitor state structure and store IO signal pointers.
 */
void dinfo_initialize_resource_monitor(
   dinfoResMonState *dinfo_rm,
   dinfoResMonSignals *sigs);

/* Move changes to resource monitor data to signals. Must be called from application main loop.
 */
void dinfo_run_resource_monitor(
    dinfoResMonState *dinfo_rm,
    os_timer *ti);

#else

#define DINFO_SET_COMMON_RESOURCE_MONITOR_SIGNALS(si, ss)
#define dinfo_initialize_resource_monitor(dinfo_rm,s)
#define dinfo_run_resource_monitor(dinfo_rm,ti)

#endif
