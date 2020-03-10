/**

  @file    gazerbeam.c
  @brief   LED light communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.9.2020

  Configure microcontroller WiFi, etc, using Android phone. The idea is simple, an Andriod
  phone blinks wifi network name (SSID) and password (PSK) with it's flash light. Microcontroller
  is equaipped with simple ambient light photo diode which sees the signal.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "gazerbeam.h"


/**
****************************************************************************************************

  @brief Initialize the Gazerbeam structure.

  The initialize_gazerbeam() function clears the structure and sets initial state.

  @param   gb Pointer to the Gazerbeam structure to initialize.
  @param   flags Reserved for future, set 0 for now.
  @return  None.

****************************************************************************************************
*/
void initialize_gazerbeam(
    Gazerbeam *gb,
    os_short flags)
{
    os_memclear(&gb, sizeof(Gazerbeam));

    gb->xmin_buf.nro_layers = 8;
    gb->xmax_buf.nro_layers = 8;
    gb->xmax_buf.find_max = OS_TRUE;

    gb->prev_x = -1;
    gb->receive_pos = -1;
}

