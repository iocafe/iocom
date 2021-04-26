/**

  @file    ioc_nickgen.c
  @brief   Nick name generator.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef IOC_NICKGEN_H_
#define IOC_NICKGEN_H_
#include "iocom.h"
#if IOC_NICKGEN_SUPPORT

/* Generate a nick name
 */
void ioc_generate_nickname(
    os_char *buf,
    os_memsz buf_sz);

#endif
#endif
