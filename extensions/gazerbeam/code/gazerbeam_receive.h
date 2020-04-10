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

  Blue tooth based WiFi configuration in selectwifi library is alternative for the GazerbeamReceiver.

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

struct Pin;

/* Logical states of a received bit, one, zero or no bit received.
 */
typedef enum GazerbeamBit
{
    GAZERBEAM_ZERO,
    GAZERBEAM_ONE
}
GazerbeamBit;

/* Maximum size of message in bytes excluding check sum but including terminating '\0' character.
 */
#define GAZERBEAM_MAX_MSG_SZ 128

/* GazerbeamReceiver state structure.
 */
typedef struct GazerbeamReceiver
{
    /* Input PIN if connected to pin interrupt. OS_NULL otherwise.
     */
    const struct Pin *pin;
    // void *int_handler_func;

    /* Mimumum and maximum pulse length filtering buffers.
     */
    GazerbeamBuffer tmin_buf, tmax_buf;
    os_timer pulse_timer;

    os_char msgbuf[GAZERBEAM_MAX_MSG_SZ + 3]; /* +3 for check sum */
    os_short n_zeros;
    os_short receive_pos;
    os_char receive_bit;

    /* Beam connected flag ans previous state. Used to display beam connected indicator
     */
    volatile os_char beam_connected;
    os_timer beam_connected_timer;

    /* If configuration matches (changes nothing)
     */
    os_boolean configuration_match;
    os_timer configuration_match_timer;

    /* Finished message without leading checksum, changed only when
       finshed_message_sz is zero and finshed_message_sz reset after reading.
     */
    volatile os_char finshed_message[GAZERBEAM_MAX_MSG_SZ];
    volatile os_short finshed_message_sz;
}
GazerbeamReceiver;

/* Flags for functions.
 */
#define GAZERBEAM_DEFAULT 0
#define GAZERBEAM_NO_NULL_TERMNATION 1

/* Initialize the GazerbeamReceiver structure.
 */
void initialize_gazerbeam_receiver(
    GazerbeamReceiver *gb,
    const struct Pin *pin,
    os_short flags);

/* Generate a message based on received data.
 */
osalStatus gazerbeam_decode_message(
    GazerbeamReceiver *gb,
    os_timer *ti);

/* Get the received message into buffer.
 */
os_memsz gazerbeam_get_message(
    GazerbeamReceiver *gb,
    os_char *buf,
    os_memsz buf_sz,
    os_short flags);
