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
typedef enum
{
    IOC_SWF_BLUE_TOOTH,
    IOC_SWF_SERIAL_PORT,
    IOC_SWF_SOCKET_TEST
}
iocSelectWiFiTransport;


typedef struct iocSelectWiFiParams
{
    /** Transport, one of IOC_SWF_BLUE_TOOTH, IOC_SWF_SERIAL_PORT, or IOC_SWF_SOCKET_TEST.
     */
    iocSelectWiFiTransport transport;

    /** Parameter string for the transport.
     */
    const os_char *parameters;
}
iocSelectWiFiParams;


/**
****************************************************************************************************
  Static global select wifi state structure
****************************************************************************************************
 */
typedef struct iocSelectWiFi
{
    /** Pointer to IOCOM root structure.
     */
    iocRoot root;

    /** Memory block handles for the server.
     */
    iocHandle exp, imp, info;

    /** Memory block structures.
     */
    iocMemoryBlock exp_mblk, imp_mblk, info_mblk;

    /** Transport, one of IOC_SWF_BLUE_TOOTH, IOC_SWF_SERIAL_PORT, or IOC_SWF_SOCKET_TEST.
     */
    iocSelectWiFiTransport transport;

    /** End point when testing with socket.
     */
    iocEndPoint *epoint;

    /** Blue tooth or serial connection.
     */
    iocConnection *con;

    /** Time when we rebooted. We do not allow reboot request for 5 seconds after boot,
        just in case there is old one hanging on.
     */
    os_timer boot_timer;
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
#include "config/include/swf-info-mblk.h"
#include "config/include/swf-signals.h"
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

