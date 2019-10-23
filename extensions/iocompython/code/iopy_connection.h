/**

  @file    iopy_connection.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


/* Initialize connection object.
 */
iocConnection *ioc_initialize_connection(
    iocConnection *con,
    iocRoot *root);

/* Release connection object.
 */
void ioc_release_connection(
    iocConnection *con);

/* Start or prepare the connection.
 */
osalStatus ioc_connect(
    iocConnection *con,
    iocConnectionParams *prm);
