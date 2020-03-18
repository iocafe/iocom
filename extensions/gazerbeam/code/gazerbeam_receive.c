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

int pekka_testaa;

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
    GAZERBEAM_VALUE half, low_limit, high_limit;
    GAZERBEAM_VALUE pulse_ms, tmin, tmax;
    GazerbeamSignalLevel signal;
    GazerbeamBit bit;

    if (os_has_elapsed_since(&gb->prev_ti, ti, 500))
    {
        gb->prev_ti = *ti;
        osal_debug_error_int("HERE XX ", pekka_testaa);
        return GAZERBEAM_NONE;
    }


    if (os_has_elapsed_since(&gb->prev_ti, ti, 60 * 1000) || gb->x_count >= 0x100000000000L)
    {
        gb->x_sum /= 2;
        gb->x_count /= 2;

        gb->prev_ti = *ti;
    }

    gb->x_sum += x;
    gb->x_count++;

    half = (os_int)(gb->x_sum / gb->x_count);
    low_limit = half - GAZERBEAM_AD_NOICE_LEVEL;
    high_limit = half + GAZERBEAM_AD_NOICE_LEVEL;

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

    /* Get minimum and maximum pulse length
     */
    pulse_ms = (GAZERBEAM_VALUE)os_get_ms_elapsed(&gb->pulse_timer, ti);
    gb->pulse_timer = *ti;
    tmin = gazerbeam_minmax(&gb->tmin_buf, pulse_ms);
    tmax = gazerbeam_minmax(&gb->tmax_buf, pulse_ms);
    if (tmin < 3 || tmax > 60) return GAZERBEAM_NONE;

    /* If we got one value (half transistion to center)
     */
    /* if (signal == GAZERBEAM_HIGH) {
        osal_debug_error_int("HERE high_detected ", x);
    }
    else
    {
        osal_debug_error_int("HERE low_detected ", x);
    } */
    //osal_debug_error_int("HERE lo lim ", tmin);
    //osal_debug_error_int("HERE hi lim ", tmax);

    bit = pulse_ms > (tmax + tmin)/2 ? GAZERBEAM_ONE : GAZERBEAM_ZERO;

   // bit = pulse_ms > 25 ? GAZERBEAM_ONE : GAZERBEAM_ZERO;
//osal_debug_error_int("HERE bit detected ", bit);

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
    os_ushort crc, crc2;

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
            gb->receive_pos = -1;
            if (n_bytes > 3)
            {
                gb->n_bytes = n_bytes;
                crc = (os_uchar)gb->msgbuf[1];
                crc <<= 8;
                crc |= (os_uchar)gb->msgbuf[0];
                gb->msgbuf[0] = 0;
                gb->msgbuf[1] = 0;
                crc2 = os_checksum(gb->msgbuf, n_bytes, OS_NULL);
                if (crc != crc2)
                {
                    osal_trace3("gazerbeam checksum error");
osal_debug_error_int("gazerbeam checksum error ", n_bytes);
                    return OSAL_STATUS_CHECKSUM_ERROR;
                }
                else
                {

                    gb->msgbuf[n_bytes] = '*';
                    gb->msgbuf[n_bytes+1] = '\0';
                    osal_console_write(gb->msgbuf + 3);
                    osal_debug_error_int("HERE ok received ", n_bytes);

                    return OSAL_COMPLETED;
                }
            }
        }
    }
    else
    {
        /* If beginning of message.
         */
        if (gb->n_zeros == 9)
        {
osal_debug_error_int("HERE beginning of message ", bit);
            gb->receive_pos = 0;
            gb->receive_bit = 0;
            os_memclear(gb->msgbuf, GAZERBEAM_MAX_MSG_SZ);
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
osal_debug_error_int("HERE failed 1 ", bit);
            return OSAL_STATUS_FAILED;
        }

        // gb->msgbuf[gb->receive_pos] = (bit == GAZERBEAM_ONE ? 1 : 0);
        gb->receive_bit = 1;
    }
    else
    {
        if (bit == GAZERBEAM_ONE)
        {
            gb->msgbuf[gb->receive_pos] |= gb->receive_bit;
        }
        if (gb->receive_bit & 0x80)
        {
            gb->receive_bit = (gb->msgbuf[gb->receive_pos] ? 1 : 0);

            if (++(gb->receive_pos) > GAZERBEAM_MAX_MSG_SZ)
            {
                gb->receive_pos = -1;
osal_debug_error_int("HERE failed 2 ", bit);
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
  @param   buf Pointer to buffer where to copy the message. Buffer is '\0' terminated if
           '\0' char fits in buffer. If you need to make sure about '\0' character,
           allocate buffer size GAZERBEAM_MAX_MSG_SZ.
  @param   buf_sz Buffer size in bytes.
  @return  Message length in bytes.

****************************************************************************************************
*/
os_memsz gazerbeam_get_message(
    Gazerbeam *gb,
    os_char *buf,
    os_memsz buf_sz)
{
    os_int n;

    n = buf_sz;
    if (n + 2 > gb->n_bytes) n = gb->n_bytes - 2;
    os_memcpy(buf, gb->msgbuf + 2, n);
    if (n < buf_sz) buf[n] = '\0';
    return n;
}

