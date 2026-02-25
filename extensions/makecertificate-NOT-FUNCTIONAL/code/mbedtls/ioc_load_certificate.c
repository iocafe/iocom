/**

  @file    ioc_load_certificate.c
  @brief   Load certificate from persistent storage.
  @version 1.0
  @date    26.2.2020

  Original copyright:

  Copyright The Mbed TLS Contributors
  SPDX-License-Identifier: Apache-2.0

  Licensed under the Apache License, Version 2.0 (the "License"); you may
  not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Adapted for use with iocom library 2021 by Pekka Lehtikoski. This file contains only code
  to create RSA keys.

****************************************************************************************************
*/
#include "makecertificate.h"
#if OSAL_TLS_SUPPORT==OSAL_TLS_MBED_WRAPPER


/**
****************************************************************************************************

  @brief Load certificate from persistent storage.
  @anchor ioc_load_certificate

  The ioc_load_certificate() function loads a certificate from persistent storage.

  @param   cert Context where to store the certificate, initialized by mbedtls_x509_crt_init(&cert)
           before calling this function. Needs to be released by mbedtls_x509_crt_free(&cert).
  @param   cert_type Persistent block number, for example OS_PBNR_SERVER_CERT or OS_PBNR_ROOT_CERT.

  @return  OSAL_SUCCESS if certificate was successfully loaded. Other return values
           indicate an error.

****************************************************************************************************
*/
osalStatus ioc_load_certificate(
    mbedtls_x509_crt *cert,
    osPersistentBlockNr cert_type)
{
    osalStatus s;
    os_char *block;
    os_memsz block_sz;
    int ret;

    /* Load certificate from persistent storage.
     */
    s = os_load_persistent_malloc(cert_type, &block, &block_sz);
    if (OSAL_IS_ERROR(s)) {
        return s;
    }

    /* Parse the certificate.
     */
    ret = mbedtls_x509_crt_parse(cert, (unsigned char*)block, block_sz);

    /* Release buffer after parsing, regardless if parsing was a success.
     */
    if (s == OSAL_MEMORY_ALLOCATED) {
        os_free(block, block_sz);
    }

    /* If parsing certificate failed, report error.
     */
    if (ret) {
        os_char buf[256];
        mbedtls_strerror(ret, buf, sizeof(buf));
        osal_debug_error_str("mbedtls_pk_parse_key failed: ", buf);
        return OSAL_STATUS_FAILED;
    }

    return OSAL_SUCCESS;
}

#endif
