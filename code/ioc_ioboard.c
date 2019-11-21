/**

  @file    ioc_ioboard.c
  @brief   Basic static network IO.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.8.2018

  The ioboard_start_communication() should be called at entry to IO board's program and
  if clean up is needed ioboard_end_communication() at exit.

  Memory blocks initialized are ioboard_export (tc = to controller) and
  ioboard_import (fc = from controller).

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#define IOCOM_IOBOARD
#include "iocom.h"

iocRoot
    ioboard_communication;

iocMemoryBlock
    ioboard_import_mblk,
    ioboard_export_mblk;

iocHandle
    ioboard_import,
    ioboard_export,
    ioboard_dinfo;

static iocEndPoint
    *ioboard_epoint;

static iocConnection
    *ioboard_connection;

/**
****************************************************************************************************

  @brief Initialize IO board communication.

  The ioboard_start_communication() function sets up an IO board with connects two memory blocks,
  "to controller" and "from controller". 

  @return  None.

****************************************************************************************************
*/
void ioboard_start_communication(
    ioboardParams *prm)
{
    ioboardParams defaultprm;
    iocMemoryBlockParams blockprm;
    iocConnectionParams conprm;
    iocEndPointParams epprm;

    if (prm == OS_NULL)
    {
        prm = &defaultprm;
    }

    ioc_initialize_root(&ioboard_communication);
    ioboard_communication.device_signal_hdr = prm->device_signal_hdr;


    if (prm->pool)
    {
        ioc_set_memory_pool(&ioboard_communication, prm->pool, prm->pool_sz);
    }

    os_memclear(&blockprm, sizeof(blockprm));
    blockprm.device_name = prm->device_name;
    blockprm.device_nr = prm->device_nr;
    blockprm.network_name = prm->network_name;

    blockprm.mblk_name = "exp";
    blockprm.nbytes = prm->send_block_sz ? prm->send_block_sz : 256;
    blockprm.flags = prm->auto_synchronization ? (IOC_SOURCE|IOC_AUTO_SYNC) : IOC_SOURCE;
    ioc_initialize_memory_block(&ioboard_export, &ioboard_export_mblk, &ioboard_communication, &blockprm);
 
    blockprm.mblk_name = "imp";
    blockprm.nbytes = prm->receive_block_sz ? prm->receive_block_sz : 256;
    blockprm.flags = prm->auto_synchronization ? (IOC_TARGET|IOC_AUTO_SYNC) : IOC_TARGET;
    ioc_initialize_memory_block(&ioboard_import, &ioboard_import_mblk, &ioboard_communication, &blockprm);

    /* Do we publish device information?
     */
    if (prm->device_info)
    {
        blockprm.mblk_name = "info";
        blockprm.buf = (char*)prm->device_info;
        blockprm.nbytes = prm->device_info_sz;
        blockprm.flags = IOC_SOURCE|IOC_STATIC;
        ioc_initialize_memory_block(&ioboard_dinfo, OS_NULL, &ioboard_communication, &blockprm);
    }

	/* Control computer connection type: IOBOARD_CTRL_LISTEN_SOCKET,
       IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_CONNECT_SERIAL,
       IOBOARD_CTRL_LISTEN_SERIAL.
	 */
    os_memclear(&conprm, sizeof(conprm));
    conprm.iface = prm->iface;
    switch (prm->ctrl_type & IOBOARD_CTRL_BASIC_MASK)
	{
		default:
		case IOBOARD_CTRL_LISTEN_SOCKET:
			ioboard_epoint = ioc_initialize_end_point(OS_NULL, &ioboard_communication);

            os_memclear(&epprm, sizeof(epprm));
            epprm.iface = prm->iface;
            epprm.flags = IOC_SOCKET;
			ioc_listen(ioboard_epoint, &epprm);
			return;

		case IOBOARD_CTRL_CONNECT_SOCKET:
            conprm.parameters = prm->socket_con_str;
            conprm.flags = IOC_SOCKET | IOC_DISABLE_SELECT;
			break;

        case IOBOARD_CTRL_CONNECT_SERIAL:
            conprm.parameters = prm->serial_con_str;
            conprm.flags = IOC_SERIAL | IOC_DISABLE_SELECT;
            break;

        case IOBOARD_CTRL_LISTEN_SERIAL:
            conprm.parameters = prm->serial_con_str;
            conprm.flags = IOC_LISTENER | IOC_SERIAL | IOC_DISABLE_SELECT;
            break;
	}

    ioboard_connection = ioc_initialize_connection(OS_NULL, &ioboard_communication);
    ioc_connect(ioboard_connection, &conprm);
}


/**
****************************************************************************************************

  @brief Shut down IO board communication.

  The ioboard_end_communication() function stops communication and releases all resources.

  @return  None.

****************************************************************************************************
*/
void ioboard_end_communication(void)
{
    ioc_release_root(&ioboard_communication);
}
