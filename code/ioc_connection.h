/**

  @file    ioc_connection.h
  @brief   Connection object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    5.8.2018

  A connection object represents logical connection between two devices. Both ends of
  communication have a connection object dedicated for that link, serialized data is
  transferred from a connection object to another.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#ifndef IOC_CONNECTION_INCLUDED
#define IOC_CONNECTION_INCLUDED

/** Frame sizes for socket and serial connections. These can never be modified, otherwise
 *  communication compatibility will break. Notice that socket frame size is not same as
 *  TCP frame size, signle tcp frame can hold multiple communication frames.
 */
/*@{*/
#define IOC_SOCKET_FRAME_SZ 464
#define IOC_SERIAL_FRAME_SZ 96
/*@}*/

/* Acknowledgement/keep alive message size in bytes.
 */
#define IOC_ACK_SIZE 3

/** Maximum number of bytes in transit. Serial receive buffer must be at least 256 bytes
    (holds 255 bytes of data). 
 */
/*@{*/

/* We can receive this many bytes without sending acknowledgement. Do not change without recalculating
   flow control.
 */
#define IOC_SERIAL_UNACKNOGLEDGED_LIMIT 40

/* We reserve "air space" for this many acknowlegement messages.
   Five acknowlegement messages acknoledge minimum 200 bytes, which is enough to keep communication
   running under any load conditions, and lockup when communication is blocked by flow control by
   both ends at same time cannot arice.
 */
#define IOC_SERIAL_NRO_ACKS_TO_RESEVE 5

/* Minimum serial port receive buffer size. For IO device this may require modifying serial port setup code.
 */
#define IOC_SERIAL_RX_BUF_MIN_SZ 256

/* Maximum data frame bytes that can be written.
 */ 
#define IOC_SERIAL_MAX_IN_AIR (IOC_SERIAL_RX_BUF_MIN_SZ - 1 - IOC_SERIAL_UNACKNOGLEDGED_LIMIT - IOC_SERIAL_NRO_ACKS_TO_RESEVE * IOC_ACK_SIZE)

/* Maximum acknowledge bytes that can be written.
 */ 
#define IOC_SERIAL_MAX_ACK_IN_AIR (IOC_SERIAL_RX_BUF_MIN_SZ - 1)
/*@}*/

/** Buffering for sockets. For socket we make assumption that the underlying socket
    implementation can buffer at least 2KB, so we can send four frames without waiting.
 */
/*@{*/

/* We can receive this many bytes from socket without sending acknowledgement. 
 */
#define IOC_SOCKET_UNACKNOGLEDGED_LIMIT 500

/* We reserve "air space" for this many acknowlegement messages.
 */
#define IOC_SOCKET_NRO_ACKS_TO_RESEVE 5

/* Maximum data frame bytes that can be written.
 */ 
#define IOC_SOCKET_MAX_IN_AIR (3 * IOC_SOCKET_FRAME_SZ)

/* Maximum acknowledge bytes that can be written.
 */ 
#define IOC_SOCKET_MAX_ACK_IN_AIR (IOC_SOCKET_MAX_IN_AIR + IOC_SOCKET_UNACKNOGLEDGED_LIMIT + IOC_SOCKET_NRO_ACKS_TO_RESEVE * IOC_ACK_SIZE)
/*@}*/


/** Flags for ioc_connect() and ioc_listen() functions. Bit fields.
 - IOC_SERIAL_LISTENER Listening end of serial communication. Effects to line negotiation
 - IOC_CONNECT_UPWARDS Connect upward to upper level of IO device hierarchy.
 - IOC_DYNAMIC_MBLKS Dynamically create dynamic memory blocks as needed.
 */
/*@{*/
#define IOC_SERIAL 0
#define IOC_SOCKET 1
#define IOC_CREATE_THREAD 2
#define IOC_CLOSE_CONNECTION_ON_ERROR 4
#define IOC_DYNAMIC_MBLKS 8
#define IOC_SERIAL_LISTENER 16
#define IOC_DISABLE_SELECT 32
#define IOC_CONNECT_UPWARDS 64
/*@}*/


/** Flags for message frame. Bit fields.
 */
/*@{*/
#define IOC_DELTA_ENCODED 1
#define IOC_COMPRESESSED 2
#define IOC_ADDR_HAS_TWO_BYTES 4
#define IOC_ADDR_HAS_FOUR_BYTES 8
#define IOC_MBLK_HAS_TWO_BYTES 16
#define IOC_MBLK_HAS_FOUR_BYTES 32
#define IOC_SYNC_COMPLETE 64
#define IOC_SYSTEM_FRAME 128
/*@}*/

/** "1/2 byte" and "has string" flags for packing memory block info.
    low bits reserved for version.
 */
#define IOC_INFO_D_2BYTES 8
#define IOC_INFO_N_2BYTES 16
#define IOC_INFO_F_2BYTES 32
#define IOC_INFO_HAS_DEVICE_NAME 64
#define IOC_INFO_HAS_MBLK_NAME 128

/** System frame types.
 */
typedef enum
{
    IOC_SYSRAME_MBLK_INFO = 1
}
iocSystemFrameType;

/* Maximum parameter string length for end point.
 */
#define IOC_CONNECTION_PRMSTR_SZ 48


/**
****************************************************************************************************
    Parameters for ioc_connect() function.
****************************************************************************************************
*/
typedef struct
{
    /** Stream interface, use one of OSAL_SERIAL_IFACE, OSAL_BLUETOOTH_IFACE, OSAL_SOCKET_IFACE
        or OSAL_TLS_IFACE defines.
     */
    const osalStreamInterface *iface;

    /** Depending on connection type, can be "127.0.0.1:8817" for TCP socket.
     */
    const os_char *parameters;
  
    /** If socket connection is accepted by listening end point, this is
        the socket handle. Otherwise this argument needs to be OS_NULL.
     */
    osalStream newsocket;

    /** Pointer to static frame buffer. OS_NULL to allocate the frame buffer.
     */
    os_char *frame_out_buf;
    
    /** Size of static frame buffer, either IOC_SOCKET_FRAME_SZ or IOC_SERIAL_FRAME_SZ.
        Zero for dynamically or pool allocated buffer.
     */
    os_int frame_out_buf_sz;

    /** Pointer to static frame buffer. OS_NULL to allocate the frame buffer.
     */
    os_char *frame_in_buf;

    /** Size of static frame buffer, either IOC_SOCKET_FRAME_SZ or IOC_SERIAL_FRAME_SZ
        Zero for dynamically or pool allocated buffer.
     */
    os_int frame_in_buf_sz;
    
    /** Flags Bit fields:
        - IOC_SOCKET Connect with TCP socket.
        - IOC_CREATE_THREAD Create thread to run connection (multithread support needed).
     */
    os_short flags;
} 
iocConnectionParams;


/**
****************************************************************************************************
    Member variables for frame being sent.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to outgoing frame buffer.
     */
    os_char *buf;

    /** Number of used bytes in buffer (current frame size). Zero if frame buffer is not used.
     */
    os_int used;

    /** Current send position within the buffer. 0 = at beginning of buffer.
     */
    os_int pos;

    /** Flags indication that outgoing frame buffer (buf) has been allocated by ioc_connect()
     */
    os_boolean allocated;

    /** Current frame count for serial communication frame enumeration.
     */
    os_uchar frame_nr;
}
iocConnectionOutgoingFrame;


/**
****************************************************************************************************
    Member variables for incoming frame.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to infoming frame buffer.
     */
    os_char *buf;

    /** Current receive position within the buffer. 0 = at beginning of buffer.
     */
    os_int pos;

    /** Flags indication that incoming frame buffer (buf) has been allocated by ioc_connect()
     */
    os_boolean allocated;

    /** Current frame count for serial communication frame enumeration.
     */
    os_uchar frame_nr;
}
iocConnectionIncomingFrame;


#if OSAL_MULTITHREAD_SUPPORT
/**
****************************************************************************************************
    Worker thread specific member variables.
****************************************************************************************************
*/
typedef struct
{
    /** Event to activate the worker thread.
     */
    osalEvent trig;

    /** True If running worker thread for the end point.
     */
    os_boolean thread_running;

    /** Flag to terminate worker thread.
     */
    os_boolean stop_thread;
}
iocConnectionWorkerThread;
#endif


/**
****************************************************************************************************
    Linked list of connection's source buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to connection's first source buffer in linked list.
     */
    struct iocSourceBuffer *first;

    /** Pointer to connection's last source buffer in linked list.
     */
    struct iocSourceBuffer *last;

    /** Connection buffer from which last send was done.
     */
    struct iocSourceBuffer *current;
}
iocConnectionsSourceBufferList;


/**
****************************************************************************************************
    Linked list of connection's target buffers.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to connection's first target buffer in linked list.
     */
    struct iocTargetBuffer *first;

    /** Pointer to connection's last target buffer in linked list.
     */
    struct iocTargetBuffer *last;
}
iocConnectionsTargetBufferList;


/**
****************************************************************************************************
    This connection in root's linked list of connections.
****************************************************************************************************
*/
typedef struct
{
    /** Pointer to the root object.
     */
    iocRoot *root;

    /** Pointer to the next connection in linked list.
     */
    struct iocConnection *next;

    /** Pointer to the previous connection in linked list.
     */
    struct iocConnection *prev;
}
iocConnectionLink;

/**
****************************************************************************************************
    This connection in root's linked list of connections. See ioc_establish_serial_connection.c
****************************************************************************************************
*/
typedef enum
{
    OSAL_SERCON_STATE_INIT_1 = 0,
    OSAL_SERCON_STATE_INIT_2,
    OSAL_SERCON_STATE_INIT_3,
    OSAL_SERCON_STATE_INIT_4,
    OSAL_SERCON_STATE_CONNECTED_5
}
iocSerialConnectionState;

/**
****************************************************************************************************
    Special characters used to establish connection. See ioc_establish_serial_connection.c
****************************************************************************************************
*/
typedef enum
{
    IOC_ACKNOWLEDGE = 255,
    IOC_SERIAL_CONNECT = 253,
    IOC_SERIAL_CONNECT_REPLY = 252,
    IOC_SERIAL_CONFIRM = 251,
    IOC_SERIAL_CONFIRM_REPLY = 250,
    IOC_SERIAL_DISCONNECT = 249
}
iocSerialCtrlChar;

/* Frame count runs normally from 1 to IOC_MAX_FRAME_NR. Exception is when the connection
   is established, the first frame count 0 zero, which never repeats. The IOC_MAX_FRAME_NR
   must be less than any control character used.
 */
#define IOC_MAX_FRAME_NR 200


/**
****************************************************************************************************

  @name Connection object structure.

  X...

****************************************************************************************************
*/
typedef struct iocConnection
{
    /** Debug identifier must be first item in the object structure. It is used to verify
        that a function argument is pointer to correct initialized object.
     */
    IOC_DEBUG_ID

    /** Flags as given to ioc_listen()
     */
    os_short flags;

    /** Parameter string
     */
    os_char parameters[IOC_CONNECTION_PRMSTR_SZ];

    /** Total frame size, constant for connection type. For example IOC_SOCKET_FRAME_SZ
        for socket communication.
     */
    os_int frame_sz;

    /** Flow control: Maximum number of bytes in transit without being acknowledged (for data).
     */
    os_int max_in_air;

    /** Flow control: Maximum number of bytes in transit without being 
        acknowledged (for ack messages).
     */
    os_ushort max_ack_in_air;

    /** Number of bytes to leave unacknwledged (Minimum acknowledgement size).
     */
    os_ushort unacknogledged_limit;

    /** OSAL stream handle (socket or serial port).
     */
    osalStream stream;

    /** Stream interface pointer, one of OSAL_SERIAL_IFACE, OSAL_SOCKET_IFACE
        or OSAL_TLS_IFACE.
     */
    const osalStreamInterface *iface;

    /** Timer to measure how long since last failed stream open try.
        Zero if stream has not been tried or it has succeeded the
        last time.
     */
    os_timer socket_open_fail_timer;

    /** Timer of the last successfull receive.
     */
    os_timer last_receive;

    /** Timer of the last successfull send.
     */
    os_timer last_send;

    /** Member variables for current outgoing frame.
     */
    iocConnectionOutgoingFrame frame_out;

    /** Member variables for current incoming frame.
     */
    iocConnectionIncomingFrame frame_in;

#if OSAL_SERIAL_SUPPORT
    /** Serial connection state
     */
    iocSerialConnectionState sercon_state;

    /** Serial communication connect sequence timer.
     */
    os_timer sercon_timer;
#endif

    /** Number of received bytes since last connect.
     */
    os_ushort bytes_received;

    /** Number of bytes acknowledged.
     */
    os_ushort bytes_acknowledged;

    /** Number of bytes sent since last connect.
     */
    os_ushort bytes_sent;

    /** Number of bytes received by the other end
        of the connection (last received RBYTES value).
     */
    os_ushort processed_bytes;

#if OSAL_MULTITHREAD_SUPPORT
    /** Worker thread specific member variables.
     */
    iocConnectionWorkerThread worker;
#endif

    /** Linked list of connection's source buffers.
     */
    iocConnectionsSourceBufferList sbuf;

    /** Linked list of connection's target buffers.
     */
    iocConnectionsTargetBufferList tbuf;

    /** This connection in root's linked list of connections.
     */
    iocConnectionLink link;

    /** Keeping track where memory block info needs to be sent.
     */
    iocSendInfoInCon sinfo;

    /** Automatic device number, used if device number is 0
     */
    os_int auto_device_nr;

    /** Flag indicating that stream is connected. Connected
        means that one message has been successfully received.
     */
    os_boolean connected;

    /** Flag indicating that the connection structure was dynamically allocated.
     */
    os_boolean allocated;
}
iocConnection;


/**
****************************************************************************************************

  @name Functions related to iocom root object

  The ioc_initialize_connection() function initializes or allocates new connection object,
  and ioc_release_connection() releases resources associated with it. Memory allocated for the
  connection is freed, if the memory was allocated by ioc_initialize_connection().

  The ioc_read() and ioc_write() functions are used to access data in connection.

****************************************************************************************************
 */
/*@{*/

/* Initialize connection object.
 */
iocConnection *ioc_initialize_connection(
    iocConnection *con,
    iocRoot *root);

/* Release connection object.
 */
void ioc_release_connection(
    iocConnection *con);

/* Close underlying socket or serial port.
 */
void ioc_close_stream(
    iocConnection *con);

/* Start or prepare the connection.
 */
osalStatus ioc_connect(
    iocConnection *con,
    iocConnectionParams *prm);

/* Connect and move data.
 */
osalStatus ioc_run_connection(
    iocConnection *con);

/* Request to terminate connection worker thread.
 */
osalStatus ioc_terminate_connection_thread(
    iocConnection *con);

/* Reset connection state to start from beginning
 */
void ioc_reset_connection_state(
    iocConnection *con);

/* Send frame to connection, if any available.
 */
osalStatus ioc_connection_send(
    iocConnection *con);

/* Send keep alive data to connection, if any available.
 */
osalStatus ioc_send_acknowledge(
    iocConnection *con);

/* Send keep alive frame when nothing has been sent for a while.
 */
osalStatus ioc_send_timed_keepalive(
    iocConnection *con,
    os_timer *tnow);

/* Acknowledge if we have reached the limit of unacknowledged bytes.
 */
osalStatus ioc_acknowledge_as_needed(
    iocConnection *con);

/* Receive data from connection.
 */
osalStatus ioc_connection_receive(
    iocConnection *con);

/* Negotiate beginning of a serial connection.
 */
osalStatus ioc_establish_serial_connection(
    iocConnection *con);
/*@}*/

#endif
