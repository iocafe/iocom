/**

  @file    iocom/examples/certificates/generate_certificate_request.c
  @brief   Example how a service should generate certificate request.
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
  Unit test code to create certificate request. The resulting certificate request stored as
  persistent block OS_PBNR_CERTIFICATE_REQUEST.
****************************************************************************************************
*/
osalStatus my_generate_certificate_request(void)
{
    osalStatus s;
    iocCertificateRequestOptions opt;
    os_memclear(&opt, sizeof(opt));
    s = ioc_certificate_request(&opt);
    osal_debug_error_status("my_generate_certificate_request failed: ", s);
    return s;
}

