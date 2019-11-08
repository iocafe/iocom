/**

  @file    ioboard.h
  @brief   Header for basic static network IO.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.8.2018

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

/* Communication root structure.
 */
extern iocRoot
    ioboard_communication;

/* To and from controller memory blocks.
 */
extern iocMemoryBlock
    ioboard_import_mblk,
    ioboard_export_mblk;

extern iocHandle
    ioboard_import,
    ioboard_export;

/* Communication between controller and the IO board.
 */
#define IOBOARD_CTRL_IS_SOCKET 1
#define IOBOARD_CTRL_IS_SERVER 2
#define IOBOARD_CTRL_IS_TLS 4
#define IOBOARD_CTRL_BASIC_MASK (IOBOARD_CTRL_IS_SOCKET|IOBOARD_CTRL_IS_SERVER)
#define IOBOARD_CTRL_LISTEN_SOCKET (IOBOARD_CTRL_IS_SOCKET|IOBOARD_CTRL_IS_SERVER)
#define IOBOARD_CTRL_CONNECT_SOCKET IOBOARD_CTRL_IS_SOCKET
#define IOBOARD_CTRL_LISTEN_TLS (IOBOARD_CTRL_IS_TLS|IOBOARD_CTRL_IS_SERVER|IOBOARD_CTRL_IS_SOCKET)
#define IOBOARD_CTRL_CONNECT_TLS (IOBOARD_CTRL_IS_TLS|IOBOARD_CTRL_IS_SOCKET)
#define IOBOARD_CTRL_LISTEN_SERIAL IOBOARD_CTRL_IS_SERVER
#define IOBOARD_CTRL_CONNECT_SERIAL 0

/* If using static pool, the pool size must be calculated.If too small, program will not work.
   If too big, memory is wasted.The IOC_POOL_SIZE_LSOCK macro calculates pool size from number
   of connectionsand size of memory blocks for sendingand receiving data for an IO board listening
   to socket port.Memory needed for the iocMemoryBlock stucture for receivedand sent data is not
   included, neither is memory for end point structure iocEndPoint(if listening for connections).
 */
#define IOBOARD_POOL_SIZE(CTRL_TYPE, MAX_CONNECTIONS, SEND_BLOCK_SZ, RECEIVE_BLOCK_SZ) \
    MAX_CONNECTIONS * sizeof(iocConnection) \
  + MAX_CONNECTIONS * 2 * ((CTRL_TYPE & IOBOARD_CTRL_IS_SOCKET) ? IOC_SOCKET_FRAME_SZ : IOC_SERIAL_FRAME_SZ) \
  + MAX_CONNECTIONS * sizeof(iocSourceBuffer) \
  + MAX_CONNECTIONS * SEND_BLOCK_SZ * sizeof(ioc_sbuf_item) \
  + MAX_CONNECTIONS * sizeof(iocTargetBuffer) \
  + MAX_CONNECTIONS * RECEIVE_BLOCK_SZ * sizeof(ioc_tbuf_item) \
  + (((CTRL_TYPE & IOBOARD_CTRL_BASIC_MASK) == IOBOARD_CTRL_LISTEN_SOCKET) ? sizeof(iocEndPoint) : 0) \
  + SEND_BLOCK_SZ \
  + RECEIVE_BLOCK_SZ
  // DO WE NEED MAX_CONNECTIONS elements for both send and receive?

/* Macro to calculate how much additional memory pool we need to publish static device information.
 */
#define IOBOARD_POOL_DEVICE_INFO(MAX_CONNECTIONS) \
    sizeof(iocMemoryBlock) + IOBOARD_MAX_CONNECTIONS * sizeof(iocSourceBuffer)

/* Macro to get interface by other defines.
 */
#if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_SOCKET
  #if IOBOARD_CTRL_CON & IOBOARD_CTRL_IS_TLS
    #define IOBOARD_IFACE OSAL_TLS_IFACE
  #else
    #define IOBOARD_IFACE OSAL_SOCKET_IFACE
  #endif
#else
    #define IOBOARD_IFACE OSAL_SERIAL_IFACE
#endif

struct iocDeviceHdr;

/* IO board parameter structure.
 */
typedef struct
{
    /** Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE or OSAL_TLS_IFACE defines.
     */
    const osalStreamInterface *iface;

    /** Device name, max 15 characters from 'a' - 'z' or 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL". 
     */
    const os_char *device_name;

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in 
        context as device name, like "TEMPCTRL1".
     */
    os_short device_nr;

	/** Control computer connection type: IOBOARD_CTRL_LISTEN_SOCKET, 
        IOBOARD_CTRL_CONNECT_SOCKET, IOBOARD_CTRL_LISTEN_SERIAL,
        IOBOARD_CTRL_CONNECT_SERIAL.
        Bit fields. IOBOARD_CTRL_IS_SOCKET, IOBOARD_CTRL_IS_SERVER.
     */
    os_int ctrl_type;

	/** If IO board connects to control computer, IOBOARD_CTRL_CONNECT_SOCKET option:
	    IP address and port as a string. For example "192.168.1.229:8369".
	    If unused can be OS_NULL.
	 */
    const os_char *socket_con_str;

	/** If control computer and IO board communicate using serial communication,
	    serial port and settings for it as a string. If unused can be OS_NULL.
	 */
    const os_char *serial_con_str;

    os_int max_connections;
    os_int send_block_sz;
    os_int receive_block_sz;

    os_char *pool;
    os_int pool_sz;

    /** Pointer to static device information.
     */
    const os_char *device_info;

    /** Size of static device information in bytes.
     */
    os_int device_info_sz;

    /** Enable IOC_AUTO_SYNC
     */
    os_boolean auto_synchronization;

    /** Pointer to static structure defining signals.
     */
    const struct iocDeviceHdr *device_signal_hdr;
}
ioboardParams;

/* Initialize the communication for an IO board.
 */
void ioboard_start_communication(ioboardParams *prm);

/* Shut down the communication.
 */
void ioboard_end_communication(void);
