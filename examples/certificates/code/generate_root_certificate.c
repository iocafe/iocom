/**

  @file    iocom/examples/certificates/generate_root_certificate.c
  @brief   Example for creating self signed root certificate.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "certificates_example_main.h"

/*
****************************************************************************************************
  Unit test code to create root certificate.
****************************************************************************************************
*/
osalStatus my_generate_root_certificate(void)
{
    osalStatus s;
    iocCertificateOptions opt;
    os_memclear(&opt, sizeof(opt));

    opt.selfsign = OS_TRUE;               /* selfsign the certificate             */
    opt.is_ca = OS_TRUE;                  /* is a CA certificate                  */
    opt.issuer_key_type = OS_PBNR_ROOT_KEY;
    opt.cert_type = OS_PBNR_ROOT_CERT;

    opt.process_name = "example";
    opt.process_nr = 3;
    opt.network_name = "cafenet";
    // opt.nickname = osal_nickname();

    s = ioc_generate_certificate(&opt);
    osal_debug_error_status("my_generate_root_certificate failed: ", s);
    return s;
}

