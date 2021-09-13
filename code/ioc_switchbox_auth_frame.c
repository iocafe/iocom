/**


  @file    ioc_switchbox_auth_frame.c
  @brief   Device/user authentication for switchbox and ecom.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Low level handling of authentication messages for ecom and switchbox communication. The base
  iocom library contains it's own authentication message related code, this implementation is
  intended for switchbox and ecom, to use interchangable IOCOM compatible authentication messages.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_SWITCHBOX_SUPPORT


/* Forward referred static functions.
 */
static void ioc_make_switchbox_authentication_message(
    iocSwitchboxAuthenticationFrameBuffer *abuf,
    iocSwitchboxAuthenticationParameters *prm);

static osalStatus ioc_switchbox_parse_authentication_message(
    os_uchar *buf,
    iocAuthenticationResults *results);


/**
****************************************************************************************************

  @brief Send switchbox/ecom authentication message to stream.
  @anchor ioc_send_switchbox_authentication_message

  If called with empty buffer, the ioc_send_switchbox_authentication_message() function
  generates outgoing data frame which contains information to authenticate the user, etc.
  This function needs to be called repeatedly (as triggered by select) until authentication
  frame is sent and the function returns OSAL_COMPLETED, or an error occurs.

  This authentication message follows the IOCOM format, so  same switchbox process can handle
  both IOCOM and ECOM protocols.

  Stream must be called after calling this function.

  @param   stream OSAL stream structure.
  @param   abuf Buffer structure for creating and sending the authentication message. Zero this
           structure before calling this function the first time.
  @param   prm Data to place within for authentication message. This must be set when the function
           is called for the first time with empty abuf. For consequent calls, this parmeter
           is ignored.

  @return  OSAL_COMPLETED Authentication message has been completely sent.
           OSAL_PENDING Authentication message not yet send, but no error thus far.
           Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_send_switchbox_authentication_message(
    osalStream stream,
    iocSwitchboxAuthenticationFrameBuffer *abuf,
    iocSwitchboxAuthenticationParameters *prm)
{
    os_memsz n_written, n;
    osalStatus s;

    if (abuf->buf_used == 0) {
        ioc_make_switchbox_authentication_message(abuf, prm);
    }

    n = abuf->buf_used - abuf->buf_pos;
    if (n > 0) {
        s = osal_stream_write(stream, abuf->buf, n, &n_written, OSAL_STREAM_DEFAULT);
        if (OSAL_IS_ERROR(s)) return s;
    }
    abuf->buf_pos += (os_short)n_written;

    /* s = osal_stream_flush(stream, OSAL_STREAM_DEFAULT);
    if (OSAL_IS_ERROR(s)) return s; */
    if (abuf->buf_pos >= abuf->buf_used) return OSAL_COMPLETED;

    return os_has_elapsed(&abuf->ti, 20000) ? OSAL_STATUS_FAILED : OSAL_PENDING;
}


/**
****************************************************************************************************

  @brief Generate authentication message.
  @anchor ioc_make_authentication_message ioc_send_switchbox_authentication_message

  The ioc_make_switchbox_authentication_message is a helper function to generate an authentication
  frame. It is called by the ioc_send_switchbox_authentication_message.

  @param   abuf Buffer structure for creating and sending the authentication message. Zero this
           structure before calling this function the first time.
  @param   prm Data to place within for authentication message. This must be set when the function
           is called for the first time with empty abuf. For consequent calls, this parmeter
           is ignored.

****************************************************************************************************
*/
static void ioc_make_switchbox_authentication_message(
    iocSwitchboxAuthenticationFrameBuffer *abuf,
    iocSwitchboxAuthenticationParameters *prm)
{
    iocSendHeaderPtrs
        ptrs;

    os_uchar
        *p,
        *auth_flags_ptr,
        flags,
        *start;

    os_char
        *buf;

    const os_char
        *network_name,
        *user_name;

    const os_char
        *password;

    os_int
        send_device_nr,
        content_bytes,
        used_bytes;

    os_ushort
        crc;

    // os_char user_name_buf[IOC_NAME_SZ], *q;

    buf = abuf->buf;

    /* Generate IOCOM frame header.
     */
    ioc_generate_header(OS_NULL, buf, &ptrs, 0, 0);

    /* Generate frame content. Here we do not check for buffer overflow,
       we know (and trust) that it fits within one frame.
     */
    p = start = (os_uchar*)buf + ptrs.header_sz;
    *(p++) = IOC_AUTHENTICATION_DATA;
    flags = 0;
    auth_flags_ptr = p;
    *(p++) = 0;

    network_name = prm->network_name;
    user_name = prm->user_name;

#if 0
// #if IOC_AUTHENTICATION_CODE
    /* If we have user name, we use it instead of device name. User name
       may have also network name, like root.cafenet.
     */
    if (con->user_override[0] != '\0')
    {
        os_strncpy(user_name_buf, con->user_override, sizeof(user_name_buf));
        q = os_strchr(user_name_buf, '.');
        if (q) *q = '\0';
        q = os_strchr(con->user_override, '.');
        if (q) network_name = q + 1;
    }
#endif

    ioc_msg_setstr(user_name, &p);
    send_device_nr = 0;
    ioc_msg_set_uint(send_device_nr,
        &p, &flags, IOC_AUTH_DEVICE_NR_2_BYTES, &flags, IOC_AUTH_DEVICE_NR_4_BYTES);
#if OSAL_SECRET_SUPPORT
    if (send_device_nr == 0) {
        os_memcpy(p, osal_global->saved.unique_id_bin, OSAL_UNIQUE_ID_BIN_SZ);
        flags |= IOC_AUTH_UNIQUE_ID;
        p += OSAL_UNIQUE_ID_BIN_SZ;
    }
#endif
    ioc_msg_setstr(network_name, &p);

    password = osal_str_empty;
#if 0
// #if IOC_AUTHENTICATION_CODE
    if ((con->flags & (IOC_LISTENER|IOC_SECURE_CONNECTION)) == IOC_SECURE_CONNECTION)
    {
        /* If we have password given by user
         */
        if (con->password_override[0] != '\0')
        {
            password = con->password_override;
        }
        else
        {
            password = root->password;
        }
        /* If we do not have client certificate chain, set flag to indicate it.
         */
        if (osal_get_network_state_int(OSAL_NS_NO_CERT_CHAIN, 0))
        {
            flags |= IOC_AUTH_CERTIFICATE_REQUEST;
        }
    }
#endif
    ioc_msg_setstr(password, &p);

    /* Set connect up and bidirectional flags.
     */
    /* if (con->flags & IOC_CONNECT_UP)
    {
        flags |= IOC_AUTH_CONNECT_UP;
    } */

    *auth_flags_ptr = flags;

    /* Finish outgoing frame with data size, frame number, and optional checksum. Quit here
       if transmission is blocked by flow control.
     */
    content_bytes = (os_int)(p - start);
    used_bytes = content_bytes + ptrs.header_sz;

    /* Fill in data size and flag as system frame.
     */
    *ptrs.data_sz_low = (os_uchar)content_bytes;
    if (ptrs.data_sz_high)
    {
        content_bytes >>= 8;
        *ptrs.data_sz_high = (os_uchar)content_bytes;
    }
    *ptrs.flags |= IOC_SYSTEM_FRAME;

    /* Calculate check sum out of whole used frame buffer. Notice that check sum
     * position within frame is zeros when calculating the check sum.
     */
    if (ptrs.checksum_low)
    {
        crc = os_checksum(buf, used_bytes, OS_NULL);
        *ptrs.checksum_low = (os_uchar)crc;
        *ptrs.checksum_high = (os_uchar)(crc >> 8);
    }

    abuf->buf_used = used_bytes;
    abuf->buf_pos = 0;
    os_get_timer(&abuf->ti);
}


/**
****************************************************************************************************

  @brief Receive and process swtchbox/ecom authentication message from stream.
  @anchor ecom_process_received_authentication_message

  The ecom_receive_authentication_message() function is called once a complete system frame
  containing authentication data for a device has been received. The authentication data
  identifies the device (device name, number and network name), optionally identifies the user
  with user name and can have password for the connection.
  If user authentication is enabled (by ioc_enable_user_authentication() function), the
  user is authenticated.

  This call should be called repeatedly (as triggered by select) until authentication message
  is completely received and processed, or an error occurs. The function will return
  OSAL_COMPLETED once authentication
  frame is sent.

  @param   stream OSAL stream structure.

  @return  OSAL_COMPLETED Authentication message has been received and processed.
           OSAL_PENDING Authentication message not yet received, but no error thus far.
           Other return values indicate an error.

****************************************************************************************************
*/
osalStatus icom_switchbox_process_authentication_message(
    osalStream stream,
    iocSwitchboxAuthenticationFrameBuffer *abuf,
    iocAuthenticationResults *results)
{
    iocReadFrameState rfs;
    os_uchar *p;

    os_uint
        mblk_id;

    os_uint
        addr;

    osalStatus status;

#if OSAL_SERIAL_SUPPORT
    os_ushort
        crc;
#endif

    os_memclear(&rfs, sizeof(rfs));
    rfs.buf = (os_uchar*)abuf->buf;
    rfs.n = abuf->buf_pos;
    rfs.frame_sz = IOC_MAX_AUTHENTICATION_FRAME_SZ;
    if (abuf->ti == 0) {
        // rfs.needed = 5;
        os_get_timer(&abuf->ti);
    }

    /* Read one received frame using IOCOM frame format.
     */
    status = ioc_read_frame(&rfs, stream);
    if (status) {
        return status;
    }

    /* If we received something, record time of receive and add to bytes received.
     */
    if (rfs.bytes_received) {
        os_get_timer(&abuf->ti);
    }
    abuf->buf_pos = rfs.n;

    /* If we have not received whole frame, we need to wait.
     */
    if (rfs.n < rfs.needed)
    {
        return os_has_elapsed(&abuf->ti, 20000) ? OSAL_STATUS_FAILED : OSAL_PENDING;
    }

#if OSAL_SERIAL_SUPPORT
    /* Get and verify check sum.
     */
    if (rfs.is_serial)
    {
        /* Get the checksum from the received data and clear the checksum position
           in received data. The check sum must be zeroed, because those values
           are zeroes when calculating check sum while sending.
         */
        crc = rfs.buf[1] | ((os_ushort)rfs.buf[2] << 8);
        rfs.buf[1] = rfs.buf[2] = 0;

        if (crc != os_checksum((os_char*)rfs.buf, rfs.needed, OS_NULL))
        {
            osal_trace("Checksum error");
            return OSAL_STATUS_FAILED;
        }
    }
#endif

    p = (os_uchar*)rfs.buf + (rfs.is_serial ? 5 : 4);
    if (rfs.extra_flags) p++;
    mblk_id = ioc_msg_get_uint(&p, rfs.flags & IOC_MBLK_HAS_TWO_BYTES,
        rfs.extra_flags & IOC_EXTRA_MBLK_HAS_FOUR_BYTES);
    addr = ioc_msg_get_uint(&p, rfs.flags & IOC_ADDR_HAS_TWO_BYTES,
        rfs.extra_flags & IOC_EXTRA_ADDR_HAS_FOUR_BYTES);

    /* Whole authentication message successfully received, parse content.
     */
    return ioc_switchbox_parse_authentication_message(p, results);
}



/**
****************************************************************************************************

  @brief Receive and process swtchbox/ecom authentication message from stream.
  @anchor ecom_process_received_authentication_message

  The ecom_receive_authentication_message() function is called once a complete frame
  containing authentication data for a device has been received. The authentication data
  identifies the device (device name, number and network name), optionally identifies the user
  with user name and can have password for the connection.

  If user authentication is enabled (by ioc_enable_user_authentication() function), the
  user is authenticated.

  This call should be called repeatedly (as triggered by select) until authentication message
  is completely received and processed, or an error occurs. The function will return
  OSAL_COMPLETED once authentication message is sent.

  @param   con Pointer to the connection object.
  @param   mblk_id Memory block identifier in this end.
  @param   data Received data, can be compressed and delta encoded, check flags.

  @return  OSAL_COMPLETED Authentication message has been received and processed.
           OSAL_PENDING Authentication message not yet send, but no error thus far.
           Other return values indicate an error.

****************************************************************************************************
*/
static osalStatus ioc_switchbox_parse_authentication_message(
    os_uchar *buf,
    iocAuthenticationResults *results)
{
    iocUser user;
    os_uchar *p, auth_flags;
    osalStatus s;
    // os_char tmp_password[IOC_PASSWORD_SZ];
    os_char nbuf[OSAL_NBUF_SZ];
    os_uint device_nr;

    p = (os_uchar*)buf + 1; /* Skip system frame IOC_SYSFRAME_MBLK_INFO byte. */
    auth_flags = (os_uchar)*(p++);

    os_memclear(&user, sizeof(user));
    user.flags = auth_flags;

#if 0
    /* If listening end of connection (server).
     */
    if (con->flags & IOC_LISTENER)
    {
        if (auth_flags & IOC_AUTH_CONNECT_UP)
        {
            con->flags &= ~IOC_CONNECT_UP;
        }
        else
        {
            if ((con->flags & IOC_CONNECT_UP) == 0)
            {
                con->flags |= IOC_CONNECT_UP;
                ioc_add_con_to_global_mbinfo(con);
            }
        }
    }
    if (auth_flags & IOC_AUTH_CLOUD_CON)
    {
        con->flags |= IOC_CLOUD_CONNECTION;
    }

    if (auth_flags & IOC_AUTH_CERTIFICATE_REQUEST)
    {
        con->flags |= IOC_NO_CERT_CHAIN;
    }
#endif

    s = ioc_msg_getstr(user.user_name, IOC_DEVICE_ID_SZ, &p);
    if (s) return OSAL_STATUS_FAILED;

    device_nr = ioc_msg_get_uint(&p,
        auth_flags & IOC_AUTH_DEVICE_NR_2_BYTES,
        auth_flags & IOC_AUTH_DEVICE_NR_4_BYTES);
    if (device_nr)
    {
        osal_int_to_str(nbuf, sizeof(nbuf), device_nr);
        os_strncat(user.user_name, nbuf, IOC_DEVICE_ID_SZ);
    }
    if (auth_flags & IOC_AUTH_UNIQUE_ID) {
        os_char unique_id_bin[OSAL_UNIQUE_ID_BIN_SZ];
        os_memcpy(unique_id_bin, p, OSAL_UNIQUE_ID_BIN_SZ);
        p += OSAL_UNIQUE_ID_BIN_SZ;
    }

    s = ioc_msg_getstr(user.network_name, IOC_NETWORK_NAME_SZ, &p);
    if (s) return OSAL_STATUS_FAILED;

    /* Get password and hash it
     */
/* #if OSAL_SECRET_SUPPORT
    s = ioc_msg_getstr(tmp_password, IOC_PASSWORD_SZ, &p);
    if (s) return s;
    if (tmp_password[0])
    {
        osal_hash_password(user.password, tmp_password, IOC_PASSWORD_SZ);
    }
    else
    {
        os_strncpy(user.password, tmp_password, IOC_PASSWORD_SZ);
    }
#endif
*/

    /* Check user authorization.
     */
/*    if (root->authorization_func &&
        (con->flags & (IOC_LISTENER|IOC_SECURE_CONNECTION))
         == (IOC_LISTENER|IOC_SECURE_CONNECTION))
    {
        ioc_release_allowed_networks(&con->allowed_networks);
        s = root->authorization_func(root, &con->allowed_networks,
            &user, con->parameters, root->authorization_context);
        if (s) return s;
    }
*/

    /** If we are automatically setting for a device (root network name is "*" or ""
     */
/*     if (!os_strcmp(root->network_name, osal_str_asterisk) || root->network_name[0] == '\0')
    {
        os_strncpy(root->network_name, user.network_name, IOC_NETWORK_NAME_SZ);
        ioc_set_network_name(root);
    }
   */

    return OSAL_COMPLETED;
}


#if 0
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
/**
****************************************************************************************************

  @brief Enable user authentication (set authentication callback function).
  @anchor ioc_enable_user_authentication

  The ioc_enable_user_authentication() function stores authentication function pointer for
  iocom library to use. This enables user/device authentication and authorization for
  the iocom.

  @param   root Pointer to iocom root object.
  @param   func Pointer to authentication function. Authentication function needs to check
           if user/device can connect and which IO networks it can access. Set ioc_authorize
           here to use default ioserver extension library authentication.
  @param   contect Pointer to pass to authentication callback. Can be used to pass any
           application data to the callback. Set OS_NULL if not needed.
  @return  None.

****************************************************************************************************
*/
void ioc_enable_user_authentication(
    struct iocRoot *root,
    ioc_authorize_user_func *func,
    void *context)
{
    root->authorization_func = func;
    root->authorization_context = context;
}


/**
****************************************************************************************************

  @brief Add a network to allowed networks structure
  @anchor ioc_add_allowed_network

  The ioc_add_allowed_network() function adds an allowed network to allowed networks structure

  @param   allowed_networks Allowed networks structure.
  @param   network_name Network name to add.
  @param   flags Flags (privileges, etc) to store for the network
  @return  None.

****************************************************************************************************
*/
void ioc_add_allowed_network(
    iocAllowedNetworkConf *allowed_networks,
    const os_char *network_name,
    os_ushort flags)
{
    iocAllowedNetwork *n;
    os_memsz needed, bytes;
    os_int count, i;

    /* If we already got this network, just "or" the flags in.
     */
    count  = allowed_networks->n_networs;
    for (i = 0; i < count; i++)
    {
        if (!os_strcmp(network_name, allowed_networks->network[i].network_name))
        {
            allowed_networks->network[i].flags |= flags;
            return;
        }
    }

    /* Allocate (more) memory if needed.
     */
    needed = (os_memsz)(count + 1) * sizeof(iocAllowedNetwork);
    if (needed > allowed_networks->bytes)
    {
        n = (iocAllowedNetwork*)os_malloc(needed, &bytes);
        if (n == OS_NULL) return;
        os_memclear(n, bytes);
        if (allowed_networks->network)
        {
            os_memcpy(n, allowed_networks->network,
                count * sizeof(iocAllowedNetwork));
            os_free(allowed_networks->network, allowed_networks->bytes);
        }
        allowed_networks->network = n;
        allowed_networks->bytes = bytes;
    }

    /* Store name and flags of the added network and increment number of networks.
     */
    n = allowed_networks->network + allowed_networks->n_networs++;
    os_strncpy(n->network_name, network_name, IOC_NETWORK_NAME_SZ);
    n->flags = flags;
}


/**
****************************************************************************************************

  @brief Release allowed networks structure set up by ioc_authorize_user_func()
  @anchor ioc_release_allowed_networks

  The ioc_release_allowed_networks() function frees memory reserved for allowed network
  array, allocated by the authentication function.

  @param   allowed_networks Allowed networks structure containing pointer to allocated data.
           after this call the structure is clear enough for reuse.
  @return  None.

****************************************************************************************************
*/
void ioc_release_allowed_networks(
    iocAllowedNetworkConf *allowed_networks)
{
    if (allowed_networks->network)
    {
        os_free(allowed_networks->network, allowed_networks->bytes);
        os_memclear(allowed_networks, sizeof(iocAllowedNetworkConf));
    }
}

/**
****************************************************************************************************

  @brief Check if network is authorized for a connection.
  @anchor ioc_is_network_authorized

  The ioc_is_network_authorized() function checks if network name given as argument is in list
  of allowed networks.

  @param   con Connection structure pointer.
  @param   flags Required privileges, 0 for normal user, IOC_AUTH_ADMINISTRATOR for administrator.
  @return  OS_TRUE if network is authorized to proceed, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean ioc_is_network_authorized(
    struct iocConnection *con,
    os_char *network_name,
    os_ushort flags)
{
#if IOC_RELAX_SECURITY
    return OS_TRUE;
#else
    iocAllowedNetwork *networks;
    os_int count, i;

    /* If security is not on, anything is fine.
     */
    if (con->link.root->authorization_func == OS_NULL) return OS_TRUE;
    if ((con->flags & (IOC_LISTENER|IOC_SECURE_CONNECTION))
         != (IOC_LISTENER|IOC_SECURE_CONNECTION)) return OS_TRUE;

    /* If we already got this network, just "or" the flags in.
     */
    networks = con->allowed_networks.network;
    count = con->allowed_networks.n_networs;
    for (i = 0; i < count; i++)
    {
        if (!os_strcmp(network_name, networks[i].network_name))
        {
            if (flags & IOC_AUTH_ADMINISTRATOR)
                return (networks[i].flags & IOC_AUTH_ADMINISTRATOR)
                    ? OS_TRUE : OS_FALSE;

            return OS_TRUE;
        }
    }
    return OS_FALSE;
#endif
}

#endif
#endif

#endif
