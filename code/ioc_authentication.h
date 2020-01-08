/**

  @file    ioc_authentication.h
  @brief   Device/user authentication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#if IOC_AUTHENTICATION_CODE

/**
****************************************************************************************************
  Flags in authentication frame.
****************************************************************************************************
*/
#define IOC_AUTH_CONNECT_UP 1
#define IOC_AUTH_DEVICE 2
#define IOC_AUTH_DEVICE_NR_2_BYTES 4
#define IOC_AUTH_DEVICE_NR_4_BYTES 8
#define IOC_AUTH_NETWORK_NAME 16
#define IOC_AUTH_USER_NAME 32
#define IOC_AUTH_PASSWORD 64
#define IOC_AUTH_BIDIRECTIONAL_COM 128

/**
****************************************************************************************************
  Authentication functions
****************************************************************************************************
 */
/*@{*/

/* Make authentication data frame.
 */
void ioc_make_authentication_frame(
    iocConnection *con);

/* Process received athentication data frame.
 */
osalStatus ioc_process_received_authentication_frame(
    struct iocConnection *con,
    os_uint mblk_id,
    os_char *data);

/*@}*/

#endif
