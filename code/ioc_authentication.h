/**

  @file    ioc_authentication.h
  @brief   Connection object.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.12.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#if IOC_AUTHENTICATION




#if IOC_AUTHENTICATION == IOC_FULL_AUTHENTICATION

/**
****************************************************************************************************
  Authentication data for one device.
****************************************************************************************************
*/
typedef struct iocSecureDevice
{
    /** Device name, max 15 characters.
     */
    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. 0 = any device number.
     */
    os_uint device_nr;

    /** Network name. Empty string = any network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** When automatic device numbering is used, the devices cannot be uniquely identified
     *  by device name, number and network name. In this case user name can be used to
     *  check if the connection is authenticated.
     */
    os_char user_name[IOC_NAME_SZ];

    /** Password for secure connections.
     */
    os_char password[IOC_NAME_SZ];

    /** Password for clear text authentication, to be passed trough socket, serial port
        or blue tooth. This is separate from passwork_tls so that the secure password
        is only passed trough encrypted connection.
     */
    // os_char password_clear[IOC_NAME_SZ];

}
iocSecureDevice;

typedef struct iocAuthenticatedDevices
{

    iocSecureDevice device[1];
}
iocAuthenticatedDevices;

#endif


/**
****************************************************************************************************
  Enumeration of fields in authentication frame.
****************************************************************************************************
*/
typedef enum iocAuthenticationItem
{
    IOC_AUTH_DEVICE_NAME,
    IOC_AUTH_DEVICE_NR,
    IOC_AUTH_NETWORK_NAME,
    IOC_AUTH_USER_NAME,
    IOC_AUTH_PASSWORD
}
iocAuthenticationItem;



/**
****************************************************************************************************
  Authentication functions
****************************************************************************************************
 */
/*@{*/

void ioc_make_authentication_frame(
    iocConnection *con);

/*@}*/

#endif
