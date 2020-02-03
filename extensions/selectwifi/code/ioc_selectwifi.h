/**

  @file    ioc_selectwifi.h
  @brief   Set wifi network name and password over blue tooth or serial port.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    3.2.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


// struct iocBServerNetwork;

/**
****************************************************************************************************
  Library initialization parameter structure
****************************************************************************************************
 */
typedef struct iocSelectWiFiParams
{
    int uke;

}
iocSelectWiFiParams;


/**
****************************************************************************************************
  Static global select wifi state structure
****************************************************************************************************
 */
typedef struct iocSelectWiFi
{
    /* Pointer to IOCOM root structure.
     */
    iocRoot root;

    /* Memory block handles for the server.
     */
    iocHandle exp, imp, info;

    /* Memory block structures.
     */
    iocMemoryBlock exp_mblk, imp_mblk, info_mblk;

    // os_timer sec_timer;
}
iocSelectWiFi;

/* Global static select wifi state
 */
extern iocSelectWiFi swf;


/**
****************************************************************************************************
  Include memory block info and signal code header. These must be after "extern iocSelectWiFi swf;"
****************************************************************************************************
 */
#ifdef SELECTWIFI_INTERNALS
#include "swf-info-mblk.h"
#include "swf-signals.h"
#endif


/**
****************************************************************************************************
  Functions
****************************************************************************************************
 */
/* Initialize the select wifi library.
 */
void ioc_initialize_selectwifi(
    iocSelectWiFiParams *prm);

/* Release resources allocated for select wifi library.
 */
void ioc_release_selectwifi(
    void);

/* Keep wifi selection functionality alive.
 */
osalStatus ioc_run_selectwifi(
    void);

