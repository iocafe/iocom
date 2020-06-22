/**

  @file    ioc_memory_block_info.h
  @brief   Memory block information.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.1.2020

  Functionality related to memory block information. Memory block information is sent trough
  connections to inform connected devices what data they can access, connect to, etc.

  Memory block information is sent through connection when a connection is established, or
  when new memory block is created while there are existing connections.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/

/** Maximum signal name string length.
 */
#define IOC_SIGNAL_NAME_SZ 32

/** Maximum device and memory block name string length.
 */
#define IOC_NAME_SZ 16

/** Maximum device identification string length (device name and number together).
    Theoretical maximum device number is 2^32, which is 10 characters, with name totals
    26 characters. Rounded up to four byte boundary, thus 28.
 */
#define IOC_DEVICE_ID_SZ 28

/** Maximum password string length. We use SHA-256 to encrypt passwords, and convert
    result to string, so we need 11 * 4 + 2 = 46 bytes. 11 = SHA hash is 32 bytes, we have
    11 groups of three bytes each. 4 = each 3 byte group on is converted to 4 characters in
    string. 2 = one byte for '!' prefix and one byte for terminating '\0' character.
 */
#define IOC_PASSWORD_SZ OSAL_SECRET_STR_SZ

/** Maximum network name string length.
 */
#define IOC_NETWORK_NAME_SZ 24

/** Signal path is signal.mblk.deviceX.network", thus maximum path size in bytes is:
 */
#define IOC_SIGNAL_PATH_SZ (IOC_SIGNAL_NAME_SZ + IOC_NAME_SZ + IOC_DEVICE_ID_SZ + IOC_NETWORK_NAME_SZ + 1)

/** Memory block path is "mblk.deviceX.network", thus maximum path size in bytes is:
 */
#define IOC_MBLK_PATH_SZ (IOC_NAME_SZ + IOC_DEVICE_ID_SZ + IOC_NETWORK_NAME_SZ + 1)

/** Device path is "deviceX.network", thus maximum path size in bytes is:
 */
#define IOC_DEVICE_PATH_SZ (IOC_DEVICE_ID_SZ + IOC_NETWORK_NAME_SZ + 1)

/** "1/2 byte" and "has string", etc flags for packing memory block info.
 */
#define IOC_INFO_D_2BYTES 2
#define IOC_INFO_D_4BYTES 4
#define IOC_INFO_N_2BYTES 8
#define IOC_INFO_N_4BYTES 16
#define IOC_INFO_F_2BYTES 32
#define IOC_INFO_HAS_DEVICE_NAME 64
#define IOC_INFO_HAS_MBLK_NAME 128

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
    os_uint device_nr;
    os_char network_name[IOC_NETWORK_NAME_SZ];
    os_char mblk_name[IOC_NAME_SZ];
    os_uint mblk_id;
    os_ushort nbytes;
    os_ushort flags;

    /** Local memory block flags (not serialized).
        Flag IOC_MBLK_LOCAL_AUTO_ID: Device number was automatically generated by this process.
     */
    os_short local_flags;
}
iocMemoryBlockInfo;


/**
****************************************************************************************************
    Send info structure within a connection.
****************************************************************************************************
*/
typedef struct
{
    /** Global upwards info transfer list. Pointer to the first memory block for
        which info needs to be sent upwards trough this connection.
     */
    struct iocMemoryBlock *current_mblk;

#if IOC_SERVER2CLOUD_CODE
    /** Pointer to next cloud specific memory block whose info needs to be send trough
        downwards connection.
     */
    struct iocMemoryBlock *current_cloud_mblk;
#endif
}
iocSendInfoInCon;


/** 
****************************************************************************************************
  Functions for managing memory block information
****************************************************************************************************
 */
/*@{*/

/* Make sure that information about new memory block gets sent.
 */
void ioc_add_mblk_to_global_mbinfo(
    struct iocMemoryBlock *mblk);

/* Make sure that information about all memory blocks is sent trough a specific connection.
 */
void ioc_add_con_to_global_mbinfo(
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

/* Process memory block information frame received from socket or serial port.
 */
osalStatus ioc_process_received_mbinfo_frame(
    struct iocConnection *con,
    os_uint mblk_id,
    os_char *data);

/* Create source and target buffers according to received memory block information.
 */
void ioc_mbinfo_received(
    struct iocConnection *con,
    iocMemoryBlockInfo *info);

/*@}*/
