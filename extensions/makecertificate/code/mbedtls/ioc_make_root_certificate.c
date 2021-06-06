/**

  @file    ioc_make_root_certificate.c
  @brief   Make root certificate using MbedTLS.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.2.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "makecertificate.h"
#if OSAL_TLS_SUPPORT==OSAL_TLS_MBED_WRAPPER

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif



/**
****************************************************************************************************

  @brief X

  X

  @param   X

  @return  None.

****************************************************************************************************
*/
void ioc_make_root_certificate()
{
}

#endif
