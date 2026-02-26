/**

  @file    iocom/examples/certificates/generate_own_server_certificate.c
  @brief   Example for creating self signed server certificate.
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
  Unit test code to create and sign own server certificate using locally stored root key
  and certificate.
****************************************************************************************************
*/
osalStatus my_generate_own_server_certificate(void)
{
    osalStatus s;
    iocCertificateOptions opt;
    os_memclear(&opt, sizeof(opt));

    // opt.selfsign = OS_TRUE;
    // opt.is_ca = OS_TRUE;
    opt.issuer_key_type = OS_PBNR_ROOT_KEY;
    opt.cert_type = OS_PBNR_SERVER_CERT;
    opt.subject_key_type = OS_PBNR_SERVER_KEY;

    opt.process_name = "example";
    opt.process_nr = 3;
    opt.network_name = "cafenet";
    // opt.nickname = osal_nickname();

    s = ioc_generate_certificate(&opt);
    osal_debug_error_status("my_generate_own_server_certificate failed: ", s);
    return s;
}

