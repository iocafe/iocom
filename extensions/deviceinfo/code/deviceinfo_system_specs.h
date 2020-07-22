/**

  @file    deviceinfo_system_specs.h
  @brief   Publish software versions, used operating system and hardware.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.7.2020

  Set software version, operating system, architecture and hardware in "exp" memory block.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/** Enumeration of published system specification items.
 */
typedef enum dinfoSystemSpecSigEnum
{
    IOC_DINFO_SI_PACKAGE,
    IOC_DINFO_SI_EOSAL,
    IOC_DINFO_SI_IOCOM,
    IOC_DINFO_SI_OS,
    IOC_DINFO_SI_OSVER,
    IOC_DINFO_SI_ARCH,
    IOC_DINFO_SI_HW,

    IOC_DINFO_SI_NRO_SIGNALS
}
dinfoSystemSpecSigEnum;

/** Structure holding system specification signal pointers.
 */
typedef struct dinfoSystemSpeSignals
{
    const iocSignal
        *sig[IOC_DINFO_SI_NRO_SIGNALS];
}
dinfoSystemSpeSignals;

/** Macro for easy set up of default system specification signals.
 */
#define DINFO_SET_COMMON_SYSTEM_SPECS_SIGNALS(sigs, staticsigs)  \
    os_memclear(&sigs, sizeof(dinfoSystemSpecSigEnum)); \
    sigs.sig[IOC_DINFO_SI_PACKAGE] = &staticsigs.exp.si_package; \
    sigs.sig[IOC_DINFO_SI_EOSAL] = &staticsigs.exp.si_eosal; \
    sigs.sig[IOC_DINFO_SI_IOCOM] = &staticsigs.exp.si_iocom; \
    sigs.sig[IOC_DINFO_SI_OS] = &staticsigs.exp.si_os; \
    sigs.sig[IOC_DINFO_SI_OSVER] = &staticsigs.exp.si_osver; \
    sigs.sig[IOC_DINFO_SI_ARCH] = &staticsigs.exp.si_arch; \
    sigs.sig[IOC_DINFO_SI_HW] = &staticsigs.exp.si_hw;

/* Publish specification in memory block signals.
 */
void dinfo_set_system_specs(
    dinfoSystemSpeSignals *sigs,
    os_char *hw);
