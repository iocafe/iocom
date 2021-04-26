/**

  @file    ioc_compress.h
  @brief   Frame data compression and uncomression.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_COMPRESS_H_
#define IOC_COMPRESS_H_
#include "iocom.h"

/** 
****************************************************************************************************

  @name Compression and uncomression functions

  The ioc_compress() function compressess data from source buffer into destination buffer.

****************************************************************************************************
 */
/*@{*/

/* Compress data.
 */
os_int ioc_compress(
    os_char *srcbuf,
    os_int *start_addr,
    os_int end_addr,
    os_char *dst,
    os_memsz dst_sz);

/* Uncompress data.
 */
os_int ioc_uncompress(
    os_char *src,
    os_int src_bytes,
    os_char *dst,
    os_memsz dst_sz,
    os_uchar flags);

/*@}*/

#endif
