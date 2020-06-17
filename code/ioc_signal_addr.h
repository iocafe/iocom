/**

  @file    ioc_signal_addr.h
  @brief   Signal address related functions
  @author  Pekka Lehtikoski
  @version 1.0
  @date    16.6.2020

  Helper functions for implementing communication callback. These function check
  signal address

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

struct iocSignal;

/* Get number of bytes needed to store the signal.
 */
os_int ioc_nro_signal_bytes(
    const struct iocSignal *signal);

/* Check if memory address range belongs to signal.
 */
os_boolean ioc_is_my_address(
    const struct iocSignal *signal,
    os_int start_addr,
    os_int end_addr);

/* Which signals are effected by changes in memory address range.
 */
const iocSignal *ioc_get_signal_range(
    const iocMblkSignalHdr *hdr,
    os_int start_addr,
    os_int end_addr,
    os_int *n_signals);

/*@}*/
