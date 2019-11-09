/**

  @file    ioc_com_status.c
  @brief   Communication status.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    29.5.2019

  Communication status refers to general communication information and settings. For example
  number of connections (sockets, etc) connected to a memory block. In future this could
  indicate which input data is selected in redundant communication, etc. Communication status
  may include also settings.

  From application's view communication status appears the same as data memory and is accessed
  using the same ioc_read(), ioc_getp_short(), ioc_write(), ioc_setp_short(), etc. functions. For data
  memory, the address is positive or zero, status memory addresses are negative.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#include "iocom.h"


/**
****************************************************************************************************

  @brief Write status data related to a memory block.
  @anchor ioc_status_write

  The ioc_status_write() function writes status data related to a memory block.

  @param   mblk Pointer to the memory block object.
  @param   addr Memory address to write to, always negative for status data.
  @param   buf Pointer to source data.
  @param   n Number of bytes to write.
  @return  None.

****************************************************************************************************
*/
void ioc_status_write(
    struct iocMemoryBlock *mblk,
    int addr,
    const os_char *buf,
    int n)
{
    IOC_MT_ROOT_PTR;

    os_int
        i,
        start_addr,
        end_addr;

    addr = IOC_STATUS_MEMORY_SZ + addr;
    if (addr < 0)
    {
        buf += -addr;
        n -= -addr;
        addr = 0;
    }

    if (addr + n > IOC_STATUS_MEMORY_SZ)
    {
        n = IOC_STATUS_MEMORY_SZ - addr;
    }

    if (n > 0)
    {
        /* Return if not changed.
         */
        for (i = 0; i < n; i++)
        {
            if (mblk->status[addr + i] != buf[i]) break;
        }
        if (i == n) return;

        ioc_set_mt_root(root, mblk->link.root);
        ioc_lock(root);

        /* Set the memory
         */
        os_memcpy(mblk->status + addr, buf, n);

        /* Do the callback.
         */
        start_addr = addr - IOC_STATUS_MEMORY_SZ;
        end_addr = start_addr + n - 1;
        for (i = 0; i < IOC_MBLK_MAX_CALLBACK_FUNCS; i++)
        {
            if (mblk->func[i])
            {
                mblk->func[i](&mblk->handle, start_addr, end_addr,
                    IOC_MBLK_CALLBACK_WRITE, mblk->context[i]);
            }
        }

        ioc_unlock(root);
    }
}


/**
****************************************************************************************************

  @brief Read data status data related to a memory block.
  @anchor ioc_status_read

  The ioc_status_read() function reads status data related to a memory block.

  @param   mblk Pointer to the memory block object.
  @param   addr Memory address to read from. Always negative for status data.
  @param   buf Pointer to buffer where to place data.
  @param   n Number of bytes to read.
  @return  None.

****************************************************************************************************
*/
void ioc_status_read(
    struct iocMemoryBlock *mblk,
    int addr,
    os_char *buf,
    int n)
{
    addr = IOC_STATUS_MEMORY_SZ + addr;
    if (addr < 0)
    {
        os_memclear(buf, n);
        buf += -addr;
        n -= -addr;
        addr = 0;
    }

    if (addr + n > IOC_STATUS_MEMORY_SZ)
    {
        os_memclear(buf, n);
        n = IOC_STATUS_MEMORY_SZ - addr;
    }

    if (n > 0)
    {
        os_memcpy(buf, mblk->status + addr, n);
    }
}


/**
****************************************************************************************************

  @brief Count number of connected streams (sockets, etc).
  @anchor ioc_count_connected_streams

  The ioc_count_connected_streams() function calculates number of connected streams, etc and
  stores the value in connection status for all memory blocks.

  @param   root Pointer to the connection root object.
  @param   incement_drop_count OS_TRUE to increment global connection drop count.
  @return  Number of connected streams (sockets, etc).

****************************************************************************************************
*/
void ioc_count_connected_streams(
    iocRoot *root,
    os_boolean incement_drop_count)
{
    iocConnection *con;
    iocMemoryBlock *mblk;
    int count;

    count = 0;
    ioc_lock(root);

    /* If we dropped a connection, add global grop count.
     */
    if (incement_drop_count)
    {
        root->drop_count++;
    }

    for (con = root->con.first;
         con;
         con = con->link.next)
    {
        if (con->connected) count++;
    }

    for (mblk = root->mblk.first;
         mblk;
         mblk = mblk->link.next)
    {
        ioc_setp_short(&mblk->handle, IOC_NRO_CONNECTED_STREAMS, count);
        ioc_setp_int(&mblk->handle, IOC_CONNECTION_DROP_COUNT, root->drop_count);
    }

    ioc_unlock(root);
}
