/**

  @file    app_main.h
  @brief   Entry point and IO controller program set up.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  Code here is general program setup code. It initializes iocom library to be used as automation
  device controller. This example code uses eosal functions everywhere, including the program
  entry point osal_main(). If you use iocom library from an existing program, just call library
  iocom functions from C or C++ code and ignore "framework style" code here.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#include "ioserver.h"
#include "gina-for-tito.h"
#include "app_root.h"
#include "app_iodevice.h"
#include "app_iodevice_gina.h"
#include "app_seq.h"
#include "app_seq_blink_led.h"
#include "app_instance.h"

/* IOCOM root object for this application
 */
extern iocRoot app_iocom;