/**

  @file    ioc_timing.h
  @brief   Timing default defines.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  This file defines default timing constants. These defines may be overridden in compiler
  settings. Customize the timeouts only if you have a good reason to do so, bad timing parameters
  may lead to inefficient, unrealiable or disfunctional communication. Timing is especially
  important for the serial communication.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_TIMING_H_
#define IOC_TIMING_H_
#include "iocom.h"

/* Long timeouts for tracing test. We cannot run in normal speed and print messages.
 */
#if OSAL_TRACE >= 3
#define IOC_SOCKET_KEEPALIVE_MS 180000
#define IOC_SOCKET_SILENCE_MS 3600000
#define IOC_SERIAL_KEEPALIVE_MS 120000
#define IOC_SERIAL_SILENCE_MS 3600000
#endif

/* Period how often to break select to check for timeouts, etc.
 */
#ifndef IOC_SOCKET_CHECK_TIMEOUTS_MS
#define IOC_SOCKET_CHECK_TIMEOUTS_MS 200
#endif

/* How often to send a keep-alive message if there is nothing else to send.
 */
#ifndef IOC_SOCKET_KEEPALIVE_MS
#define IOC_SOCKET_KEEPALIVE_MS 10000
#endif

/* How long silence (nothing received) on the line indicates broken connection.
 */
#ifndef IOC_SOCKET_SILENCE_MS
#define IOC_SOCKET_SILENCE_MS 20000
#endif

/* How often to send CONNECT character while establishing serial connection.
 */
#ifndef IOC_SERIAL_CONNECT_PERIOD_MS 
#define IOC_SERIAL_CONNECT_PERIOD_MS 300
#endif

/* Period how often to break select to check for timeouts, etc.
 */
#ifndef IOC_SERIAL_CHECK_TIMEOUTS_MS
#define IOC_SERIAL_CHECK_TIMEOUTS_MS 50
#endif

/* How often to send a keep-alive message if there is nothing else to send.
 */
#ifndef IOC_SERIAL_KEEPALIVE_MS
#define IOC_SERIAL_KEEPALIVE_MS 150
#endif

/* How long silence (nothing received) on the line indicates broken connection.
 */
#ifndef IOC_SERIAL_SILENCE_MS
#define IOC_SERIAL_SILENCE_MS 250
#endif

#endif
