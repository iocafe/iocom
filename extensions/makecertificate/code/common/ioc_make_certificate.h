/**

  @file    ioc_make_certificate.h
  @brief   Making root and cerver certificates.
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
#ifndef MAKE_CERTIFICATE_H_
#define MAKE_CERTIFICATE_H_
#include "makecertificate.h"


typedef struct iocKeyOptions
{
    /* Either OS_PBNR_SERVER_KEY or OS_PBNR_ROOT_KEY.
     */
    osPersistentBlockNr key_type;

    /* Length of key in bits. If zero defaults to 2048.
     */
    os_short rsa_keysize;

    /* The output format to use: If FALSE saves key in PEM format. if TRUE,
       then DER format */
    os_boolean der_format;
}
iocKeyOptions;


typedef struct iocCertificateOptions
{
    /* OS_PBNR_SERVER_KEY or OS_PBNR_ROOT_KEY. */
    osPersistentBlockNr issuer_key_type;

    /* Either OS_PBNR_ROOT_CERT or OS_PBNR_SERVER_CERT.
     */
    osPersistentBlockNr cert_type;

    /* Process identification.
     */
    os_char *process_name;
    os_int process_nr;
    os_char *io_network_name;

    const os_char *issuer_crt;     /* filename of the issuer certificate   */
    const os_char *request_file;   /* filename of the certificate request  */
    const os_char *subject_key;    /* filename of the subject key file     */
    const os_char *issuer_key;     /* filename of the issuer key file      */
    const os_char *subject_pwd;    /* password for the subject key file    */
    const os_char *issuer_pwd;     /* password for the issuer key file     */
    const os_char *output_file;    /* where to store the constructed CRT   */
    const os_char *subject_name;   /* subject name for certificate         */
    const os_char *issuer_name;    /* issuer name for certificate          */
    const os_char *not_before;     /* validity period not before           */
    const os_char *not_after;      /* validity period not after            */
    const os_char *serial;         /* serial number string                 */
    os_boolean selfsign;               /* selfsign the certificate             */
    os_boolean is_ca;                  /* is a CA certificate                  */
    int max_pathlen;            /* maximum CA path length               */
    int authority_identifier;   /* add authority identifier to CRT      */
    int subject_identifier;     /* add subject identifier to CRT        */
    int basic_constraints;      /* add basic constraints ext to CRT     */
    int version;                /* CRT version                          */
    unsigned char key_usage;    /* key usage flags                      */
    unsigned char ns_cert_type; /* NS cert type                         */
}
iocCertificateOptions;


/* Generate a new RSA key.
 */
osalStatus ioc_generate_key(
    iocKeyOptions *popt);

osalStatus ioc_generate_certificate(
    iocCertificateOptions *popt);

#endif
