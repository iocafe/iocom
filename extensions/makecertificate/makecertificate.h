/**

  @file    makecertificate.h
  @brief   Main header file fof the makecertificate library.
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
#ifndef MAKECERTIFICATE_H_
#define MAKECERTIFICATE_H_
#include "eosalx.h"

/* If C++ compilation, all functions, etc. from this point on included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

#include "code/common/ioc_make_certificate.h"

#if OSAL_TLS_SUPPORT==OSAL_TLS_MBED_WRAPPER
#include "code/mbedtls/ioc_mbedtls_certificate.h"
#endif

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
