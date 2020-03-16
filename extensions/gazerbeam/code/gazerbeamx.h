/**

  @file    common/pins_gazerbeam.h
  @brief   LED light communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.3.2020

  Configure microcontroller WiFi, etc, using Android phone. The idea is simple, an Andriod
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

/* Logical states of a received bit, one, zero or no bit received.
 */
typedef enum GazerbeamBit
{
    GAZERBEAM_ZERO,
    GAZERBEAM_ONE,
    GAZERBEAM_NONE
}
GazerbeamBit;

/* Logical AD signal level.
 */
typedef enum
{
    GAZERBEAM_LOW,
    GAZERBEAM_HIGH
}
GazerbeamSignalLevel;

/* Maximum limit for nro_layers setting.
 */
#define MAX_GAZERBEAM_LAYERS 10

/* Type of AD conveted value in software.
 */
#define GAZERBEAM_VALUE os_int

/* Minimum and maximum AD signal must be at least this much apart
   that we even try to get the signal.
 */
#define GAZERBEAM_AD_NOICE_LEVEL 100

/* Maximum size of message in bytes.
 */
#define GAZERBEAM_MAX_MSG_SZ 199

/* Gazerbeam state structure. Typically allocated as global flat structure.
 */
typedef struct GazerbeamBuffer
{
    /* Buffers for tracking minimum or maximum signal value.
     */
    GAZERBEAM_VALUE x[MAX_GAZERBEAM_LAYERS];
    GAZERBEAM_VALUE z[MAX_GAZERBEAM_LAYERS];

    /* Just internal counter for filling the x and z buffers.
     */
    os_int run_count;

    /* How many AD values are used to keep track maximum and minimum
     * signal levels. N = 2^nro_layers.
     */
    os_int nro_layers;

    /* Looking for maximum or minimum signal value?
     */
    os_boolean find_max;
}
GazerbeamBuffer;


/* Gazerbeam state structure.
 */
typedef struct Gazerbeam
{
    /* Mimumum and maximum filtering buffers.
     */
    GazerbeamBuffer xmin_buf, xmax_buf;

    /* Previous signal value and digital level.
     */
    // GAZERBEAM_VALUE prev_x;
    os_timer prev_ti;
    GazerbeamSignalLevel prev_signal;

    os_char msgbuf[GAZERBEAM_MAX_MSG_SZ + 1];
    os_int n_bytes;
    os_int n_zeros;
    os_int receive_pos;
    os_char receive_bit;
}
Gazerbeam;


/* Initialize the Gazerbeam structure.
 */
void initialize_gazerbeam(
    Gazerbeam *gb,
    os_short flags);

/* Decode analog input reading to logical ones and zeroes.
 */
GazerbeamBit gazerbeam_decode_modulation(
    Gazerbeam *gb,
    GAZERBEAM_VALUE x,
    os_timer *ti);

/* Generate a message based on received data.
 */
osalStatus gazerbeam_decode_message(
    Gazerbeam *gb,
    GAZERBEAM_VALUE x,
    os_timer *ti);

/* Get the received message into buffer.
 */
os_memsz gazerbeam_get_message(
    Gazerbeam *gb,
    os_char *buf,
    os_memsz buf_sz);

/* Find out minimum or maximum value of the last N samples.
 */
GAZERBEAM_VALUE gazerbeam_minmax(
    GazerbeamBuffer *gbb,
    GAZERBEAM_VALUE x);
