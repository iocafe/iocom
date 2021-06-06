/**

  @file    iocom/examples/certificates/generate_root_key.c
  @brief   Example for creating private root key.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eosal.h"
#include "certificates_example_main.h"

/*
****************************************************************************************************
  Unit test code to create root key.
****************************************************************************************************
*/
void my_generate_root_key(void)
{
    iocKeyOptions opt;
    os_memclear(&opt, sizeof(opt));
    opt.key_type = OS_PBNR_ROOT_KEY;
    opt.der_format = OS_TRUE;
    ioc_generate_key(&opt);
}

