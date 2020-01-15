/**

  @file    app_main.h
  @brief   Entry point and IO controller program set up.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  Code here is general program setup code. It initializes iocom library to be used as dynamic
  IO controller. This example code uses eosal functions everywhere, including the program
  entry point osal_main(). If you use iocom library from an existing program, just call library
  iocom functions from C or C++ code and ignore "framework style" code here.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#include "nodeconf.h"
#include "ioserver.h"

OSAL_C_HEADER_BEGINS
#include "network-defaults.h"
#include "signals.h"
#include "info-mblk-binary.h"
OSAL_C_HEADER_ENDS

#include "app_root.h"
#include "app_instance.h"

extern iocRoot ioapp_root;
