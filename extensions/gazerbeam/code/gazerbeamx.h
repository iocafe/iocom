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


#define MAX_GAZERBEAM_LAYERS 10
#define GAZERBEAM_VALUE_TYPE os_int

typedef struct GazerbeamBuffer
{
    GAZERBEAM_VALUE_TYPE x[MAX_GAZERBEAM_LAYERS];
    GAZERBEAM_VALUE_TYPE z[MAX_GAZERBEAM_LAYERS];
    os_int run_count;
    os_int nro_layers;
    os_boolean find_max;
}
GazerbeamBuffer;


/* Gazerbeam state structure.
 */
typedef struct Gazerbeam
{
    GazerbeamBuffer xmin;
}
Gazerbeam;


/* Initialize the Gazerbeam structure.
 */
void initialize_gazerbeam(
    Gazerbeam *gb,
    os_short flags);

/*
 */
GazerbeamBit gazerbeam_new_signal_value(
    Gazerbeam *gb,
    os_int x);

GAZERBEAM_VALUE_TYPE gazerbeam_minmax(
    GazerbeamBuffer *gbb,
    GAZERBEAM_VALUE_TYPE x);
