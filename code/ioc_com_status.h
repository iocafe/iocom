/**

  @file    ioc_com_status.h
  @brief   Communication status.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    29.5.2019

  Communication status refers to general communication information and settings. For example
  number of connections (sockets, etc) connected to a memory block. In future this could
  indicate which input data is selected in redundant communication, etc. Communication status
  may include also settings.

  From application's view communication status appears the same as data memory and is accessed
  using the same ioc_read(), ioc_get16(), ioc_write(), ioc_set16(), etc. functions. For data
  memory, the address is positive or zero, status memory addresses are negative.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_COM_STATUS_INCLUDED
#define IOC_COM_STATUS_INCLUDED


/**
****************************************************************************************************
    Used status memory addresses.
****************************************************************************************************
*/
typedef enum
{
    /** Number of connected streams at this moment, two bytes (addresses -2 and -1).
     */
    IOC_NRO_CONNECTED_STREAMS = -2,

    /** How many times socket connection has been closed.
     */
    IOC_CONNECTION_DROP_COUNT = -6,

    /** Size of status memory in bytes.
     */
    IOC_STATUS_MEMORY_SZ = 6
}
iocStatusMemoryMap;


struct iocMemoryBlock;

/* Write status data.
 */
void ioc_status_write(
    struct iocMemoryBlock *mblk,
    int addr,
    const os_char *buf,
    int n);

/* Read status data.
 */
void ioc_status_read(
    struct iocMemoryBlock *mblk,
    int addr,
    os_char *buf,
    int n);

/* Count number of connected streams (sockets, etc).
 */
void ioc_count_connected_streams(
    iocRoot *root,
    os_boolean incement_drop_count);

#endif
