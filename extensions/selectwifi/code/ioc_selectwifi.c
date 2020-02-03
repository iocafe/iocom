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
#define SELECTWIFI_INTERNALS
#include "selectwifi.h"

/* We may want to run connection in separate thread, if multithreading is supported?
 */
#if OSAL_MULTITHREAD_SUPPORT
    #define SWF_CREATE_THREAD 1
#else
    #define SWF_CREATE_THREAD 0
#endif

/* Static global signal structure for select wifi.
 */
iocSelectWiFi swf;
static os_char swf_pool[10000]; // CALCULATE POOL SIZE NEEDED ????????????????????????????????????????????????


/* Prototypes for forward referred static functions.
 */


/**
****************************************************************************************************

  @brief Initialize selecy wifi library.

  The ioc_initialize_selectwifi() function sets up the wifi select functionality.

  @param   swf Pointer to wifi network select object to be initialized.
  @param   prm Flat parameter structure.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_selectwifi(
    iocSelectWiFiParams *prm)
{
    const os_char device_name[] = "wifi";
    os_int device_nr = IOC_AUTO_DEVICE_NR;
    const os_char network_name[] = "iocafenet";

    iocMemoryBlockParams blockprm;
    os_memclear(&swf, sizeof(iocSelectWiFi));

    ioc_initialize_root(&swf.root);
    ioc_set_memory_pool(&swf.root, swf_pool, sizeof(swf_pool));

    /* Generate memory blocks.
     */
    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = device_name;
    blockprm.device_nr = device_nr;
    blockprm.network_name = network_name;

    blockprm.mblk_name = selectwifi.exp.hdr.mblk_name;
    blockprm.nbytes = SELECTWIFI_EXP_MBLK_SZ;
    blockprm.flags = IOC_MBLK_UP|IOC_AUTO_SYNC|IOC_FLOOR;
    ioc_initialize_memory_block(&swf.exp, &swf.exp_mblk, &swf.root, &blockprm);

    blockprm.mblk_name = selectwifi.imp.hdr.mblk_name;
    blockprm.nbytes = SELECTWIFI_IMP_MBLK_SZ;
    blockprm.flags = IOC_MBLK_DOWN|IOC_AUTO_SYNC|IOC_FLOOR;
    ioc_initialize_memory_block(&swf.imp, &swf.imp_mblk, &swf.root, &blockprm);

    blockprm.mblk_name = "info";
    blockprm.buf = (char*)&ioapp_signal_config;
    blockprm.nbytes = sizeof(ioapp_signal_config);
    blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
    ioc_initialize_memory_block(&swf.info, &swf.info_mblk, &swf.root, &blockprm);

    iocEndPoint *epoint = ioc_initialize_end_point(OS_NULL, &swf.root);
    iocEndPointParams epprm;
    os_memclear(&epprm, sizeof(epprm));
    epprm.iface = OSAL_SOCKET_IFACE;
#if SWF_CREATE_THREAD
    epprm.flags = IOC_SOCKET | IOC_CONNECT_UP | IOC_CREATE_THREAD ;
#else
    epprm.flags = IOC_SOCKET | IOC_CONNECT_UP;
#endif
    ioc_listen(epoint, &epprm);
}


/**
****************************************************************************************************

  @brief Release resources allocated for select wifi library.

  The ioc_release_selectwifi() function releases memory and other resources allocated for
  wifi network select.

  @param   swf Pointer to wifi network select object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_selectwifi(
    void)
{
    ioc_release_memory_block(&swf.exp);
    ioc_release_memory_block(&swf.imp);
    ioc_release_memory_block(&swf.info);
}


/**
****************************************************************************************************

  @brief Keep wifi selection functionality alive.

  The ioc_run_selectwifi() function needs to be called repeatedly to keep the
  functionality responsive.

  @param   swf Pointer to wifi network select object.
  @return  If working in something, the function returns OSAL_SUCCESS. Return value
           OSAL_STATUS_NOTHING_TO_DO indicates that this thread can be switched to slow
           idle mode as far as the bserver knows.

****************************************************************************************************
*/
osalStatus ioc_run_selectwifi(
    void)
{
    return OSAL_SUCCESS;
}
