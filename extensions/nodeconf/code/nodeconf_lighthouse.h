/**

  @file    nodeconf_lighthouse.h
  @brief   Get listening socket port number and transport.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Get listening socket port number and transport.
 */
osalStatus ioc_get_lighthouse_info(
    iocConnectionConfig *connconf,
    os_int *port,
    iocTransportEnum *transport);
