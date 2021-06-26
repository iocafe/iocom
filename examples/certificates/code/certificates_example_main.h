/**

  @file    iocom/examples/certificates/certificates_example_main.h
  @brief   Example and unit tests for certificate utilities.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "eosalx.h"
#include "makecertificate.h"

osalStatus my_generate_root_key(void);
osalStatus my_generate_root_certificate(void);
osalStatus my_generate_server_key(void);
osalStatus my_generate_certificate_request(void);

