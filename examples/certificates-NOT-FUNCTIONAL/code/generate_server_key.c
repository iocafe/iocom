/**

  @file    iocom/examples/certificates/generate_server_key.c
  @brief   Example for creating private server key.
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
  Unit test code to create server key. The resulting key is stored as persistent
  block OS_PBNR_SERVER_KEY.
****************************************************************************************************
*/
osalStatus my_generate_server_key(void)
{
    iocKeyOptions opt;
    osalStatus s;
    os_memclear(&opt, sizeof(opt));
    opt.key_type = OS_PBNR_SERVER_KEY;
    opt.der_format = OS_TRUE;
    s = ioc_generate_key(&opt);
    osal_debug_error_status("my_generate_server_key failed: ", s);
    return s;
}

