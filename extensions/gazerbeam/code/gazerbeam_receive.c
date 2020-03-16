/**

  @file    gazerbeam_receive.c
  @brief   LED light communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.3.2020

  Configure microcontroller WiFi, etc, using Android phone.

  Message 0 = zero bit, 1 = one bit, x = data bit. Message is started by nine zeroes followed
  by one (there can be extra zeroes). There must be also one 1 to separate the data data bytes.
  Me
  0000000001 xxxxxxxx 1 xxxxxxxx 1 xxxxxxxx 1

  Followed imediately by repeat message. Beginning of next message is termination of the
  previous one. So at least start of next message 0000000001 is needed to process the previous
  one.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "gazerbeam.h"


/**
****************************************************************************************************

  @brief Decode analog input reading to logical ones and zeroes.

  Decode signal modulation into bits. This function needs to be called on analog input values
  on suitable frequency, perhaps from interrupt handler.

  @param   gb Pointer to the Gazerbeam structure.
  @param   x New signal value.
  @param   ti Timer value for signal
  @return  GAZERBEAM_ZERO if bit "0" is received, GAZERBEAM_ONE if bit "1" is received, or
           GAZERBEAM_NONE if no data bit is received.

****************************************************************************************************
*/
GazerbeamBit gazerbeam_decode_modulation(
    Gazerbeam *gb,
    GAZERBEAM_VALUE x,
    os_timer *ti)
{
    GAZERBEAM_VALUE xmin, xmax, one_third, low_limit, high_limit;
    // os_int dx;
    GazerbeamSignalLevel signal;
    GazerbeamBit bit;

    if (!os_elapsed2(&gb->prev_ti, ti, 2))
    {
        return GAZERBEAM_NONE;
    }
    gb->prev_ti = *ti;

    /* Add new value to minimum and maximum filters and get current minimum and maximum value.
     */
    xmin = gazerbeam_minmax(&gb->xmin_buf, x);
    xmax = gazerbeam_minmax(&gb->xmax_buf, x);
    if (xmin + GAZERBEAM_AD_NOICE_LEVEL >= xmax)
    {
        // gb->prev_x = -1;
        gb->receive_pos = -1;
        return GAZERBEAM_NONE;
    }

    /* If this is different than previous value, we are in transition, ignore.
     */
    /* dx = (os_int)x - (os_int)gb->prev_x;
    gb->prev_x = x;
    if (dx < 0) dx = -dx;
    if (dx > (xmax - xmin) / 10)
    {
        return GAZERBEAM_NONE;
    }
    */

    /* Limits for high, low and stopped in middle levels.
     */
    one_third = 2 * (xmax - xmin) / 5;
    low_limit = xmin + one_third;
    high_limit = xmax - one_third;

    /* Decide digital signal level, low, high or center.
     */
    if (x < low_limit)
    {
        signal = GAZERBEAM_LOW;
    }
    else if (x > high_limit)
    {
        signal = GAZERBEAM_HIGH;
    }
    else
    {
        return GAZERBEAM_NONE;
    }

    /* If this is same signal as previous, we have no new data.
     */
    if (signal == gb->prev_signal) return GAZERBEAM_NONE;
    gb->prev_signal = signal;

    /* If we got one value (half transistion to center)
     */
    if (signal == GAZERBEAM_HIGH) {
        osal_debug_error_int("HERE high_detected ", x);
    }
    else
    {
        osal_debug_error_int("HERE low_detected ", x);
    }
    osal_debug_error_int("HERE lo lim ", low_limit);
    osal_debug_error_int("HERE hi lim ", high_limit);


bit = GAZERBEAM_NONE; // GAZERBEAM_ONE; GAZERBEAM_ZERO;

    return bit;
}


/**
****************************************************************************************************

  @brief Generate a message based on received data.

  Form messages from bits. This function is called repeatedly with light intensity analog input
  value x. If calls gazerbeam_decode_modulation to get received "0" and "1" bits, and generates
  messages of these. This function needs to be called on analog input values
  on suitable frequency, perhaps from interrupt handler.

  @param   gb Pointer to the Gazerbeam structure.
  @param   x New signal value.
  @param   ti Timer value for signal
  @return  OSAL_COMPLETED when a complete message has been received, OSAL_SUCCESS when data
           was received and added to buffer. OSAL_PENDING indicates that noting useful was done,
           other values indicate that we are receiving garbage.

****************************************************************************************************
*/
osalStatus gazerbeam_decode_message(
    Gazerbeam *gb,
    GAZERBEAM_VALUE x,
    os_timer *ti)
{
    GazerbeamBit bit;
    os_int n_bytes;

    bit = gazerbeam_decode_modulation(gb, x, ti);
    if (bit == GAZERBEAM_NONE) return OSAL_PENDING;

    /* Track if we got at least nine zeroes in row followed by one, which marks a beginning of
       a message. Return if we are not receiving the message.
     */
    if (bit == GAZERBEAM_ZERO)
    {
        if (gb->n_zeros < 9) gb->n_zeros++;
        if (gb->n_zeros == 9)
        {
            /* If we received complete message before this one */
            n_bytes = gb->receive_pos;
            if (gb->receive_bit > 1) n_bytes++;
            gb->receive_pos = -1;
            if (n_bytes > 0)
            {
                gb->n_bytes = n_bytes;
                return OSAL_COMPLETED;
            }
        }
    }
    else
    {
        /* If beginning of message.
         */
        if (gb->n_zeros == 9)
        {
            gb->receive_pos = 0;
            gb->receive_bit = 0;
            return OSAL_PENDING;
        }

        gb->n_zeros = 0;
    }
    if (gb->receive_pos < 0) return OSAL_PENDING;

    /* If expecting "1" bit starting a character.
     */
    if (gb->receive_bit == 0)
    {
        /* We must have 1 here, otherwise message is corrupted.
         */
        if (bit != GAZERBEAM_ONE)
        {
            gb->receive_pos = -1;
            return OSAL_STATUS_FAILED;
        }

        gb->msgbuf[gb->receive_pos] = (bit == GAZERBEAM_ONE ? 1 : 0);
        gb->receive_bit = 1;
    }
    else
    {
        if (bit == GAZERBEAM_ONE)
        {
            gb->msgbuf[gb->receive_pos] |= gb->receive_bit;
        }
        if (gb->receive_bit > 0x7F)
        {
            gb->receive_bit = 0;
            if (++(gb->receive_pos) > GAZERBEAM_MAX_MSG_SZ)
            {
                gb->receive_pos = -1;
                return OSAL_STATUS_FAILED;
            }
        }
        else
        {
            gb->receive_bit <<= 1;
        }
    }

    return OSAL_SUCCESS;
}


/**
****************************************************************************************************

  @brief Get the received message into buffer.

  This function needs to be called immediately when gazerbeam_decode_message return OSAL_COMPLETED
  to get the received message.

  @param   gb Pointer to the Gazerbeam structure.
  @param   buf Pointer to buffer where to copy the message.
  @param   buf_sz Buffer size in bytes.
  @return  Message length in bytes.

****************************************************************************************************
*/
os_memsz gazerbeam_get_message(
    Gazerbeam *gb,
    os_char *buf,
    os_memsz buf_sz)
{
    if (buf_sz > gb->n_bytes) buf_sz = gb->n_bytes;
    os_memcpy(buf, gb->msgbuf, buf_sz);
    return buf_sz;
}

