/**

  @file    ioc_generate_key.c
  @brief   Key generation.
  @version 1.0
  @date    26.2.2020

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

  Adapted for use with iocom library 2021 by Pekka Lehtikoski. This file contains code to create
  RSA keys only.

****************************************************************************************************
*/
#include "makecertificate.h"
#if OSAL_TLS_SUPPORT==OSAL_TLS_MBED_WRAPPER

#include "extensions/tls/mbedtls/osal_mbedtls.h"


#define FORMAT_PEM              0
#define FORMAT_DER              1

#define DFL_TYPE                MBEDTLS_PK_RSA      // MBEDTLS_PK_RSA or MBEDTLS_PK_ECKEY
#define DFL_RSA_KEYSIZE         4096                // Key size bytes, for example 1024 or 4096
#define DFL_FILENAME            "keyfile.pem"
#define DFL_FORMAT              FORMAT_PEM          // FORMAT_PEM or FORMAT_DER
#define DFL_USE_DEV_RANDOM      0


/*
 * global options
 */
typedef struct iocKeyOptions
{
    int type;                   /* the type of key to generate          */
    int rsa_keysize;            /* length of key in bits                */
    const char *filename;       /* filename of the key file             */
    int format;                 /* the output format to use             */
    int use_dev_random;         /* use /dev/random as entropy source    */
} iocKeyOptions;


#if !defined(MBEDTLS_PK_WRITE_C) || !defined(MBEDTLS_PEM_WRITE_C) || \
    !defined(MBEDTLS_FS_IO) || !defined(MBEDTLS_ENTROPY_C) || \
    !defined(MBEDTLS_CTR_DRBG_C)
#error "MBEDTLS_PK_WRITE_C and/or MBEDTLS_FS_IO and/or " \
            "MBEDTLS_ENTROPY_C and/or MBEDTLS_CTR_DRBG_C and/or " \
            "MBEDTLS_PEM_WRITE_C" \
            "not defined.\n" );
#endif


static int write_private_key(
    mbedtls_pk_context *key,
    iocKeyOptions *opt)
{
    int ret;
    FILE *f;
    unsigned char output_buf[16000];
    unsigned char *c = output_buf;
    size_t len = 0;

    memset(output_buf, 0, 16000);
    if( opt->format == FORMAT_PEM )
    {
        if( ( ret = mbedtls_pk_write_key_pem( key, output_buf, 16000 ) ) != 0 )
            return( ret );

        len = strlen( (char *) output_buf );
    }
    else
    {
        if( ( ret = mbedtls_pk_write_key_der( key, output_buf, 16000 ) ) < 0 )
            return( ret );

        len = ret;
        c = output_buf + sizeof(output_buf) - len;
    }

    if( ( f = fopen(opt->filename, "wb" ) ) == NULL )
        return( -1 );

    if( fwrite( c, 1, len, f ) != len )
    {
        fclose( f );
        return( -1 );
    }

    fclose( f );

    return( 0 );
}


void ioc_generate_key(void)
{
    int ret = 1;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    mbedtls_pk_context key;
    char buf[1024];
    iocKeyOptions opt;
    osalTLS *t;
    t = osal_global->tls;

    mbedtls_pk_init( &key );
    // mbedtls_ctr_drbg_init( &ctr_drbg );
    memset( buf, 0, sizeof( buf ) );

    os_memclear(&opt, sizeof(opt));
    opt.type                = DFL_TYPE;
    opt.rsa_keysize         = DFL_RSA_KEYSIZE;
    opt.filename            = DFL_FILENAME;
    opt.format              = DFL_FORMAT;
    opt.use_dev_random      = DFL_USE_DEV_RANDOM;

    /* Generate the key
     */
    osal_trace( "\n  . Generating the private key ..." );
    fflush( stdout );

    ret = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type( (mbedtls_pk_type_t) opt.type));
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
    osal_trace( "  . Writing key to file..." );

    if( ( ret = write_private_key( &key, &opt) ) != 0 )
    {
        osal_trace( " failed\n" );
        goto exit;
    }

    osal_trace( " ok\n" );

    exit_code = MBEDTLS_EXIT_SUCCESS;

exit:

#ifdef MBEDTLS_ERROR_C
    if (exit_code != MBEDTLS_EXIT_SUCCESS)
    {
        mbedtls_strerror(ret, buf, sizeof(buf));
        osal_debug_error_str("generate_key failed: ", buf);
    }
#endif

    mbedtls_pk_free(&key);
}


#endif
