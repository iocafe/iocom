/**

  @file    ioc_mbedtls_certificate.h
  @brief   Header for MbedTLS specific functions.
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
#ifndef MBEDTLS_CERTIFICATE_H_
#define MBEDTLS_CERTIFICATE_H_
#include "makecertificate.h"
#include "extensions/tls/mbedtls/osal_mbedtls.h"

/* Load RSA key from persistent storage.
 */
osalStatus ioc_load_key(
    mbedtls_pk_context *key,
    osPersistentBlockNr key_type);

/* Load certificate from persistent storage.
 */
osalStatus ioc_load_certificate(
    mbedtls_x509_crt *cert,
    osPersistentBlockNr cert_type);

#endif
