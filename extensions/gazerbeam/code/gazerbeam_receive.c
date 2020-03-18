/**

  @file    gazerbeam_receive.c
  @brief   LED light communication.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.3.2020

  Configure microcontroller WiFi, etc, using Android phone. The idea is simple, an Andriod
  phone blinks wifi network name (SSID) and password (PSK) with it's flash light. Microcontroller
  is equaipped with simple ambient light photo diode which sees the signal.

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

#if GAZERBEAM_PINS_SUPPORT
#define PINS_OS_INT_HANDLER_HDRS 1
#include "pins.h"
#endif


/* Pointer to gazerbeam receiver structure, is connected to pin interrupt. OS_NULL otherwise.
 */
GazerbeamReceiver *global_gazerbeam = OS_NULL;


/**
****************************************************************************************************

  @brief Pin state change interrupt handler function.

  The gazerbeam_led_int_handler() function is interrupt handler function.

****************************************************************************************************
*/
BEGIN_PIN_INTERRUPT_HANDLER(gazerbeam_led_int_handler)
     gazerbeam_decode_message(global_gazerbeam, OS_NULL);
END_PIN_INTERRUPT_HANDLER


/**
****************************************************************************************************

  @brief Initialize the GazerbeamReceiver structure.

  The initialize_gazerbeam_receiver() function clears the structure and sets initial state.

  @param   gb Pointer to the GazerbeamReceiver structure to initialize.
  @param   pin Input GPIO pin if connected to pin interrupt. OS_NULL otherwise.
  @param   flags Reserved for future, set GAZERBEAM_DEFAULT (0) for now.
  @return  None.

****************************************************************************************************
*/
void initialize_gazerbeam_receiver(
    GazerbeamReceiver *gb,
    const struct Pin *pin,
    os_short flags)
{
    os_memclear(gb, sizeof(GazerbeamReceiver));

    gb->tmin_buf.nro_layers = 4;
    gb->tmax_buf.nro_layers = 4;
    gb->tmax_buf.find_max = OS_TRUE;

    gb->receive_pos = -1;

#if GAZERBEAM_PINS_SUPPORT
    /* If we have pin, attach interrupt handler.
     */
    if (pin)
    {
        global_gazerbeam = gb;
        pinInterruptParams prm;
        gb->pin = pin;
        os_memclear(&prm, sizeof(prm));
        prm.int_handler_func = gazerbeam_led_int_handler;
        prm.flags = PINS_INT_CHANGE;
        pin_attach_interrupt(pin, &prm);
    }
#endif
}


/**
****************************************************************************************************

  @brief Generate a message based on received data.

  Form messages from bits. This function can be called from interrupt handler, so no debug
  prints, etc.

  @param   gb Pointer to the GazerbeamReceiver structure.
  @param   ti Timer value for signal
  @return  OSAL_COMPLETED when a complete message has been received, OSAL_SUCCESS when data
           was received and added to buffer. OSAL_PENDING indicates that noting useful was done,
           other values indicate that we are receiving garbage.

****************************************************************************************************
*/
osalStatus gazerbeam_decode_message(
    GazerbeamReceiver *gb,
    os_timer *ti)
{
    GazerbeamBit bit;
    os_int n_bytes;
    os_ushort crc, crc2;
    GAZERBEAM_VALUE pulse_ms, tmin, tmax;
    os_timer til;

    if (ti == OS_NULL)
    {
        os_get_timer(&til);
        ti = &til;
    }

    /* Get minimum and maximum pulse length
     */
    pulse_ms = (GAZERBEAM_VALUE)os_get_ms_elapsed(&gb->pulse_timer, ti);
    gb->pulse_timer = *ti;
    tmin = gazerbeam_minmax(&gb->tmin_buf, pulse_ms);
    tmax = gazerbeam_minmax(&gb->tmax_buf, pulse_ms);
    if (tmin < 2 || tmax > 60) return OSAL_PENDING;

    /* Decice from pulse length if this is one or zero.
     */
    bit = pulse_ms > (tmax + tmin)/2 ? GAZERBEAM_ONE : GAZERBEAM_ZERO;

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
            gb->receive_complete = OS_TRUE;
            if (n_bytes >= 3 && gb->finshed_message_sz == 0)
            {
                crc = (os_uchar)gb->msgbuf[1];
                crc <<= 8;
                crc |= (os_uchar)gb->msgbuf[0];
                gb->msgbuf[0] = 0;
                gb->msgbuf[1] = 0;
                crc2 = os_checksum(gb->msgbuf, n_bytes, OS_NULL);
                if (crc != crc2)
                {
                    return OSAL_STATUS_CHECKSUM_ERROR;
                }
                else
                {
                    n_bytes -= 2; /* No checksum any more */
                    os_memcpy((void*)gb->finshed_message, gb->msgbuf + 2, n_bytes);
                    gb->finshed_message_sz = n_bytes;
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
            gb->receive_pos = 0;
            gb->receive_bit = 0;
            gb->msgbuf[0] = 0;
            gb->receive_complete = OS_FALSE;
        }

        gb->n_zeros = 0;
    }
    if (gb->receive_pos < 0 || gb->receive_complete) return OSAL_PENDING;

    /* If expecting "1" bit starting a character.
     */
    if (gb->receive_bit == 0)
    {
        /* We must have 1 here, otherwise message is corrupted.
         */
        if (bit == GAZERBEAM_ZERO)
        {
            gb->receive_complete = OS_TRUE;
            return OSAL_SUCCESS;
        }

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
            gb->receive_bit = 0;

            if (++(gb->receive_pos) >= GAZERBEAM_MAX_MSG_SZ + 2)
            {
                gb->receive_pos = -1;
                gb->receive_complete = OS_TRUE;
                return OSAL_STATUS_FAILED;
            }
            gb->msgbuf[gb->receive_pos] = 0;
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

  This function needs to be called when gazerbeam_decode_message returns OSAL_COMPLETED
  to get the received message. It can also be called periodically, if there is no received
  message the function sets buf to "\0" and returns 0.

  Notice that message doesn't have to be a string. If message is binary structure, etc.
  Use GAZERBEAM_NO_NULL_TERMNATION flag to stop settring terminatinf character.
  null termination just means that there is 0 byte after the data content

  @param   gb Pointer to the GazerbeamReceiver structure.
  @param   buf Pointer to buffer where to copy the message. Buffer is '\0' terminated,
           recommended minimum size GAZERBEAM_MAX_MSG_SZ.
           Buffer can be shorter than the recommended size, but If buffer is shorter than
           message, message trunkated but still '\0' terminated.
  @param   buf_sz Buffer size in bytes.
  @param   flags GAZERBEAM_DEFAULT (0) to terminate buffer with NULL character,
           or GAZERBEAM_NO_NULL_TERMNATION not to.
  @return  Message length in bytes excluding terminating '\0' character.

****************************************************************************************************
*/
os_memsz gazerbeam_get_message(
    GazerbeamReceiver *gb,
    os_char *buf,
    os_memsz buf_sz,
    os_short flags)
{
    os_int n;

    n = gb->finshed_message_sz;
    if ((flags & GAZERBEAM_NO_NULL_TERMNATION) == 0) buf_sz--;
    if (n > buf_sz) n = buf_sz;
    if (n) os_memcpy(buf, (void*)gb->finshed_message, n);
    if ((flags & GAZERBEAM_NO_NULL_TERMNATION) == 0) buf[n] = '\0';
    gb->finshed_message_sz = 0;
    return n;
}
