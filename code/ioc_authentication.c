/**

  @file    ioc_authentication.c
  @brief   Device/user authentication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Low level of user authentication and authorization. Handles serialization of authentication
  frames over connection and on server (IOC_FULL_AUTHENTICATION) works as interface between
  iocom and authentication code. Notice that ioserver extension library contains default
  server authentication which should be sufficient for simpler applications.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_AUTHENTICATION_CODE


/**
****************************************************************************************************

  @brief Make authentication data frame.
  @anchor ioc_make_authentication_frame

  The ioc_make_authentication_frame() generates ourgoind data frame which contains information
  to authenticate this IO device, etc.

  @param   con Pointer to the connection object.
  @return  None.

****************************************************************************************************
*/
void ioc_make_authentication_frame(
    iocConnection *con)
{
    iocRoot
        *root;

    iocSendHeaderPtrs
        ptrs;

    os_uchar
        *p,
        *auth_flags_ptr,
        flags,
        *start;

    os_char
        *network_name,
        *user_name,
        user_name_buf[IOC_NAME_SZ],
        *password,
        *q;

    os_int
        content_bytes,
        used_bytes;

    os_int
        bytes,
        device_nr;

    os_ushort
        crc,
        u;

    root = con->link.root;

    /* Set frame header.
     */
    ioc_generate_header(con, con->frame_out.buf, &ptrs, 0, 0);

    /* Generate frame content. Here we do not check for buffer overflow,
       we know (and trust) that it fits within one frame.
     */
    p = start = (os_uchar*)con->frame_out.buf + ptrs.header_sz;
    *(p++) = IOC_AUTHENTICATION_DATA;
    flags = 0;
    auth_flags_ptr = p;
    *(p++) = 0;

    network_name = root->network_name;
    user_name = root->device_name;
    device_nr = root->device_nr;

    /* If we have user name, we use it instead of device name. User name
       may have also network name, like ispy.iocafenet.
     */
    if (con->user_override[0] != '\0')
    {
        os_strncpy(user_name_buf, con->user_override, sizeof(user_name_buf));
        q = os_strchr(user_name_buf, '.');
        if (q) *q = '\0';
        device_nr = 0;
        q = os_strchr(con->user_override, '.');
        if (q) network_name = q + 1;
    }

    ioc_msg_setstr(user_name, &p);
    ioc_msg_set_uint(device_nr < IOC_AUTO_DEVICE_NR ? device_nr : 0,
        &p, &flags, IOC_AUTH_DEVICE_NR_2_BYTES, &flags, IOC_AUTH_DEVICE_NR_4_BYTES);
    ioc_msg_setstr(network_name, &p);

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

    ioc_msg_setstr(password, &p);

    /* If other end has not acknowledged enough data to send the
       frame, cancel the send.
     */
    content_bytes = (os_int)(p - start);
    used_bytes = content_bytes + ptrs.header_sz;
    u = con->bytes_sent - con->processed_bytes;
    bytes = con->max_in_air - (os_int)u;
    if (used_bytes > bytes)
    {
        osal_trace2_int("Authentication canceled by flow control, free space on air=", bytes);
        return;
    }

    /* Set connect up and bidirectional flags.
     */
    if (con->flags & IOC_CONNECT_UP)
    {
        flags |= IOC_AUTH_CONNECT_UP;
    }
#if IOC_BIDIRECTIONAL_MBLK_CODE
    if (con->flags & IOC_BIDIRECTIONAL_MBLKS)
    {
        flags |= IOC_AUTH_BIDIRECTIONAL_COM;
    }
#endif

    /* Fill in data size, flag as system frame, and flags for authentication data.
     */
    *ptrs.data_sz_low = (os_uchar)content_bytes;
    if (ptrs.data_sz_high)
    {
        content_bytes >>= 8;
        *ptrs.data_sz_high = (os_uchar)content_bytes;
    }
    con->frame_out.used = used_bytes;
    *ptrs.flags |= IOC_SYSTEM_FRAME;
    *auth_flags_ptr = flags;

    /* Frame not rejected by flow control, increment frame number.
     */
    if (con->frame_out.frame_nr++ >= IOC_MAX_FRAME_NR)
    {
        con->frame_out.frame_nr = 1;
    }

    /* Calculate check sum out of whole used frame buffer. Notice that check sum
     * position within frame is zeros when calculating the check sum.
     */
    if (ptrs.checksum_low)
    {
        crc = os_checksum(con->frame_out.buf, con->frame_out.used, OS_NULL);
        *ptrs.checksum_low = (os_uchar)crc;
        *ptrs.checksum_high = (os_uchar)(crc >> 8);
    }

    con->authentication_sent = OS_TRUE;
osal_debug_error("HERE AUTH SENT");
}


/**
****************************************************************************************************

  @brief Process complete athentication data frame received from socket or serial port.
  @anchor ioc_process_received_authentication_frame

  The ioc_process_received_authentication_frame() function is called once a complete system frame
  containing authentication data for a device has been received. The authentication data
  identifies the device (device name, number and network name), optionally identifies the user
  with user name and can have password for the connection.
  If user authentication is enabled (by ioc_enable_user_authentication() function), the
  user is authenticated.

  The secondary task of authentication frame is to inform server side of the accepted connection
  is upwards or downwards in IO device hierarchy.

  @param   con Pointer to the connection object.
  @param   mblk_id Memory block identifier in this end.
  @param   data Received data, can be compressed and delta encoded, check flags.

  @return  OSAL_SUCCESS if succesfull. Other values indicate unauthenticated device or user,
           or a corrupted frame.

****************************************************************************************************
*/
osalStatus ioc_process_received_authentication_frame(
    struct iocConnection *con,
    os_uint mblk_id,
    os_char *data)
{
#if IOC_AUTHENTICATION_CODE == IOC_FULL_AUTHENTICATION
    iocUser user;
    iocRoot *root;
    os_uint device_nr;
    os_uchar auth_flags, *p;
    os_char nbuf[OSAL_NBUF_SZ];
    osalStatus s;

    p = (os_uchar*)data + 1; /* Skip system frame IOC_SYSRAME_MBLK_INFO byte. */
    auth_flags = (os_uchar)*(p++);

    os_memclear(&user, sizeof(user));
    user.flags = auth_flags;

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

#if IOC_BIDIRECTIONAL_MBLK_CODE
        if (auth_flags & IOC_AUTH_BIDIRECTIONAL_COM)
        {
            con->flags |= IOC_BIDIRECTIONAL_MBLKS;
        }
        else
        {
            con->flags &= ~IOC_BIDIRECTIONAL_MBLKS;
        }
#endif
    }

    s = ioc_msg_getstr(user.user_name, IOC_DEVICE_ID_SZ, &p);
    if (s) return s;

    device_nr = ioc_msg_get_uint(&p,
        auth_flags & IOC_AUTH_DEVICE_NR_2_BYTES,
        auth_flags & IOC_AUTH_DEVICE_NR_4_BYTES);
    if (device_nr)
    {
        osal_int_to_str(nbuf, sizeof(nbuf), device_nr);
        os_strncat(user.user_name, nbuf, IOC_DEVICE_ID_SZ);
    }

    s = ioc_msg_getstr(user.network_name, IOC_NETWORK_NAME_SZ, &p);
    if (s) return s;

    s = ioc_msg_getstr(user.password, IOC_PASSWORD_SZ, &p);
    if (s) return s;

    /* Check user autorization.
     */
    root = con->link.root;
    if (root->authorization_func)
    {
        ioc_release_allowed_networks(&con->allowed_networks);
        s = root->authorization_func(root, &con->allowed_networks,
            &user, OS_NULL /* IP address */, &root->authorization_context);
        if (s) return s;
    }

#endif
osal_debug_error("HERE AUTH RECEIVED");
    con->authentication_received = OS_TRUE;
    return OSAL_SUCCESS;
}


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
    os_char *network_name,
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
    needed = (count + 1) * sizeof(iocAllowedNetwork);
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

  @brief Check if network is authorized.
  @anchor ioc_is_network_authorized

  The ioc_is_network_authorized() function checks if network name given as argument is in list
  of allowed networks.

  @param   allowed_networks Allowed networks structure.
  @param   network_name Network name to check.
  @param   flags Required privileges, 0 for normal user, IOC_AUTH_ADMINISTRATOR for administrarot.
  @return  OS_TRUE if network is authorized to proceed, OS_FALSE if not.

****************************************************************************************************
*/
os_boolean ioc_is_network_authorized(
    struct iocRoot *root,
    iocAllowedNetworkConf *allowed_networks,
    os_char *network_name,
    os_ushort flags)
{
#if IOC_RELAX_SECURITY
    return OS_TRUE;
#else
    os_int count, i;

    /* If security is not on, anything is fine.
     */
    if (root->authorization_func == OS_NULL) return OS_TRUE;

    /* If we already got this network, just "or" the flags in.
     */
    count  = allowed_networks->n_networs;
    for (i = 0; i < count; i++)
    {
        if (!os_strcmp(network_name, allowed_networks->network[i].network_name))
        {
            if (flags & IOC_AUTH_ADMINISTRATOR)
                return (allowed_networks->network[i].flags & IOC_AUTH_ADMINISTRATOR) ? OS_TRUE : OS_FALSE;

            return OS_TRUE;
        }
    }
    return OS_FALSE;
#endif
}

#endif

#endif
