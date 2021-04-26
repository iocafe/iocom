/**

  @file    ioc_establish_serial_connection.c
  @brief   Send data to connection.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

#if OSAL_SERIAL_SUPPORT


/**
****************************************************************************************************

  @brief Establish serial connection.
  @anchor ioc_connection_send

  The ioc_establish_serial_connection function establises the starting point for data transfer,
  so that both ends of communication are at initial point.

  @param   con Pointer to the connection object.
  @return  OSAL_SUCCESS if all connection is establised and we can transfer data normally.
           OSAL_PENDING = we are estabilishing serial connection
           Other return values indicate on error.

****************************************************************************************************
*/
osalStatus ioc_establish_serial_connection(
    iocConnection *con)
{
    IOC_MT_ROOT_PTR;

    os_memsz
        n_written,
        n_read;

    const os_uchar
        connect_char = IOC_SERIAL_CONNECT,
        confirm_char = IOC_SERIAL_CONFIRM,
        disconnect_char = IOC_SERIAL_DISCONNECT,
        connect_reply_char = IOC_SERIAL_CONNECT_REPLY,
        confirm_reply_char = IOC_SERIAL_CONFIRM_REPLY;

    os_uchar
        buf[32];

    if ((con->flags & IOC_SOCKET) || con->sercon_state == OSAL_SERCON_STATE_CONNECTED_5) 
    {
        return OSAL_SUCCESS;
    }

    ioc_set_mt_root(root, con->link.root);
    ioc_lock(root);

    /* If we are running serial connection, follow connect procedure. Notice that
       checking for received control frames is in ioc_connection_receive.c.
     */

    /* Client end of the serial connection.
     */
    if ((con->flags & IOC_LISTENER) == 0)
    {
        switch (con->sercon_state)
        {
            default:
            case OSAL_SERCON_STATE_INIT_1:
                /* Clear RX and TX buffers.
                 */
                osal_stream_flush(con->stream,
                    OSAL_STREAM_CLEAR_RECEIVE_BUFFER|
                    OSAL_STREAM_CLEAR_TRANSMIT_BUFFER);

                /* Start timer.
                 */
                os_get_timer(&con->sercon_timer);

                /* Send connect character.
                 */
                osal_stream_write(con->stream, (os_char*)&connect_char, 1,
                    &n_written, OSAL_STREAM_DEFAULT);
                osal_debug_assert(n_written == 1);

                /* Move on to step 2.
                 */
                con->sercon_state = OSAL_SERCON_STATE_INIT_2;
                break;

            case OSAL_SERCON_STATE_INIT_2:
                /* Try to read a character.
                 */
                osal_stream_read(con->stream, (os_char*)buf, sizeof(buf),
                    &n_read, OSAL_STREAM_DEFAULT);

                /* If last character  received is CONNECT_REPLY character,
                   then send CONFIRM character and start wait for CONFIRM_REPLY.
                 */
                if (n_read >= 1 && n_read < sizeof(buf))
                    if (buf[n_read-1] == IOC_SERIAL_CONNECT_REPLY)
                {
                    /* Send confirm character.
                     */
                    osal_stream_write(con->stream, (os_char*)&confirm_char, 1,
                        &n_written, OSAL_STREAM_DEFAULT);
                    osal_debug_assert(n_written == 1);

                    /* Start timer.
                     */
                    os_get_timer(&con->sercon_timer);

                    con->sercon_state = OSAL_SERCON_STATE_INIT_3;
                    break;
                }

                /* If time out while waiting for CONNECT_REPLY, start over.
                 */
                if (os_has_elapsed(&con->sercon_timer, IOC_SERIAL_CONNECT_PERIOD_MS))
                {
                    con->sercon_state = OSAL_SERCON_STATE_INIT_1;
                }
                break;

            case OSAL_SERCON_STATE_INIT_3:
                /* Try to read a character. Now we read only one character
                   because other end may start sending acual data immediately
                   after confirm character.
                 */
                osal_stream_read(con->stream, (os_char*)buf, 1,
                    &n_read, OSAL_STREAM_DEFAULT);

                /* If CONFIRM_REPLY character received, clear connection
                   state and move on to data transfer.
                 */
                if (n_read == 1 && buf[0] == IOC_SERIAL_CONFIRM_REPLY)
                {
                    ioc_reset_connection_state(con);
                    con->sercon_state = OSAL_SERCON_STATE_CONNECTED_5;
                    ioc_unlock(root);
                    return OSAL_SUCCESS;
                }

                /* If time out while waiting for CONFIRMT_REPLY, start over.
                 */
                if (os_has_elapsed(&con->sercon_timer, IOC_SERIAL_CONNECT_PERIOD_MS) /* || n_read */)
                {
                    con->sercon_state = OSAL_SERCON_STATE_INIT_1;
                }
                break;
        }
    }

    /* Server end of the connection.
     */
    else
    {
        switch (con->sercon_state)
        {
            default:
            case OSAL_SERCON_STATE_INIT_1:
                osal_stream_write(con->stream, (os_char*)&disconnect_char, 1,
                    &n_written, OSAL_STREAM_DEFAULT);
                osal_debug_assert(n_written == 1);
                con->sercon_state = OSAL_SERCON_STATE_INIT_2;
                break;

            case OSAL_SERCON_STATE_INIT_2:
                osal_stream_flush(con->stream,
                    OSAL_STREAM_CLEAR_RECEIVE_BUFFER|
                    OSAL_STREAM_CLEAR_TRANSMIT_BUFFER);
                con->sercon_state = OSAL_SERCON_STATE_INIT_3;
                break;

            case OSAL_SERCON_STATE_INIT_3:
               /* Try to read a character.
                 */
                osal_stream_read(con->stream, (os_char*)buf, sizeof(buf),
                    &n_read, OSAL_STREAM_DEFAULT);

                /* If last character received is CONNECT character, then send
                   CONNECT_REPLY character and start wait for CONFIRM.
                 */
                if (n_read >= 1 && n_read < sizeof(buf))
                    if (buf[n_read-1] == IOC_SERIAL_CONNECT)
                {
                    /* Send connect reply character.
                     */
                    osal_stream_write(con->stream, (os_char*)&connect_reply_char, 1,
                        &n_written, OSAL_STREAM_DEFAULT);
                    osal_debug_assert(n_written == 1);

                    con->sercon_state = OSAL_SERCON_STATE_INIT_4;
                }
                break;

            case OSAL_SERCON_STATE_INIT_4:
               /* Try to read a character.
                 */
                osal_stream_read(con->stream, (os_char*)buf, sizeof(buf),
                    &n_read, OSAL_STREAM_DEFAULT);

                /* If last character received is CONFIRM character, then send
                   CONNECT_REPLY character and start wait for CONFIRM.
                 */
                if (n_read == 1 && buf[0] == IOC_SERIAL_CONFIRM)
                {
                    /* Send confirm reply character.
                     */
                    osal_stream_write(con->stream, (os_char*)&confirm_reply_char, 1,
                        &n_written, OSAL_STREAM_DEFAULT);
                    osal_debug_assert(n_written == 1);

                    ioc_reset_connection_state(con);
                    con->sercon_state = OSAL_SERCON_STATE_CONNECTED_5;
                    ioc_unlock(root);
                    return OSAL_SUCCESS;
                }

                /* If we received something else but confirm, not good.
                   Go back to waiting for CONNECT charcter.
                 */
                if (n_read)
                {
                    con->sercon_state = OSAL_SERCON_STATE_INIT_3;
                }
                break;
        }
    }

    /* Still establishing serial connection.
     */
    ioc_unlock(root);
    return OSAL_PENDING;
}


#endif
