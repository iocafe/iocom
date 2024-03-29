/**

  @file    ioc_generate_key.c
  @brief   Key generation.
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

#define DFL_TYPE                MBEDTLS_PK_RSA      // MBEDTLS_PK_RSA or MBEDTLS_PK_ECKEY
#define DFL_RSA_KEYSIZE         2048                // Key size, for example 1024, 2048 or 4096


#if !defined(MBEDTLS_PK_WRITE_C) || !defined(MBEDTLS_PEM_WRITE_C) || \
    !defined(MBEDTLS_FS_IO) || !defined(MBEDTLS_ENTROPY_C) || \
    !defined(MBEDTLS_CTR_DRBG_C)
#error "MBEDTLS_PK_WRITE_C and/or MBEDTLS_FS_IO and/or " \
            "MBEDTLS_ENTROPY_C and/or MBEDTLS_CTR_DRBG_C and/or " \
            "MBEDTLS_PEM_WRITE_C" \
            "not defined.\n" );
#endif


/**
****************************************************************************************************

  @brief Save RSA key to persistent storage.
  @anchor write_private_key

  Save RSA key into persisten block (can be a file, EEPROM, etc).

  @param   opt Pointer to "options" structure. The key_type member must be either
           OS_PBNR_SERVER_KEY or OS_PBNR_ROOT_KEY (persistent block number).
           If der_format is set, the key is saved in DER format. If not set, key is saved as PEM.

  @return  OSAL_SUCCESS if key was successfully generated and saved. Other return values
           indicate an error.

****************************************************************************************************
*/
static osalStatus write_private_key(
    mbedtls_pk_context *key,
    iocKeyOptions *opt)
{
    int ret;
    os_char *output_buf, *c;
    os_memsz output_buf_sz;
    size_t len;
    osalStatus s;

    /* Buffer is dynamically allocated to limit stack use (micro-controllers)
     */
    output_buf = os_malloc(3 * opt->rsa_keysize/2 + 200, &output_buf_sz);
    if (output_buf == OS_NULL) {
        return OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
    }

    os_memclear(output_buf, output_buf_sz);
    if (!opt->der_format)
    {
        ret = mbedtls_pk_write_key_pem(key, (unsigned char*)output_buf, output_buf_sz);
        if (ret) {
            goto failed;
        }

        /* Includes terminating '\0' character in length.
         */
        len = os_strlen(output_buf); // CHECK SHOULD TERMINATING '\0' CHARACTER BE SAVED? NOW SAVED.
    }
    else
    {
        ret = mbedtls_pk_write_key_der(key, (unsigned char*)output_buf, output_buf_sz);
        if (ret < 0) {
            goto failed;
        }

        len = ret;
        c = output_buf + output_buf_sz - len;
    }

    s = os_save_persistent(opt->key_type, c, len, OS_FALSE);
    os_free(output_buf, output_buf_sz);
    return s;

failed:
    os_memclear(output_buf, output_buf_sz);
    mbedtls_strerror(ret, output_buf, output_buf_sz);
    osal_debug_error_str("mbedtls_pk_write_key_? failed: ", output_buf);
    os_free(output_buf, output_buf_sz);
    return OSAL_STATUS_FAILED;
}


/**
****************************************************************************************************

  @brief Generate new RSA key.
  @anchor ioc_generate_key

  Generate RSA key and save it as persistent block.

  @param   popt Pointer to "options" structure for generating key. The key_type member, either
           OS_PBNR_SERVER_KEY or OS_PBNR_ROOT_KEY selects if new key is saved as root key
           or server key. If omitted defaults to OS_PBNR_SERVER_KEY.
           Member rsa_keysize specifies key size, default 2048.
           Set der_format to save the key in DER format. If not set, key is saved as PEM.
           PEM is more readable, DER is smaller.

  @return  OSAL_SUCCESS if key was successfully generated and saved. Other return values
           indicate an error.

****************************************************************************************************
*/
osalStatus ioc_generate_key(
    iocKeyOptions *popt)
{
    int ret = 1;
    osalStatus s = OSAL_STATUS_FAILED;
    mbedtls_pk_context key;
    iocKeyOptions opt;
    osalTLS *t;
    t = osal_global->tls;

    mbedtls_pk_init( &key );
    // mbedtls_ctr_drbg_init( &ctr_drbg );

    if (popt) {
        os_memcpy(&opt, popt, sizeof(opt));
    }
    else {
        os_memclear(&opt, sizeof(opt));
    }
    if (opt.key_type == 0) {
        opt.key_type = OS_PBNR_SERVER_KEY;
    }
    if (opt.rsa_keysize == 0) {
        opt.rsa_keysize = DFL_RSA_KEYSIZE;
    }

    /* Generate the key
     */
    osal_trace( "Generating the private RSA key" );

    ret = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(DFL_TYPE));
    if (ret) {
        osal_debug_error_int("generate_key failed! mbedtls_pk_setup:", ret);
        goto exit;
    }

    ret = mbedtls_rsa_gen_key(mbedtls_pk_rsa(key), mbedtls_ctr_drbg_random, &t->ctr_drbg,
        opt.rsa_keysize, 65537);
    if (ret) {
        osal_debug_error_int("generate_key failed! mbedtls_rsa_gen_key:", ret);
        goto exit;
    }

    /* Export key
     */
    osal_trace("Writing key to persistent storage");

    s = write_private_key(&key, &opt);
    if (s) {
        osal_trace("Failed");
        goto exit;
    }

    s = OSAL_SUCCESS;

exit:

#ifdef MBEDTLS_ERROR_C
    if (OSAL_IS_ERROR(s)) {
        os_char buf[1024];
        os_memclear(buf, sizeof(buf));
        mbedtls_strerror(ret, buf, sizeof(buf));
        osal_debug_error_str("generate_key failed: ", buf);
    }
#endif

    mbedtls_pk_free(&key);
    return s;
}

#endif
