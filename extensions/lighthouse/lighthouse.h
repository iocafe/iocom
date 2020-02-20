/**

  @file    lighthouse.h
  @brief   Service discovery using UDP multicasts.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.2.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef LIGHTHOUSE_INCLUDED
#define LIGHTHOUSE_INCLUDED

/* Include iocom and operating system abstraction layer.
 */
#include "iocom.h"

/* Lighthouse multicast IP and port
   239.0.0.0-239.255.255.255 Organization-Local Scope [David_Meyer][RFC2365]
 */
#ifndef LIGHTHOUSE_IP
#define LIGHTHOUSE_IP "239.63.68.63"
#endif
#ifndef LIGHTHOUSE_PORT
#define LIGHTHOUSE_PORT ":6368"
#endif

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Light house message ID
 */
#define LIGHTHOUSE_MSG_ID 0xB3

/* Light house UDP message header.
 */
typedef struct LighthouseMessageHdr
{
    os_uchar tstamp[8];

    os_uchar msg_id;
    os_uchar hdr_sz;
    os_uchar checksum_low;
    os_uchar checksum_high;

    os_uchar port_nr_low;
    os_uchar port_nr_high;
    os_uchar random_nr_low;
    os_uchar random_nr_high;

    os_uchar publish_sz;
    os_uchar transport;
    os_uchar reserved_1;
    os_uchar reserved_2;

    os_uchar reserved_3;
    os_uchar reserved_4;
    os_uchar reserved_5;
    os_uchar reserved_6;
}
LighthouseMessageHdr;

/* Maximum string data size
 */
#define LIGHTHOUSE_PUBLISH_SZ (24*5)

/* Light house UDP message.
 */
typedef struct LighthouseMessage
{
    LighthouseMessageHdr hdr;
    os_char publish[LIGHTHOUSE_PUBLISH_SZ];
}
LighthouseMessage;


#include "code/lighthouse_server.h"
#include "code/lighthouse_client.h"

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
