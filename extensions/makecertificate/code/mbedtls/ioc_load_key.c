/**

  @file    ioc_load_key.c
  @brief   Load key from persistent storage.
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

  @brief Load RSA key from persistent storage.
  @anchor ioc_load_key

  The ioc_load_key() function loads RSA key from persistent storage. The key can be either DER
  or PEM format.

  @param   key Context where to store the key, initialized by mbedtls_pk_init(&key) beforehand.
           Needs to be released by mbedtls_pk_free(&key).
  @param   key_type Either OS_PBNR_SERVER_KEY or OS_PBNR_ROOT_KEY. Persistent block number.

  @return  OSAL_SUCCESS if key was successfully loaded. Other return values indicate an error.

****************************************************************************************************
*/
osalStatus ioc_load_key(
    mbedtls_pk_context *key,
    osPersistentBlockNr key_type)
{
    osalStatus s;
    os_char *block;
    os_memsz block_sz;
    int ret;

    /* Load key data from persistent storage.
     */
    s = os_load_persistent_malloc(key_type, &block, &block_sz);
    if (OSAL_IS_ERROR(s)) {
        return s;
    }

    /* Parse the key.
     */
    ret = mbedtls_pk_parse_key(key, (unsigned char*)block, block_sz, NULL, 0);

    /* Release buffer after parsing, regardless if parsing was a success.
     */
    if (s == OSAL_MEMORY_ALLOCATED) {
        os_free(block, block_sz);
    }

    /* If parsing key failed.
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
