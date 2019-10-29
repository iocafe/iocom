/**

  @file    ioc_compress.h
  @brief   Frame data compression and uncomression.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.8.2018

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#ifndef IOC_COMPRESS_INCLUDED
#define IOC_COMPRESS_INCLUDED




/** 
****************************************************************************************************

  @name Compression and uncomression functions

  The ioc_compress() function compressess data from source buffer into destination buffer.

****************************************************************************************************
 */
/*@{*/


/* Compress data.
 */
int ioc_compress(
    os_char *srcbuf,
    int *start_addr,
    int end_addr,
    os_char *dst,
    int dst_sz);


/* Uncompress data.
 */
int ioc_uncompress(
    os_char *src,
    int src_bytes,
    os_char *dst,
    int dst_sz,
    os_uchar flags);

/*@}*/

#endif
