/**

  @file    iocom/applications/iodomainctrl/iodomainctrl.h
  @brief   Socket server example.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.9.2019

  Basic IO domain controller application.

  Copyright 2012 - 2019 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IODOMAINCTRL_INCLUDED
#define IODOMAINCTRL_INCLUDED

/* Include iotopology and iodomain headers. This further includes iocom and eosal.
 */
#include "extensions/iotopology/iotopology.h"
#include "extensions/iodomain/iodomain.h"

/* String buffer sizes.
 */
#define IODOMAIN_PORT_SZ 8
#define IODOMAIN_SERIAL_PRM_SZ 64
#define IODOMAIN_PATH_SZ 64

/* Which communications we do listen to?
 */
extern os_boolean iodomain_listen_tls;
extern os_boolean iodomain_listen_plain_tcp;
extern os_boolean iodomain_listen_serial;

/* Communication ports.
 */
extern os_char iodomain_tls_port[IODOMAIN_PORT_SZ];
extern os_char iodomain_tcp_port[IODOMAIN_PORT_SZ];
extern os_char iodomain_serial_prm[IODOMAIN_SERIAL_PRM_SZ];

/* TLS specific.
   iodomain_server_cert: Path to server certificate file.
   iodomain_server_key: Path to server key file.
 */
extern os_char iodomain_server_cert[IODOMAIN_PATH_SZ];
extern os_char iodomain_server_key[IODOMAIN_PATH_SZ];


#endif
