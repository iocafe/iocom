/**

  @file    common/pins_gazerbeam.h
  @brief   LED to LED communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.3.2020

  Configure microcontroller WiFi, etc, using Android phone, etc. The idea is simple, an Andriod
  phone blinks wifi network name (SSID) and password (PSK) with it's flash light. Microcontroller
  is equaipped with simple ambient light photo diode which sees the signal.

  - Band pass filter is used to get right frequency range.
  - Simple modulation schema.

  Blue tooth based WiFi configuration in selectwifi library is alternative for the Gazerbeam.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


typedef enum GazerbeamBit
{
    GAZERBEAM_ZERO,
    GAZERBEAM_ONE,
    GAZERBEAM_NONE
}
GazerbeamBit;


/* Gazerbeam state structure.
 */
typedef struct Gazerbeam
{
    const Pin *pin;

    os_timer timer;
    os_int code;
    os_int prev_code;
    os_int pos;
    os_boolean start_led_on;
    os_boolean led_on;

    MorseRecipe recipe;
}
Gazerbeam;


/* Setup the Gazer beam structure.
 */
void gazerbeam_setup(
    Gazerbeam *gb,
    os_boolean flags);

/*
 */
GazerbeamBit gazerbeam_new_value(
    Gazerbeam *gb,
    os_int code);
