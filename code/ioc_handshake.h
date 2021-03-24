/**

  @file    ioc_handshake.h
  @brief   Handshake for switchbox network selection and for copying trusted certificate.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

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

/** IOC_HANDSHAKE_NETWORK_SERVICE indicates that this socket client is IO network service
    connecting to cloud server to share an end point.
    IOC_HANDSHAKE_CLIENT is that this is an IO device or user interface application
    connecting to IO network service trough a cloud server.
 */
typedef enum iocHandshakeProcessType {
    IOC_HANDSHAKE_NETWORK_SERVICE = 0x61,
    IOC_HANDSHAKE_CLIENT = 0x62
}
iocHandshakeProcessType;

/* Write data to socket.
 */
typedef osalStatus ioc_hanshake_write_socket(
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    os_int flags,
    void *context);

/* Read data from socket.
 */
typedef osalStatus ioc_hanshake_read_socket(
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    os_int flags,
    void *context);

/* Save received certificate (client only).
 */
typedef void ioc_hanshake_save_trust_certificate(
    const os_char *cert,
    os_memsz cert_sz,
    void *context);



/**
****************************************************************************************************
  Function for sending/receiving switchbox network selection and trusted certificate.
****************************************************************************************************
 */
/*@{*/

/* Make client handshake message (socket client side only).
 */
osalStatus ioc_make_clent_handshake_message(
    iocHandshakeProcessType process_type,
    const os_char *cloud_netname,
    os_boolean request_trust_certificate,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context);

/* Process received client handshake (only server side of the socket)
 */
osalStatus ioc_process_clent_handshake_message(
    iocHandshakeProcessType *process_type,
    os_char *cloud_netname,
    os_memsz cloud_netname_sz,
    os_boolean *trust_certificate_requested,
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context);

/* Send trust certificate to client (only server side of the socket)
 */
osalStatus ioc_send_trust_certificate(
    const os_char *cert,
    os_memsz cert_sz,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context);

/* Receive and save trusted certificate (socket client side only)
 */
osalStatus ioc_process_trust_certificate(
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context,
    ioc_hanshake_save_trust_certificate *save_trust_certificate_func,
    void *save_trust_certificate_context);

/*@}*/

#endif
#endif
