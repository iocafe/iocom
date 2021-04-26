/**

  @file    ioc_ioboard.c
  @brief   Basic static network IO.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  The ioboard_start_communication() should be called at entry to IO board's program and
  if clean up is needed ioboard_end_communication() at exit.

  Memory blocks initialized are ioboard_exp (tc = to controller) and
  ioboard_imp (fc = from controller).

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#define IOCOM_IOBOARD
#include "iocom.h"

iocRoot
    ioboard_root;

iocMemoryBlock
    ioboard_import_mblk,
    ioboard_export_mblk;

iocHandle
    ioboard_imp,
    ioboard_exp,
    ioboard_dinfo;

#if IOC_STREAMER_SUPPORT
iocHandle
    ioboard_conf_imp,
    ioboard_conf_exp,
    ioboard_dimp,
    ioboard_dexp;
#endif

#if OSAL_SOCKET_SUPPORT
static iocEndPoint
    *ioboard_epoint;
#endif

static iocConnection
    *ioboard_connection;


/**
****************************************************************************************************

  @brief Initialize IOCOM and set up memory blocks for the ioboard.

  The ioboard_setup_communication() function initializes IOCOM and sets up memory blocks for
  basic IO board use. These memory blocks are "exp", "imp", "conf_exp", "conf_imp" and "info".

  @param   prm IO board communication parameters.
  @return  None.

****************************************************************************************************
*/
void ioboard_setup_communication(
    ioboardParams *prm)
{
    iocMemoryBlockParams blockprm;

#if IOC_DEVICE_STREAMER
    ioc_streamer_initialize();
#endif
    ioc_initialize_root(&ioboard_root, IOC_CREATE_OWN_MUTEX);
    ioc_set_iodevice_id(&ioboard_root, prm->device_name, prm->device_nr,
        prm->password, prm->network_name);

    if (prm->pool_sz)
    {
        ioc_set_memory_pool(&ioboard_root, prm->pool, prm->pool_sz);
    }

    os_memclear(&blockprm, sizeof(blockprm));
#if IOC_MBLK_SPECIFIC_DEVICE_NAME
    blockprm.device_name = prm->device_name;
    blockprm.device_nr = prm->device_nr;
    blockprm.network_name = prm->network_name;
#endif

    blockprm.mblk_name = "exp";
    blockprm.nbytes = prm->exp_mblk_sz;
    blockprm.flags = IOC_MBLK_UP;
    ioc_initialize_memory_block(&ioboard_exp, &ioboard_export_mblk, &ioboard_root, &blockprm);
#if IOC_SIGNAL_RANGE_SUPPORT
    ioboard_export_mblk.signal_hdr = prm->exp_signal_hdr;
#endif
    blockprm.mblk_name = "imp";
    blockprm.nbytes = prm->imp_mblk_sz;
    blockprm.flags = IOC_MBLK_DOWN;
    ioc_initialize_memory_block(&ioboard_imp, &ioboard_import_mblk, &ioboard_root, &blockprm);
#if IOC_SIGNAL_RANGE_SUPPORT
    ioboard_import_mblk.signal_hdr = prm->imp_signal_hdr;
#endif

#if IOC_STREAMER_SUPPORT
    if (prm->dexp_mblk_sz) {
        blockprm.mblk_name = "dexp";
        blockprm.nbytes = prm->dexp_mblk_sz;
        blockprm.flags = IOC_MBLK_UP;
        ioc_initialize_memory_block(&ioboard_dexp, OS_NULL, &ioboard_root, &blockprm);
#if IOC_SIGNAL_RANGE_SUPPORT
        ioboard_dexp.mblk->signal_hdr = prm->dexp_signal_hdr;
#endif
    }

    if (prm->dimp_mblk_sz) {
        blockprm.mblk_name = "dimp";
        blockprm.nbytes = prm->dimp_mblk_sz;
        blockprm.flags = IOC_MBLK_DOWN;
        ioc_initialize_memory_block(&ioboard_dimp, OS_NULL, &ioboard_root, &blockprm);
#if IOC_SIGNAL_RANGE_SUPPORT
        ioboard_dimp.mblk->signal_hdr = prm->dimp_signal_hdr;
#endif
    }

    if (prm->conf_exp_mblk_sz)
    {
        blockprm.mblk_name = "conf_exp";
        blockprm.nbytes = prm->conf_exp_mblk_sz;
        blockprm.flags = IOC_MBLK_UP;
        ioc_initialize_memory_block(&ioboard_conf_exp, OS_NULL, &ioboard_root, &blockprm);
#if IOC_SIGNAL_RANGE_SUPPORT
        ioboard_conf_exp.mblk->signal_hdr = prm->conf_exp_signal_hdr;
#endif
    }

    if (prm->conf_imp_mblk_sz)
    {
        blockprm.mblk_name = "conf_imp";
        blockprm.nbytes = prm->conf_imp_mblk_sz;
        blockprm.flags = IOC_MBLK_DOWN;
        ioc_initialize_memory_block(&ioboard_conf_imp, OS_NULL, &ioboard_root, &blockprm);
#if IOC_SIGNAL_RANGE_SUPPORT
        ioboard_conf_imp.mblk->signal_hdr = prm->conf_imp_signal_hdr;
#endif
    }
#endif

    /* Do we publish device information?
     */
    if (prm->device_info)
    {
        blockprm.mblk_name = "info";
        blockprm.buf = (char*)prm->device_info;
        blockprm.nbytes = prm->device_info_sz;
        blockprm.flags = IOC_MBLK_UP|IOC_STATIC;
        ioc_initialize_memory_block(&ioboard_dinfo, OS_NULL, &ioboard_root, &blockprm);
    }

    prm->mblk_setup_called = OS_TRUE;
}


/**
****************************************************************************************************

  @brief Start communicating.

  The ioboard_start_communication() connects to controller or starts listening for incoming
  connections.

  The function calls ioboard_setup_communication() if it has not been called earlier.

  @param   prm IO board communication parameters.
  @return  None.

****************************************************************************************************
*/
void ioboard_start_communication(
    ioboardParams *prm)
{
    iocConnectionParams conprm;

    if (!prm->mblk_setup_called) {
        ioboard_setup_communication(prm);
    }

#if OSAL_MULTITHREAD_SUPPORT
  // #define IOC_CT_FLAG IOC_CREATE_THREAD  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  #define IOC_CT_FLAG 0
#else
  #define IOC_CT_FLAG 0
#endif

    /* Control computer connection type: IOBOARD_CTRL_LISTEN_SOCKET,
       IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_CONNECT_SERIAL,
       IOBOARD_CTRL_LISTEN_SERIAL.
     */
    os_memclear(&conprm, sizeof(conprm));
    conprm.iface = prm->iface;
    switch (prm->ctrl_type & IOBOARD_CTRL_BASIC_MASK)
    {
        default:
#if OSAL_SOCKET_SUPPORT
        case IOBOARD_CTRL_LISTEN_SOCKET:
            ioboard_epoint = ioc_initialize_end_point(OS_NULL, &ioboard_root);

            iocEndPointParams epprm;
            os_memclear(&epprm, sizeof(epprm));
            epprm.iface = prm->iface;
            epprm.flags = IOC_SOCKET | IOC_CONNECT_UP | IOC_CT_FLAG;
            ioc_listen(ioboard_epoint, &epprm);
            return;

        case IOBOARD_CTRL_CONNECT_SOCKET:
            conprm.parameters = prm->socket_con_str;
            conprm.flags = IOC_SOCKET | IOC_DISABLE_SELECT | IOC_CONNECT_UP | IOC_CT_FLAG;
            break;
#endif

#if OSAL_SERIAL_SUPPORT
        case IOBOARD_CTRL_CONNECT_SERIAL:
            conprm.parameters = prm->serial_con_str;
            conprm.flags = IOC_SERIAL | IOC_DISABLE_SELECT | IOC_CONNECT_UP | IOC_CT_FLAG;
            break;

        case IOBOARD_CTRL_LISTEN_SERIAL:
            conprm.parameters = prm->serial_con_str;
            conprm.flags = IOC_LISTENER | IOC_SERIAL
                | IOC_DISABLE_SELECT | IOC_CONNECT_UP | IOC_CT_FLAG;
            break;
#endif
    }
#if OSAL_SOCKET_SUPPORT
    conprm.lighthouse_func = prm->lighthouse_func;
    conprm.lighthouse = prm->lighthouse;
#endif

    ioboard_connection = ioc_initialize_connection(OS_NULL, &ioboard_root);
    ioc_connect(ioboard_connection, &conprm);
}


#if OSAL_PROCESS_CLEANUP_SUPPORT
/**
****************************************************************************************************

  @brief Shut down IO board communication.

  The ioboard_end_communication() function stops communication and releases allocated resources.

  @return  None.

****************************************************************************************************
*/
void ioboard_end_communication(void)
{
    ioc_release_root(&ioboard_root);
}
#endif
