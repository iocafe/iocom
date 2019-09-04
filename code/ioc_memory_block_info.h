/**

  @file    ioc_memory_block_info.h
  @brief   Memory block information.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    24.6.2019

  Functionality related to memory block information. Memory block information is sent trough
  connections to inform connected devices what data they can access, connect to, etc.

  Memory block information is sent through connection when a connection is established, or
  when new memory block is created while there are existing connections.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOC_MEMORY_BLOCK_INFO_INCLUDED
#define IOC_MEMORY_BLOCK_INFO_INCLUDED

/** Maximum device name string length.
 */
#define IOC_NAME_SZ 12

struct iocMemoryBlock;
struct iocConnection;

/**
****************************************************************************************************
    Memory block information structure.
    This structure is used to pass received memory block information to ioc_mbinfo_received()
    function.
****************************************************************************************************
*/
typedef struct
{
    os_char device_name[IOC_NAME_SZ];
    os_ushort device_nr;
    os_char mblk_name[IOC_NAME_SZ];
    os_ushort mblk_nr;
    os_ushort mblk_id;
    os_ushort nbytes;
    os_ushort flags;
}
iocMemoryBlockInfo;


/**
****************************************************************************************************
    Send info structure within a connection.
****************************************************************************************************
*/
typedef struct
{
    struct iocMemoryBlock *current_mblk;
}
iocSendInfoInCon;


/** 
****************************************************************************************************

  @name Functions related to memory block information

  X...

****************************************************************************************************
 */
/*@{*/

/* Make sure that information about new memory block gets sent.
 */
void ioc_add_mblk_to_mbinfo(
    struct iocMemoryBlock *mblk);

/* Make sure that information about all memory blocks is sent trough a specific connection.
 */
void ioc_add_con_to_mbinfo(
    struct iocConnection *con);

/* Memory block information for connection is not needed for now.
 */
void ioc_mbinfo_con_is_closed(
    struct iocConnection *con);

/* Check if we have information to send trough the connection.
 */
struct iocMemoryBlock *ioc_get_mbinfo_to_send(
    struct iocConnection *con);

/* Memory block information was successfully sent, move on to the next one.
 */
void ioc_mbinfo_sent(
    struct iocConnection *con,
    struct iocMemoryBlock *mblk);

/* Memory block is being deleted, remove it from all send info.
 */
void ioc_mbinfo_mblk_is_deleted(
    struct iocMemoryBlock *mblk);

/* Create source and target buffers according to received memory block information.
 */
void ioc_mbinfo_received(
    struct iocConnection *con,
    iocMemoryBlockInfo *info);

/*@}*/

#endif
