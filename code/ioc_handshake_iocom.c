/**

  @file    ioc_handshake_iocom.c
  @brief   Iocom specific code for first handshake.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if OSAL_SOCKET_SUPPORT


/* Write data to socket.
 */
static osalStatus ioc_write_iocom_socket(
    const os_char *buf,
    os_memsz n,
    os_memsz *n_written,
    void *context)
{
    iocConnection *con = (iocConnection*)context;
    return osal_stream_write(con->stream, buf, n, n_written, OSAL_STREAM_DEFAULT);
}

/* Read data from socket.
 */
static osalStatus ioc_read_iocom_socket(
    os_char *buf,
    os_memsz n,
    os_memsz *n_read,
    void *context)
{
    iocConnection *con = (iocConnection*)context;
    return osal_stream_read(con->stream, buf, n, n_read, OSAL_STREAM_DEFAULT);
}

/* Save received certificate (client only).
 */
static void ioc_save_iocom_trust_certificate(
    const os_uchar *cert,
    os_memsz cert_sz,
    void *context)
{
}


/* Load certificate (server only).
 */
static os_memsz ioc_load_iocom_trust_certificate(
    const os_uchar *cert_buf,
    os_memsz cert_buf_sz,
    void *context)
{
    return 0;
}


/**
****************************************************************************************************

  @brief Do first handshake for the connection (only sockets).
  @anchor ioc_first_handshake

  Socket handshake for switchbox cloud network name and trusted certificate copy

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if ready, OSAL_PENDING while not yet completed. Other values indicate
           an error (broken socket).

****************************************************************************************************
*/
osalStatus ioc_first_handshake(
    struct iocConnection *con)
{
    osalStatus s;

os_boolean cert_match = OS_TRUE;

    if (con->flags & IOC_SOCKET)
    {
        if (!con->handshake_ready)
        {
            /* Server side socket ?
             */
            if (con->flags & IOC_LISTENER) {
                s = ioc_server_handshake(&con->handshake, IOC_HANDSHAKE_REGULAR_SERVER,
                    ioc_read_iocom_socket, con,
                    ioc_write_iocom_socket, con,
                    ioc_load_iocom_trust_certificate, con);
            }

            /* Otherwise client side socket.
             */
            else {
                s = ioc_client_handshake(&con->handshake, IOC_HANDSHAKE_CLIENT, "kepuli", !cert_match,
                    ioc_read_iocom_socket, con,
                    ioc_write_iocom_socket, con,
                    ioc_save_iocom_trust_certificate, con);

                if (s == OSAL_SUCCESS && !cert_match) {
                    return OSAL_STATUS_SERVER_CERT_REJECTED;
                }
            }

            osal_stream_flush(con->stream, OSAL_STREAM_DEFAULT);

            if (s) {
                return s;
            }

            ioc_release_handshake_state(&con->handshake);
            con->handshake_ready = OS_TRUE;
        }
    }

    return OSAL_SUCCESS;
}

#endif
