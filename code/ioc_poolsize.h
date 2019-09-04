/**

  @file    ioc_poolsize.h
  @brief   Macros for calculating pool size.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.7.2018

  If using static pool, the pool size must be calculated. If too small, program will not work.
  If too big, memory is wasted. The IOC_POOL_SIZE_LSOCK macro calculates pool size from number 
  of connections and size of memory blocks for sending and receiving data for an IO board listening
  to socket port. Memory needed for the iocMemoryBlock stucture for received and sent data is not 
  included, neither is memory for end point structure iocEndPoint (if listening for connections).

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#if 0
moved to ioboard.h
#define IOC_POOL_SIZE(MAX_CONNECTIONS, SEND_BLOCK_SZ, RECEIVE_BLOCK_SZ) \
    MAX_CONNECTIONS * sizeof(iocConnection) \
  + MAX_CONNECTIONS * 2*IOC_SOCKET_FRAME_SZ \
  + MAX_CONNECTIONS * sizeof(iocSourceBuffer) \
  + MAX_CONNECTIONS * SEND_BLOCK_SZ * sizeof(ioc_sbuf_item) \
  + MAX_CONNECTIONS * sizeof(iocTargetBuffer) \
  + MAX_CONNECTIONS * RECEIVE_BLOCK_SZ * sizeof(ioc_tbuf_item) \
  + sizeof(iocEndPoint) \
  + SEND_BLOCK_SZ \
  + RECEIVE_BLOCK_SZ
// DO WE NEED MAX_CONNECTIONS elements for both send and receive?
#endif
