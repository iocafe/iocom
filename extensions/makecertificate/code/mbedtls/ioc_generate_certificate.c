/**

  @file    ioc_generate_key.c
  @brief   Certificate generation and signing
  @version 1.0
  @date    26.2.2020

  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
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

  Adapted for use with iocom library 2021 by Pekka Lehtikoski.

****************************************************************************************************
*/
#include "makecertificate.h"
#if OSAL_TLS_SUPPORT==OSAL_TLS_MBED_WRAPPER

#include "mbedtls/x509_crt.h"
#include "mbedtls/x509_csr.h"
#include "mbedtls/md.h"


#if !defined(MBEDTLS_X509_CRT_WRITE_C) || !defined(MBEDTLS_X509_CRT_PARSE_C) || \
    !defined(MBEDTLS_PK_WRITE_C) || !defined(MBEDTLS_PEM_WRITE_C) || \
    !defined(MBEDTLS_FS_IO) || !defined(MBEDTLS_ENTROPY_C) || \
    !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_ENTROPY_C) || \
    !defined(MBEDTLS_ERROR_C) || !defined(MBEDTLS_SHA256_C)
#error "MBEDTLS_* defines missing."
#endif



#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf          printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif /* MBEDTLS_PLATFORM_C */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(MBEDTLS_X509_CSR_PARSE_C)
#define USAGE_CSR                                                           \
    "    request_file=%%s         default: (empty)\n"                           \
    "                            If request_file is specified, subject_key,\n"  \
    "                            subject_pwd and subject_name are ignored!\n"
#else
#define USAGE_CSR ""
#endif /* MBEDTLS_X509_CSR_PARSE_C */

#define DFL_ISSUER_CRT          ""
#define DFL_REQUEST_FILE        ""
#define DFL_ISSUER_KEY          "ca.key"
#define DFL_SUBJECT_PWD         ""
#define DFL_ISSUER_PWD          ""
#define DFL_OUTPUT_FILENAME     "cert.crt"
// #define DFL_SUBJECT_NAME        "CN=Cert,O=mbed TLS,C=UK"
// #define DFL_ISSUER_NAME         "CN=CA,O=mbed TLS,C=UK"
#define DFL_NOT_BEFORE          "20210101000000"
#define DFL_NOT_AFTER           "22501231235959"
#define DFL_SERIAL              "1"
#define DFL_MAX_PATHLEN         -1
#define DFL_KEY_USAGE           0
#define DFL_NS_CERT_TYPE        0
#define DFL_VERSION             3
#define DFL_AUTH_IDENT          1
#define DFL_SUBJ_IDENT          1
#define DFL_CONSTRAINTS         1
#define DFL_DIGEST              MBEDTLS_MD_SHA256

#define USAGE \
    "\n usage: cert_write param=<>...\n"                \
    "\n acceptable parameters:\n"                       \
    USAGE_CSR                                           \
    "    subject_key=%%s          default: subject.key\n"   \
    "    subject_pwd=%%s          default: (empty)\n"       \
    "    subject_name=%%s         default: CN=Cert,O=mbed TLS,C=UK\n"   \
    "\n"                                                \
    "    issuer_crt=%%s           default: (empty)\n"       \
    "                            If issuer_crt is specified, issuer_name is\n"  \
    "                            ignored!\n"                \
    "    issuer_name=%%s          default: CN=CA,O=mbed TLS,C=UK\n"     \
    "\n"                                                \
    "    selfsign=%%d             default: 0 (false)\n"     \
    "                            If selfsign is enabled, issuer_name and\n" \
    "                            issuer_key are required (issuer_crt and\n" \
    "                            subject_* are ignored\n"   \
    "    issuer_key=%%s           default: ca.key\n"        \
    "    issuer_pwd=%%s           default: (empty)\n"       \
    "    output_file=%%s          default: cert.crt\n"      \
    "    serial=%%s               default: 1\n"             \
    "    not_before=%%s           default: 20010101000000\n"\
    "    not_after=%%s            default: 20301231235959\n"\
    "    is_ca=%%d                default: 0 (disabled)\n"  \
    "    max_pathlen=%%d          default: -1 (none)\n"     \
    "    md=%%s                   default: SHA256\n"        \
    "                            Supported values:\n"       \
    "                            MD2, MD4, MD5, SHA1, SHA256, SHA512\n"\
    "    version=%%d              default: 3\n"            \
    "                            Possible values: 1, 2, 3\n"\
    "    subject_identifier=%%s   default: 1\n"             \
    "                            Possible values: 0, 1\n"   \
    "                            (Considered for v3 only)\n"\
    "    authority_identifier=%%s default: 1\n"             \
    "                            Possible values: 0, 1\n"   \
    "                            (Considered for v3 only)\n"\
    "    basic_constraints=%%d    default: 1\n"             \
    "                            Possible values: 0, 1\n"   \
    "                            (Considered for v3 only)\n"\
    "    key_usage=%%s            default: (empty)\n"       \
    "                            Comma-separated-list of values:\n"     \
    "                            digital_signature\n"     \
    "                            non_repudiation\n"       \
    "                            key_encipherment\n"      \
    "                            data_encipherment\n"     \
    "                            key_agreement\n"         \
    "                            key_cert_sign\n"  \
    "                            crl_sign\n"              \
    "                            (Considered for v3 only)\n"\
    "    ns_cert_type=%%s         default: (empty)\n"       \
    "                            Comma-separated-list of values:\n"     \
    "                            ssl_client\n"            \
    "                            ssl_server\n"            \
    "                            email\n"                 \
    "                            object_signing\n"        \
    "                            ssl_ca\n"                \
    "                            email_ca\n"              \
    "                            object_signing_ca\n"     \
    "\n"



osalStatus ioc_write_certificate(
    mbedtls_x509write_cert *crt, // const char *output_file,
    iocCertificateOptions *opt,
    int (*f_rng)(void *, unsigned char *, size_t),
    void *p_rng)
{
    int ret;
    const os_memsz output_buf_sz = 4096;
    os_char *output_buf;
    size_t len;
    osalStatus s;

    output_buf = os_malloc(output_buf_sz, OS_NULL);
    os_memclear(output_buf, output_buf_sz);
    ret = mbedtls_x509write_crt_pem(crt, (unsigned char*)output_buf, output_buf_sz, f_rng, p_rng);
    if (ret < 0) {
        goto failed;
    }

    /* Includes terminating '\0' character in length.
     */
    len = os_strlen(output_buf); // CHECK SHOULD TERMINATING '\0' CHARACTER BE SAVED? NOW SAVED.

    s = os_save_persistent(opt->cert_type, output_buf, len, OS_FALSE);
    os_free(output_buf, output_buf_sz);
    return s;

failed:
    os_memclear(output_buf, output_buf_sz);
    mbedtls_strerror(ret, output_buf, output_buf_sz);
    osal_debug_error_str("mbedtls_x509write_crt_ failed: ", output_buf);
    os_free(output_buf, output_buf_sz);
    return OSAL_STATUS_FAILED;
}

// Generate a certificate
osalStatus ioc_generate_certificate(
    iocCertificateOptions *popt)
{
    int ret = 1;
    osalStatus exit_code = OSAL_STATUS_FAILED;
    mbedtls_x509_crt issuer_crt;
    mbedtls_pk_context loaded_issuer_key, loaded_subject_key;
    mbedtls_pk_context *issuer_key = &loaded_issuer_key,
                *subject_key = &loaded_subject_key;
    char buf[1024];
    char issuer_name[256];
    char subject_name_buf[128];
    os_char nbuf[OSAL_NBUF_SZ];
    iocCertificateOptions opt;
    mbedtls_md_type_t opt_md;       /* Hash used for signing                */

#if defined(MBEDTLS_X509_CSR_PARSE_C)
    char subject_name[256];
    mbedtls_x509_csr csr;
#endif
    mbedtls_x509write_cert crt;
    mbedtls_mpi serial;
    osalStatus s;

    osalTLS *t;
    t = osal_global->tls;

    /*
     * Set to sane values
     */
    mbedtls_x509write_crt_init( &crt );
    mbedtls_pk_init( &loaded_issuer_key );
    mbedtls_pk_init( &loaded_subject_key );
    mbedtls_mpi_init( &serial );
#if defined(MBEDTLS_X509_CSR_PARSE_C)
    mbedtls_x509_csr_init( &csr );
#endif
    mbedtls_x509_crt_init( &issuer_crt );
    memset( buf, 0, 1024 );


    os_memcpy(&opt, popt, sizeof(opt));
    os_memclear(&opt_md, sizeof(opt_md));
    if (opt.issuer_key_type == 0) opt.issuer_key_type = OS_PBNR_ROOT_KEY;
    if (opt.issuer_crt == OS_NULL) opt.issuer_crt      = DFL_ISSUER_CRT;
    if (opt.request_file == OS_NULL) opt.request_file  = DFL_REQUEST_FILE;
    if (opt.subject_key_type == OS_NULL) opt.subject_key_type = OS_PBNR_SERVER_KEY;
    if (opt.issuer_key == OS_NULL) opt.issuer_key      = DFL_ISSUER_KEY;
    if (opt.subject_pwd == OS_NULL) opt.subject_pwd    = DFL_SUBJECT_PWD;
    if (opt.issuer_pwd == OS_NULL) opt.issuer_pwd      = DFL_ISSUER_PWD;
    if (opt.output_file == OS_NULL) opt.output_file    = DFL_OUTPUT_FILENAME;

    if (opt.subject_name == OS_NULL)
    {
        os_strncpy(subject_name_buf, "CN=", sizeof(subject_name_buf));
        os_strncat(subject_name_buf, opt.process_name ? opt.process_name : "*", sizeof(subject_name_buf));
        if (opt.process_nr) {
            osal_int_to_str(nbuf, sizeof(nbuf), opt.process_nr);
            os_strncat(subject_name_buf, nbuf, sizeof(subject_name_buf));
        }
        if (opt.network_name) {
            os_strncat(subject_name_buf, ".", sizeof(subject_name_buf));
            os_strncat(subject_name_buf, opt.network_name, sizeof(subject_name_buf));
        }
        opt.subject_name = subject_name_buf;

#ifdef OSAL_TLS_ORGANIZATION
        os_strncat(subject_name_buf, ",O=" OSAL_TLS_ORGANIZATION, sizeof(subject_name_buf));
#else
        os_strncat(subject_name_buf, ",O=iocafe", sizeof(subject_name_buf));
#endif

#ifdef OSAL_TLS_COUNTRY
        os_strncat(subject_name_buf, ",C=" OSAL_TLS_COUNTRY, sizeof(subject_name_buf));
#else
        os_strncat(subject_name_buf, ",C=FJ", sizeof(subject_name_buf));
#endif
    }

    if (opt.issuer_name == OS_NULL) {
        opt.issuer_name = opt.subject_name;
    }

    if (opt.not_before == OS_NULL) opt.not_before      = DFL_NOT_BEFORE;
    if (opt.not_after == OS_NULL) opt.not_after        = DFL_NOT_AFTER;
    if (opt.serial == OS_NULL) opt.serial              = DFL_SERIAL;
    opt.max_pathlen         = DFL_MAX_PATHLEN;
    opt.key_usage           = DFL_KEY_USAGE;
    opt.ns_cert_type        = DFL_NS_CERT_TYPE;
    opt.version             = DFL_VERSION - 1;
    opt_md                  = DFL_DIGEST;
    opt.subject_identifier   = DFL_SUBJ_IDENT;
    opt.authority_identifier = DFL_AUTH_IDENT;
    opt.basic_constraints    = DFL_CONSTRAINTS;

#if 0
    for( i = 1; i < argc; i++ )
    {

        p = argv[i];
        if( ( q = strchr( p, '=' ) ) == NULL )
            goto usage;
        *q++ = '\0';

        if( strcmp( p, "request_file" ) == 0 )
            opt.request_file = q;
        else if( strcmp( p, "subject_key" ) == 0 )
            opt.subject_key = q;
        else if( strcmp( p, "issuer_key" ) == 0 )
            opt.issuer_key = q;
        else if( strcmp( p, "subject_pwd" ) == 0 )
            opt.subject_pwd = q;
        else if( strcmp( p, "issuer_pwd" ) == 0 )
            opt.issuer_pwd = q;
        else if( strcmp( p, "issuer_crt" ) == 0 )
            opt.issuer_crt = q;
        else if( strcmp( p, "output_file" ) == 0 )
            opt.output_file = q;
        else if( strcmp( p, "subject_name" ) == 0 )
        {
            opt.subject_name = q;
        }
        else if( strcmp( p, "issuer_name" ) == 0 )
        {
            opt.issuer_name = q;
        }
        else if( strcmp( p, "not_before" ) == 0 )
        {
            opt.not_before = q;
        }
        else if( strcmp( p, "not_after" ) == 0 )
        {
            opt.not_after = q;
        }
        else if( strcmp( p, "serial" ) == 0 )
        {
            opt.serial = q;
        }
        else if( strcmp( p, "authority_identifier" ) == 0 )
        {
            opt.authority_identifier = atoi( q );
            if( opt.authority_identifier != 0 &&
                opt.authority_identifier != 1 )
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
        }
        else if( strcmp( p, "subject_identifier" ) == 0 )
        {
            opt.subject_identifier = atoi( q );
            if( opt.subject_identifier != 0 &&
                opt.subject_identifier != 1 )
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
        }
        else if( strcmp( p, "basic_constraints" ) == 0 )
        {
            opt.basic_constraints = atoi( q );
            if( opt.basic_constraints != 0 &&
                opt.basic_constraints != 1 )
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
        }
        else if( strcmp( p, "md" ) == 0 )
        {
            if( strcmp( q, "SHA1" ) == 0 )
                opt.md = MBEDTLS_MD_SHA1;
            else if( strcmp( q, "SHA256" ) == 0 )
                opt.md = MBEDTLS_MD_SHA256;
            else if( strcmp( q, "SHA512" ) == 0 )
                opt.md = MBEDTLS_MD_SHA512;
            else if( strcmp( q, "MD2" ) == 0 )
                opt.md = MBEDTLS_MD_MD2;
            else if( strcmp( q, "MD4" ) == 0 )
                opt.md = MBEDTLS_MD_MD4;
            else if( strcmp( q, "MD5" ) == 0 )
                opt.md = MBEDTLS_MD_MD5;
            else
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
        }
        else if( strcmp( p, "version" ) == 0 )
        {
            opt.version = atoi( q );
            if( opt.version < 1 || opt.version > 3 )
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
            opt.version--;
        }
        else if( strcmp( p, "selfsign" ) == 0 )
        {
            opt.selfsign = atoi( q );
            if( opt.selfsign < 0 || opt.selfsign > 1 )
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
        }
        else if( strcmp( p, "is_ca" ) == 0 )
        {
            opt.is_ca = atoi( q );
            if( opt.is_ca < 0 || opt.is_ca > 1 )
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
        }
        else if( strcmp( p, "max_pathlen" ) == 0 )
        {
            opt.max_pathlen = atoi( q );
            if( opt.max_pathlen < -1 || opt.max_pathlen > 127 )
            {
                mbedtls_printf( "Invalid argument for option %s\n", p );
                goto usage;
            }
        }
        else if( strcmp( p, "key_usage" ) == 0 )
        {
            while( q != NULL )
            {
                if( ( r = strchr( q, ',' ) ) != NULL )
                    *r++ = '\0';

                if( strcmp( q, "digital_signature" ) == 0 )
                    opt.key_usage |= MBEDTLS_X509_KU_DIGITAL_SIGNATURE;
                else if( strcmp( q, "non_repudiation" ) == 0 )
                    opt.key_usage |= MBEDTLS_X509_KU_NON_REPUDIATION;
                else if( strcmp( q, "key_encipherment" ) == 0 )
                    opt.key_usage |= MBEDTLS_X509_KU_KEY_ENCIPHERMENT;
                else if( strcmp( q, "data_encipherment" ) == 0 )
                    opt.key_usage |= MBEDTLS_X509_KU_DATA_ENCIPHERMENT;
                else if( strcmp( q, "key_agreement" ) == 0 )
                    opt.key_usage |= MBEDTLS_X509_KU_KEY_AGREEMENT;
                else if( strcmp( q, "key_cert_sign" ) == 0 )
                    opt.key_usage |= MBEDTLS_X509_KU_KEY_CERT_SIGN;
                else if( strcmp( q, "crl_sign" ) == 0 )
                    opt.key_usage |= MBEDTLS_X509_KU_CRL_SIGN;
                else
                {
                    mbedtls_printf( "Invalid argument for option %s\n", p );
                    goto usage;
                }

                q = r;
            }
        }
        else if( strcmp( p, "ns_cert_type" ) == 0 )
        {
            while( q != NULL )
            {
                if( ( r = strchr( q, ',' ) ) != NULL )
                    *r++ = '\0';

                if( strcmp( q, "ssl_client" ) == 0 )
                    opt.ns_cert_type |= MBEDTLS_X509_NS_CERT_TYPE_SSL_CLIENT;
                else if( strcmp( q, "ssl_server" ) == 0 )
                    opt.ns_cert_type |= MBEDTLS_X509_NS_CERT_TYPE_SSL_SERVER;
                else if( strcmp( q, "email" ) == 0 )
                    opt.ns_cert_type |= MBEDTLS_X509_NS_CERT_TYPE_EMAIL;
                else if( strcmp( q, "object_signing" ) == 0 )
                    opt.ns_cert_type |= MBEDTLS_X509_NS_CERT_TYPE_OBJECT_SIGNING;
                else if( strcmp( q, "ssl_ca" ) == 0 )
                    opt.ns_cert_type |= MBEDTLS_X509_NS_CERT_TYPE_SSL_CA;
                else if( strcmp( q, "email_ca" ) == 0 )
                    opt.ns_cert_type |= MBEDTLS_X509_NS_CERT_TYPE_EMAIL_CA;
                else if( strcmp( q, "object_signing_ca" ) == 0 )
                    opt.ns_cert_type |= MBEDTLS_X509_NS_CERT_TYPE_OBJECT_SIGNING_CA;
                else
                {
                    mbedtls_printf( "Invalid argument for option %s\n", p );
                    goto usage;
                }

                q = r;
            }
        }
        else
            goto usage;
    }

    mbedtls_printf("\n");
#endif


    mbedtls_printf( " ok\n" );

    // Parse serial to MPI
    //
    mbedtls_printf( "  . Reading serial number..." );

    if( ( ret = mbedtls_mpi_read_string( &serial, 10, opt.serial ) ) != 0 )
    {
        mbedtls_strerror(ret, buf, sizeof(buf) );
        mbedtls_printf( " failed\n  !  mbedtls_mpi_read_string "
                        "returned -0x%04x - %s\n\n", -ret, buf );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    // Parse issuer certificate if present
    //
    if( !opt.selfsign && strlen( opt.issuer_crt ) )
    {
        /*
         * 1.0.a. Load the certificates
         */
        mbedtls_printf( "  . Loading the issuer certificate ..." );

        if( ( ret = mbedtls_x509_crt_parse_file( &issuer_crt, opt.issuer_crt ) ) != 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf) );
            mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse_file "
                            "returned -0x%04x - %s\n\n", -ret, buf );
            goto exit;
        }

        ret = mbedtls_x509_dn_gets( issuer_name, sizeof(issuer_name),
                                 &issuer_crt.subject );
        if( ret < 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  mbedtls_x509_dn_gets "
                            "returned -0x%04x - %s\n\n", -ret, buf );
            goto exit;
        }

        opt.issuer_name = issuer_name;

        mbedtls_printf( " ok\n" );
    }

#if defined(MBEDTLS_X509_CSR_PARSE_C)
    // Parse certificate request if present
    //
    if( !opt.selfsign && strlen( opt.request_file ) )
    {
        /*
         * 1.0.b. Load the CSR
         */
        mbedtls_printf( "  . Loading the certificate request ..." );

        if( ( ret = mbedtls_x509_csr_parse_file( &csr, opt.request_file ) ) != 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  mbedtls_x509_csr_parse_file "
                            "returned -0x%04x - %s\n\n", -ret, buf );
            goto exit;
        }

        ret = mbedtls_x509_dn_gets( subject_name, sizeof(subject_name),
                                 &csr.subject );
        if( ret < 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  mbedtls_x509_dn_gets "
                            "returned -0x%04x - %s\n\n", -ret, buf );
            goto exit;
        }

        opt.subject_name = subject_name;
        subject_key = &csr.pk;

        mbedtls_printf( " ok\n" );
    }
#endif /* MBEDTLS_X509_CSR_PARSE_C */

    /* 1.1. Load the keys
     */
    if( !opt.selfsign && !strlen( opt.request_file ) )
    {
        s = ioc_load_key(&loaded_subject_key, opt.subject_key_type);
        if (s) {
            goto exit;
        }
        mbedtls_printf( " ok\n" );
    }

    mbedtls_printf( "  . Loading the issuer key ..." );

    s = ioc_load_key(&loaded_issuer_key, opt.issuer_key_type);
    if (s) {
        goto exit;
    }

    /* ret = mbedtls_pk_parse_keyfile( &loaded_issuer_key, opt.issuer_key, opt.issuer_pwd );
    if (ret != 0) {
        mbedtls_strerror(ret, buf, sizeof(buf));
        mbedtls_printf( " failed\n  !  mbedtls_pk_parse_keyfile "
                        "returned -x%02x - %s\n\n", -ret, buf );
        goto exit;
    }
    */

    // Check if key and issuer certificate match
    //
    if( strlen( opt.issuer_crt ) )
    {
        if( mbedtls_pk_check_pair( &issuer_crt.pk, issuer_key ) != 0 )
        {
            mbedtls_printf( " failed\n  !  issuer_key does not match "
                            "issuer certificate\n\n" );
            goto exit;
        }
    }

    mbedtls_printf( " ok\n" );

    if( opt.selfsign )
    {
        opt.subject_name = opt.issuer_name;
        subject_key = issuer_key;
    }

    mbedtls_x509write_crt_set_subject_key( &crt, subject_key );
    mbedtls_x509write_crt_set_issuer_key( &crt, issuer_key );

    /*
     * 1.0. Check the names for validity
     */
    if( ( ret = mbedtls_x509write_crt_set_subject_name( &crt, opt.subject_name ) ) != 0 )
    {
        mbedtls_strerror(ret, buf, sizeof(buf));
        mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_subject_name "
                        "returned -0x%04x - %s\n\n", -ret, buf );
        goto exit;
    }

    if( ( ret = mbedtls_x509write_crt_set_issuer_name( &crt, opt.issuer_name ) ) != 0 )
    {
        mbedtls_strerror(ret, buf, sizeof(buf));
        mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_issuer_name "
                        "returned -0x%04x - %s\n\n", -ret, buf );
        goto exit;
    }

    mbedtls_printf( "  . Setting certificate values ..." );

    mbedtls_x509write_crt_set_version( &crt, opt.version );
    mbedtls_x509write_crt_set_md_alg( &crt, opt_md );

    ret = mbedtls_x509write_crt_set_serial( &crt, &serial );
    if( ret != 0 )
    {
        mbedtls_strerror(ret, buf, sizeof(buf));
        mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_serial "
                        "returned -0x%04x - %s\n\n", -ret, buf );
        goto exit;
    }

    ret = mbedtls_x509write_crt_set_validity( &crt, opt.not_before, opt.not_after );
    if( ret != 0 )
    {
        mbedtls_strerror(ret, buf, sizeof(buf));
        mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_validity "
                        "returned -0x%04x - %s\n\n", -ret, buf );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    if( opt.version == MBEDTLS_X509_CRT_VERSION_3 &&
        opt.basic_constraints != 0 )
    {
        mbedtls_printf( "  . Adding the Basic Constraints extension ..." );

        ret = mbedtls_x509write_crt_set_basic_constraints( &crt, opt.is_ca,
                                                           opt.max_pathlen );
        if( ret != 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  x509write_crt_set_basic_contraints "
                            "returned -0x%04x - %s\n\n", -ret, buf );
            goto exit;
        }

        mbedtls_printf( " ok\n" );
    }

#if defined(MBEDTLS_SHA1_C)
    if( opt.version == MBEDTLS_X509_CRT_VERSION_3 &&
        opt.subject_identifier != 0 )
    {
        mbedtls_printf( "  . Adding the Subject Key Identifier ..." );

        ret = mbedtls_x509write_crt_set_subject_key_identifier( &crt );
        if( ret != 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_subject"
                            "_key_identifier returned -0x%04x - %s\n\n",
                            -ret, buf );
            goto exit;
        }

        mbedtls_printf( " ok\n" );
    }

    if( opt.version == MBEDTLS_X509_CRT_VERSION_3 &&
        opt.authority_identifier != 0 )
    {
        mbedtls_printf( "  . Adding the Authority Key Identifier ..." );

        ret = mbedtls_x509write_crt_set_authority_key_identifier( &crt );
        if( ret != 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_authority_"
                            "key_identifier returned -0x%04x - %s\n\n",
                            -ret, buf );
            goto exit;
        }

        mbedtls_printf( " ok\n" );
    }
#endif /* MBEDTLS_SHA1_C */

    if( opt.version == MBEDTLS_X509_CRT_VERSION_3 &&
        opt.key_usage != 0 )
    {
        mbedtls_printf( "  . Adding the Key Usage extension ..." );

        ret = mbedtls_x509write_crt_set_key_usage( &crt, opt.key_usage );
        if( ret != 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_key_usage "
                            "returned -0x%04x - %s\n\n", -ret, buf );
            goto exit;
        }

        mbedtls_printf( " ok\n" );
    }

    if( opt.version == MBEDTLS_X509_CRT_VERSION_3 &&
        opt.ns_cert_type != 0 )
    {
        mbedtls_printf( "  . Adding the NS Cert Type extension ..." );

        ret = mbedtls_x509write_crt_set_ns_cert_type( &crt, opt.ns_cert_type );
        if( ret != 0 )
        {
            mbedtls_strerror(ret, buf, sizeof(buf));
            mbedtls_printf( " failed\n  !  mbedtls_x509write_crt_set_ns_cert_type "
                            "returned -0x%04x - %s\n\n", -ret, buf );
            goto exit;
        }

        mbedtls_printf( " ok\n" );
    }

    /*
     * 1.2. Writing the certificate
     */
    mbedtls_printf( "  . Writing the certificate..." );

    s = ioc_write_certificate(&crt, &opt, mbedtls_ctr_drbg_random, &t->ctr_drbg);
    if (s) {
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    exit_code = OSAL_SUCCESS;

exit:
#if defined(MBEDTLS_X509_CSR_PARSE_C)
    mbedtls_x509_csr_free( &csr );
#endif /* MBEDTLS_X509_CSR_PARSE_C */
    mbedtls_x509_crt_free( &issuer_crt );
    mbedtls_x509write_crt_free( &crt );
    mbedtls_pk_free( &loaded_subject_key );
    mbedtls_pk_free( &loaded_issuer_key );
    mbedtls_mpi_free( &serial );
    return exit_code;
}

#endif
