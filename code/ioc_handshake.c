/**

  @file    ioc_handshake.c
  @brief   Handshake for switchbox network selection and for copying trusted certificate.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  This handshake is used to pass "cloud network name" to clients and services connecting to
  switchbox cloud server, and to request trusted sertificate from socket server. Same
  handshake (usually 1 byte) is sent even when switchbox is used, this allows the same
  clients to connect directly or trough switchbox server without modification.

  Second function of this handshake is to allow client for trusted certificate from server.
  This relates to pairing and autoconfiguring TLS security.

  This is done after the TLS hanshaking, but before passing user login information to server.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if OSAL_SOCKET_SUPPORT

/* Forward referred static functions.
 */
static osalStatus ioc_send_client_handshake_message(
    iocHandshakeState *state,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context);

static osalStatus ioc_process_handshake_message(
    iocHandshakeState *state,
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context);

#if OSAL_TLS_SUPPORT
static osalStatus ioc_send_trust_certificate(
    iocHandshakeState *state,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context,
    ioc_hanshake_load_trust_certificate *load_trust_certificate_func,
    void *load_trust_certificate_context);

static osalStatus ioc_process_trust_certificate(
    iocHandshakeState *state,
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context,
    ioc_hanshake_save_trust_certificate *save_trust_certificate_func,
    void *save_trust_certificate_context);
#endif


/**
****************************************************************************************************

  @brief Initialize handshake state structure.
  @anchor ioc_initialize_handshake_state

  The ioc_initialize_handshake_state() function initializes handshake state structure for use
  (mostly fills it with zeros).

  This function must not be called on handshake structure which is already initialized,
  except if ioc_release_handshake_state has been called.

  @param   state Handshake state initialized by ioc_initialize_handshake_state().
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_handshake_state(
    iocHandshakeState *state)
{
    os_memclear(state, sizeof(iocHandshakeState));
    state->cert_sz = 0xFFFF;
}


/**
****************************************************************************************************

  @brief Release memory allocated to maintain handshake state.
  @anchor ioc_release_handshake_state

  The ioc_release_handshake_state() function releases memory allocated for handshake state
  (pointers in state structure).

  This function can be called on any initialized handshake structure, even if it has been
  released before.

  @param   state Handshake state initialized by ioc_initialize_handshake_state().
  @return  None.

****************************************************************************************************
*/
void ioc_release_handshake_state(
    iocHandshakeState *state)
{
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    os_free(state->cloud_netname, state->cloud_netname_sz);
#endif

#if OSAL_TLS_SUPPORT
    os_free(state->cert, state->cert_sz);
#endif

    os_memclear(state, sizeof(iocHandshakeState));
}


/**
****************************************************************************************************

  @brief Do client handshake (socket client only).
  @anchor ioc_client_handshake

  The ioc_client_handshake() does client end of handshaking. This function is called repeatedly
  until it return OSAL_SUCCESS (0). If this function return OSAL_PENDING, the caller
  should call flush on socket.

  @param   state handshake state initialized by ioc_initialize_handshake_state().
  @param   process_type IOC_HANDSHAKE_NETWORK_SERVICE indicates that this socket client is
           IO network service connecting to switchbox cloud server to share an end point.
           IOC_HANDSHAKE_CLIENT if this is an IO device or user interface application
           connecting to IO network service trough a cloud server.
  @param   cloud_netname Name of IO network to service to publish (IOC_HANDSHAKE_NETWORK_SERVICE)
           or connect to (IOC_HANDSHAKE_CLIENT).
  @param   request_trust_certificate Set OS_TRUE to request switchbox cloud service to send
           trust certificate back. If OS_FALSE, no message from cloud server is expected.
  @param   write_socket_func Pointer to socket write function. Using function pointer
           allows using this code with both iocom and ecom protocols.
  @param   write_socket_context Application specific context to pass to write_socket_func().
           Something like stream handle, etc.
  @param   save_trust_certificate_func Function to save trust certificate, called once nonempty
           trust certificate has been completely received.
  @param   save_trust_certificate_context Application specific context to pass to
           save_trust_certificate() function.

  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
osalStatus ioc_client_handshake(
    iocHandshakeState *state,
    iocHandshakeClientType process_type,
    const os_char *cloud_netname,
    os_boolean request_trust_certificate,
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context,
    ioc_hanshake_save_trust_certificate *save_trust_certificate_func,
    void *save_trust_certificate_context)
{
    osalStatus s = OSAL_SUCCESS;
    os_char len;

    /* Send client handshake message (socket client side only).
     */
    if (!state->hand_shake_message_done)
    {
        state->client_type = (os_char)process_type;
#if OSAL_DYNAMIC_MEMORY_ALLOCATION
        if (state->cloud_netname == OS_NULL) {
            len = (os_char)os_strlen(cloud_netname);
            if (len > 1) {
                if (len > OSAL_NETWORK_NAME_SZ) {
                    osal_debug_error_str("Too long cloud netname: ", cloud_netname);
                    return OSAL_STATUS_FAILED;
                }
                state->cloud_netname_sz = len + IOC_HANDSHAKE_HDR_BYTES;
                state->cloud_netname = os_malloc(state->cloud_netname_sz, OS_NULL);
                if (state->cloud_netname == OS_NULL) return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
                os_memcpy(state->cloud_netname + IOC_HANDSHAKE_HDR_BYTES, cloud_netname, len);

                state->cloud_netname[0] = (os_char)process_type | IOC_HANDSHAKE_HAS_NET_NAME_BIT;
#if OSAL_TLS_SUPPORT
                if (request_trust_certificate) {
                    state->cloud_netname[0] |= IOC_HANDSHAKE_REQUEST_TRUST_CERTIFICATE_BIT;
                }
#endif
                state->cloud_netname[1] = (os_char)len;
            }
        }
#endif

#if OSAL_TLS_SUPPORT
        state->copy_trust_certificate = (os_boolean)request_trust_certificate;
#endif

        s = ioc_send_client_handshake_message(state, write_socket_func, write_socket_context);
        if (s) {
            return s;
        }

        state->hand_shake_message_done = OS_TRUE;
    }

#if OSAL_TLS_SUPPORT
    if (request_trust_certificate) {
        s = ioc_process_trust_certificate(state, read_socket_func, read_socket_context,
            save_trust_certificate_func, save_trust_certificate_context);
    }
#endif

    return s;
}


/**
****************************************************************************************************

  @brief Do server handshake (socket server side only).
  @anchor ioc_server_handshake

  The ioc_server_handshake() does server end of handshaking. This function is called repeatedly
  until it return OSAL_SUCCESS (0). If this function return OSAL_PENDING, the caller
  should call flush on socket.

  @param   state handshake state initialized by ioc_initialize_handshake_state().
  @param   process_type Server process type:
           - IOC_HANDSHAKE_SWITCHBOX_ENDPOINT This is switchbox end of cloud connection.
           - IOC_HANDSHAKE_REGULAR_ENDPOINT This is regular socket server side end of connection.
  @param   read_socket_func Pointer to socket read function.
  @param   read_socket_context Application specific context to pass to read_socket_func().
           Something like stream handle, etc.
  @param   write_socket_func Pointer to socket write function. Using function pointer
           allows using this code with both iocom and ecom protocols.
  @param   write_socket_context Application specific context to pass to write_socket_func().
           Something like stream handle, etc.
  @param   load_trust_certificate_func Pointer to function to load trust certificate.
  @param   load_trust_certificate_context Application specific context to pass to
           load_trust_certificate() function.

  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
osalStatus ioc_server_handshake(
    iocHandshakeState *state,
    iocHandshakeServerType process_type,
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context,
    ioc_hanshake_load_trust_certificate *load_trust_certificate_func,
    void *load_trust_certificate_context)
{
    osalStatus s = OSAL_SUCCESS;

    /* Send client handshake message (socket client side only).
     */
    if (!state->hand_shake_message_done)
    {
        state->server_type = (os_char)process_type;
        s = ioc_process_handshake_message(state, read_socket_func, read_socket_context);
        if (s) {
            return s;
        }

        state->hand_shake_message_done = OS_TRUE;
    }

#if OSAL_TLS_SUPPORT
    if (state->copy_trust_certificate) {
        s = ioc_send_trust_certificate(state, write_socket_func, write_socket_context,
            load_trust_certificate_func, load_trust_certificate_context);
    }
#endif

    return s;
}


#if OSAL_DYNAMIC_MEMORY_ALLOCATION
/**
****************************************************************************************************

  @brief Find out is connected client network service a nwteork service or UI, etc. client.
  @anchor ioc_get_handshake_client_type

  The ioc_get_handshake_client_type() is called by switchbox to determine if:
  - IOC_HANDSHAKE_NETWORK_SERVICE The connected client is network service'etc, which should be
    made accessible though switchox cloud service.
  - IOC_HANDSHAKE_CLIENT The connection is from network client and should be forwareded by
    switchbox to service "cloud-netname".

  @param   state handshake state initialized by ioc_initialize_handshake_state().
  @return  Either IOC_HANDSHAKE_NETWORK_SERVICE or IOC_HANDSHAKE_CLIENT.

****************************************************************************************************
*/
iocHandshakeClientType ioc_get_handshake_client_type(
    iocHandshakeState *state)
{
    return state->client_type & IOC_HANDSHAKE_TYPE_MASK;
}

/**
****************************************************************************************************

  @brief Find out is cloud network name to share specified by socket client.
  @anchor ioc_get_handshake_cloud_netname

  The ioc_get_handshake_cloud_netname() is called by switchbox to get cloud networ name:
  - IOC_HANDSHAKE_NETWORK_SERVICE The connected client is network service'etc, and
    this is the name to use for it within switchbox.
  - IOC_HANDSHAKE_CLIENT The connection is from network client and should be forwareded by
    switchbox to service with this name.

  @param   state handshake state initialized by ioc_initialize_handshake_state().
  @return  Pointer to cloud network name string, or OS_NULL if none.

****************************************************************************************************
*/
const os_char *ioc_get_handshake_cloud_netname(
    iocHandshakeState *state)
{
    return state->cloud_netname;
}
#endif


/**
****************************************************************************************************

  @brief Make client handshake message (socket client only).
  @anchor ioc_send_client_handshake_message

  The ioc_send_client_handshake_message() generates outgoing data frame which contains information
  to authenticate this IO device, etc.

  @param   state Current handshake state.
  @param   cloud_netname Name of IO network to service to publish (IOC_HANDSHAKE_NETWORK_SERVICE)
           or connect to (IOC_HANDSHAKE_CLIENT).
  @param   write_socket_func Pointer to socket write function. Using function pointer
           allows using this code with both iocom and ecom protocols.
  @param   write_socket_context Application specific context to pass to write_socket_func().
           Something like stream handle, etc.

  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_send_client_handshake_message(
    iocHandshakeState *state,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context)
{
    os_memsz n_written;
    osalStatus s;
    os_char n, one_byte_handshake;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    if (state->cloud_netname) {
        n = state->cloud_netname_sz - state->cloud_netname_pos;
        if (n > 0) {
            s = write_socket_func(state->cloud_netname + state->cloud_netname_pos,
                n, &n_written, write_socket_context);
            state->cloud_netname_pos += (os_char)n_written;
        }
        return (s == OSAL_SUCCESS && n_written < n) ? OSAL_PENDING : s;
    }
#endif

    one_byte_handshake = state->client_type;
#if OSAL_TLS_SUPPORT
    if (state->copy_trust_certificate) {
        one_byte_handshake |= IOC_HANDSHAKE_REQUEST_TRUST_CERTIFICATE_BIT;
    }
#endif
    n = 1;
    s = write_socket_func(&one_byte_handshake,
        n, &n_written, write_socket_context);

    if (s == OSAL_SUCCESS && n_written < n) {
        s = OSAL_PENDING;
    }

    return s;
}


/**
****************************************************************************************************

  @brief Process received client handshake (socket server only)
  @anchor ioc_process_handshake_message

  The ioc_process_handshake_message() receives and parses handshake message from socket client
  to socket server.

  @param   state Current handshake state.
  @param   read_socket_func Pointer to socket read function.
  @param   read_socket_context Application specific context to pass to read_socket_func().
           Something like stream handle, etc.

  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_process_handshake_message(
    iocHandshakeState *state,
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context)
{
    os_memsz n_read;
    osalStatus s;
    os_char c, n, tmp[OSAL_NETWORK_NAME_SZ], *p;

    if (state->cloud_netname_pos == 0)
    {
        s = read_socket_func(&c, 1, &n_read, read_socket_context);
        state->client_type = c;
        if (s == OSAL_SUCCESS && n_read <= 0) return OSAL_PENDING;
        if (s) return s;
        if ((c & IOC_HANDSHAKE_TYPE_MASK) != IOC_HANDSHAKE_NETWORK_SERVICE &&
            (c & IOC_HANDSHAKE_TYPE_MASK) != IOC_HANDSHAKE_CLIENT)
        {
            return OSAL_STATUS_FAILED;
        }

#if OSAL_TLS_SUPPORT
        if (c & IOC_HANDSHAKE_REQUEST_TRUST_CERTIFICATE_BIT) {
            state->copy_trust_certificate = OS_TRUE;
        }
#endif
        state->cloud_netname_pos = 1;
        if ((s & IOC_HANDSHAKE_HAS_NET_NAME_BIT) == 0) return OSAL_SUCCESS;
    }

    if (state->cloud_netname_pos == 1)
    {
        s = read_socket_func(&c, 1, &n_read, read_socket_context);
        if (s == OSAL_SUCCESS && n_read <= 0) return OSAL_PENDING;
        if (s) return s;
        if (c <= 0 || c > OSAL_NETWORK_NAME_SZ) {
            return OSAL_STATUS_FAILED;
        }
        state->cloud_netname_sz = c;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
        if (state->server_type == IOC_HANDSHAKE_SWITCHBOX_SERVER) {
            state->cloud_netname = os_malloc(c, OS_NULL);
            if (state->cloud_netname == OS_NULL) {
                return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
            }
        }
        state->cloud_netname_pos = IOC_HANDSHAKE_HDR_BYTES;
#endif
    }

    c = state->cloud_netname_pos - IOC_HANDSHAKE_HDR_BYTES;
    n = state->cloud_netname_sz - c;

#if OSAL_DYNAMIC_MEMORY_ALLOCATION
    p = state->cloud_netname ? state->cloud_netname : tmp;
#else
    p = tmp;
#endif
    s = read_socket_func(p + c, n, &n_read, read_socket_context);
    if (s == OSAL_SUCCESS) {
        state->cloud_netname_pos += n_read;
        return n_read >= n ? OSAL_SUCCESS : OSAL_PENDING;
    }
    return s;
}


#if OSAL_TLS_SUPPORT
/**
****************************************************************************************************

  @brief Send trust certificate to client (socket server only)
  @anchor ioc_send_trust_certificate

  The ioc_send_trust_certificate() sends trust certificate to socket client. It is up to
  socket client application to use or not use this certificate. If the sertificate is
  requested (trust_certificate_requested is OS_TRUE), this function must be called.

  @param   state Current handshake state.
  @param   write_socket_func Pointer to socket write function. Using function pointer
           allows using this code with both iocom and ecom protocols.
  @param   write_socket_context Application specific context to pass to write_socket_func().
           Something like stream handle, etc.
  @param   load_trust_certificate_func Pointer to function to load trust certificate.
  @param   load_trust_certificate_context Application specific context to pass to
           load_trust_certificate() function.

  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_send_trust_certificate(
    iocHandshakeState *state,
    ioc_hanshake_write_socket *write_socket_func,
    void *write_socket_context,
    ioc_hanshake_load_trust_certificate *load_trust_certificate_func,
    void *load_trust_certificate_context)
{
    os_memsz n_written;
    osalStatus s;
    os_ushort cert_sz, n;
    static const os_char doublezero[2] = {0,0};

    /* If we have not tried to load the certificate.
     */
    if (state->cert_sz == 0xFFFF)
    {
        state->cert_sz = 0;

        /* Get certificate size in bytes.
         */
        cert_sz = load_trust_certificate_func(OS_NULL, 0, load_trust_certificate_context);
        if (cert_sz > 0) {
            state->cert = (os_uchar*)os_malloc(cert_sz + 2, OS_NULL);
            if (state->cert == OS_NULL) return OSAL_STATUS_FAILED;
            state->cert_sz = cert_sz + 2;
            load_trust_certificate_func(state->cert + 2, cert_sz,
                load_trust_certificate_context);
            state->cert[0] = (os_uchar)cert_sz;
            state->cert[1] = (os_uchar)(cert_sz >> 8);
        }
        else {
            osal_debug_error("No trusted certificate to send");
        }
    }

    if (state->cert_sz >= 2) {
        n = state->cert_sz - state->cert_pos;
        s = write_socket_func((os_char*)state->cert + state->cert_pos,
            n, &n_written, write_socket_context);
    }
    else {
        n = 2 - state->cert_pos;
        s = write_socket_func(doublezero, n, &n_written, write_socket_context);
    }
    if (s) return s;
    state->cert_pos += (os_ushort)n_written;
    return n_written >= n ? OSAL_SUCCESS : OSAL_PENDING;
}
#endif


#if OSAL_TLS_SUPPORT
/**
****************************************************************************************************

  @brief Receive and save trusted certificate (socket client only)
  @anchor ioc_process_trust_certificate

  The ioc_process_trust_certificate() receive trust certificate and call save_trust_certificate_func
  to save it.

  @param   state Current handshake state.
  @param   read_socket_func Pointer to socket read function.
  @param   read_socket_context Application specific context to pass to read_socket_func().
           Something like stream handle, etc.
  @param   save_trust_certificate_func Function to save trust certificate, called once nonempty
           trust certificate has been completely received.
  @param   save_trust_certificate_context Application specific context to pass to
           save_trust_certificate() function.

  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
static osalStatus ioc_process_trust_certificate(
    iocHandshakeState *state,
    ioc_hanshake_read_socket *read_socket_func,
    void *read_socket_context,
    ioc_hanshake_save_trust_certificate *save_trust_certificate_func,
    void *save_trust_certificate_context)
{
    os_memsz n_read;
    osalStatus s;
    os_ushort cert_pos, n;
    os_uchar c;

    while (state->cert_pos < 2) {
        s = read_socket_func((os_char*)&c, 1, &n_read, read_socket_context);
         if (s == OSAL_SUCCESS && n_read <= 0) return OSAL_PENDING;
        if (s) return s;
        state->cert_sz = (state->cert_pos ? (((os_ushort)c) << 8) : c);
        state->cert_pos++;
    }

    if (state->cert_sz == 0) {
        osal_debug_error("Empty trusted certificate received");
        return OSAL_SUCCESS;
    }
    if (state->cert == OS_NULL) {
        state->cert = (os_uchar*)os_malloc(state->cert_sz, OS_NULL);
        if (state->cert == OS_NULL) return OSAL_STATUS_FAILED;
    }
    cert_pos = (state->cert_pos - 2);
    n = state->cert_sz - cert_pos;
    s = read_socket_func((os_char*)state->cert + cert_pos, n, &n_read, read_socket_context);
    if (s == OSAL_SUCCESS && n_read <= 0) return OSAL_PENDING;
    if (s) return s;

    state->cert_pos += (os_ushort)n_read;
    if (n_read < n) return OSAL_PENDING;

    save_trust_certificate_func(state->cert, state->cert_sz, save_trust_certificate_context);
    return OSAL_SUCCESS;
}
#endif

#endif
