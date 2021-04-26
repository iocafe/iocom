/**

  @file    ioc_handshake.h
  @brief   Handshake for switchbox network selection and for copying trusted certificate.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_HANDSHAKE_H_
#define IOC_HANDSHAKE_H_
#include "iocom.h"
#if OSAL_SOCKET_SUPPORT


/**
****************************************************************************************************
  Defines and structures.
****************************************************************************************************
*/

/** IOC_HANDSHAKE_NETWORK_SERVICE indicates that this socket client is IO network service
    connecting to cloud server to share an end point.
    IOC_HANDSHAKE_CLIENT is that this is an IO device or user interface application
    connecting to IO network service either directly or trough a cloud server.
 */
typedef enum iocHandshakeClientType {
    IOC_HANDSHAKE_NETWORK_SERVICE = 0x12,
    IOC_HANDSHAKE_CLIENT = 0x13
}
iocHandshakeClientType;

/** IOC_HANDSHAKE_SWITCHBOX_ENDPOINT This is switchbox end of cloud connection.
    IOC_HANDSHAKE_REGULAR_ENDPOINT This is regular socket server side end of connection.
 */
typedef enum iocHandshakeServerType
{
    IOC_HANDSHAKE_SWITCHBOX_SERVER = 0x14,
    IOC_HANDSHAKE_REGULAR_SERVER = 0x15
}
iocHandshakeServerType;

#define IOC_HANDSHAKE_HDR_BYTES 2
#define IOC_HANDSHAKE_REQUEST_TRUST_CERTIFICATE_BIT 0x80
#define IOC_HANDSHAKE_HAS_NET_NAME_BIT 0x40
#define IOC_HANDSHAKE_TYPE_MASK 0x3F
#define IOC_HANDSHAKE_SECURE_MARK_BYTE 0x5B

/** Current handshake state.
 */
typedef struct iocHandshakeState
{
    /** Socket client type, see enum iocHandshakeClientType, may contain IOC_HANDSHAKE_HAS_NET_NAME_BIT
       or IOC_HANDSHAKE_REQUEST_TRUST_CERTIFICATE_BIT.
     */
    os_char client_type;

    /** Socket server type, see enum iocHandshakeServerType.
     */
    os_char server_type;

    /** Handshake message has been dealt with, certificate may follow.
     */
    os_boolean hand_shake_message_done;

#if OSAL_TLS_SUPPORT
    os_boolean copy_trust_certificate;
    os_boolean mark_byte_done;
#endif

    /** Current read/write position in cloud_netname, offset IOC_HANDSHAKE_HDR_BYTES.
     */
    os_char cloud_netname_pos;

    /** Size of network name, bytes. In client includes header, in server not.
     */
    os_char cloud_netname_sz;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_char *cloud_netname;
#endif

#if OSAL_TLS_SUPPORT
    os_uchar *cert;
    os_ushort cert_sz;
    os_ushort cert_pos;
#endif
}
iocHandshakeState;


/**
****************************************************************************************************
  Functions type definitions to access socket and server certificate.
****************************************************************************************************
*/

/* Save received certificate (client only).
 */
typedef void ioc_hanshake_save_trust_certificate(
    const os_uchar *cert,
    os_memsz cert_sz,
    void *context);

/* Load certificate (server only).
 */
typedef os_memsz ioc_hanshake_load_trust_certificate(
    const os_uchar *cert_buf,
    os_memsz cert_buf_sz,
    void *context);


/**
****************************************************************************************************
  Functions for sending/receiving switchbox network selection and trusted certificate.
****************************************************************************************************
*/

/* Initialize handshake state structure.
 */
void ioc_initialize_handshake_state(
    iocHandshakeState *hs_state);

/* Release memory allocated to maintain handshake state.
 */
void ioc_release_handshake_state(
    iocHandshakeState *hs_state);

/* Do client handshake (socket client only).
 */
osalStatus ioc_client_handshake(
    iocHandshakeState *state,
    iocHandshakeClientType process_type,
    const os_char *cloud_netname,
    os_boolean request_trust_certificate,
    osalStream stream,
    ioc_hanshake_save_trust_certificate *save_trust_certificate_func,
    void *save_trust_certificate_context);

/* Do server handshake (socket server side only).
 */
osalStatus ioc_server_handshake(
    iocHandshakeState *state,
    iocHandshakeServerType process_type,
    osalStream stream,
    ioc_hanshake_load_trust_certificate *load_trust_certificate_func,
    void *load_trust_certificate_context);

/* Get client type after ioc_server_handshake() has been completed.
 */
iocHandshakeClientType ioc_get_handshake_client_type(
    iocHandshakeState *state);

/* Get network name to use in cloud after server handshake.
 */
const os_char *ioc_get_handshake_cloud_netname(
    iocHandshakeState *state);

#endif
#endif
