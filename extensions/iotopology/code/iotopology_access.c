/**

  @file    iocdomain_main.c
  @brief   IO domain controller library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.9.2019

  The domain controller library listens for connections from IO devices and other IO comain
  controllers. Once an IO device connects to domain, memory maps for the device are created.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iotopology/iotopology.h"




/**
****************************************************************************************************

  @brief Set up and start IO domain.

  The iodomain_start() starts the IO domain listening for TLS socket connections.

  The iofomain function listens for socket connections and dynamically creates memory blocks
  according to information received from the device.

  @return  None.

****************************************************************************************************
*/
void xxxxiodomain_start(
    void)
{
}


