/**

  @file    ioc_server_write_persistent.h
  @brief   Write persistent block to IO device
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.2.2020

  Write persistent block, like flash program or certificate chain, to an IO device.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "ioserver.h"


/**
****************************************************************************************************

  @brief Get data to and start writing.
  @anchor ioc_start_persistent_writer

  The ioc_start_persistent_writer() function gets source data and starts writing data to
  the IO device.

  @param   default_block_nr If reading from persistent storage, this is default block
           number for the case when file name doesn't specify one.
  @param   dir Directory from where files are read, if using file system.
  @param   file_name Specifies file name or persistent block number.
  @param   mblk Pointer to memory block. Used to get path to device, thus any device's
           memory block will do.

  @return  Pointer to persistent writer object, or OSAL_NULL if the function failed.
           The persistant writer object returned by this function must be released by
           ioc_release_persistent_writer() call.

****************************************************************************************************
*/
iocPersistentWriter *ioc_start_persistent_writer(
    osPersistentBlockNr default_block_nr,
    const os_char *dir,
    const os_char *file_name,
    iocMemoryBlock *mblk)
{
    iocPersistentWriter *wr;
    iocStream *stream;
    osalStatus s;
    os_char *buf;
    os_memsz n_read;
    os_int select;

select = OS_PBNR_CLIENT_CERT_CHAIN; // Block number on target IO device

    stream = ioc_open_stream(mblk->link.root, select, "frd_buf", "tod_buf", "conf_exp", "conf_imp",
        mblk->device_name, mblk->device_nr, mblk->network_name, IOC_IS_CONTROLLER);
    if (stream == OS_NULL)
    {
        osal_error(OSAL_WARNING, iocom_mod, OSAL_STATUS_FAILED,
            "opening upload stream to IO device failed");
        return OS_NULL;
    }

    /* Get data from persistent block or from file.
     */
    s = osal_get_persistent_block_or_file(default_block_nr, dir,
        file_name, &buf, &n_read, OS_FILE_NULL_CHAR);
    if (OSAL_IS_ERROR(s)) {
        osal_error(OSAL_WARNING, iocom_mod, s, "no data to upload");
        ioc_release_stream(stream);
        return OS_NULL;
    }

    /* Allocate and initialize writer object.
     */
    wr = (iocPersistentWriter*)os_malloc(sizeof(iocPersistentWriter), OS_NULL);
    if (wr == OS_NULL)
    {
        if (s == OSAL_MEMORY_ALLOCATED) {
            os_free(buf, n_read);
        }
        ioc_release_stream(stream);
        return OS_NULL;
    }
    os_memclear(wr, sizeof(iocPersistentWriter));
    wr->buf_allocated = (os_boolean) (s == OSAL_MEMORY_ALLOCATED);
    wr->buf = buf;
    wr->buf_sz = n_read;
    wr->stream = stream;

    ioc_start_stream_write(stream, buf, n_read, OS_FALSE);
    return wr;
}


/**
****************************************************************************************************

  @brief Release persistent writer object.
  @anchor ioc_release_persistent_writer

  The ioc_release_persistent_writer() function releases persistent writer object and all
  resources allocated for it.

  @param   wr Pointer to persistent writer object.
  @return  None.

****************************************************************************************************
*/
void ioc_release_persistent_writer(
    iocPersistentWriter *wr)
{
    if (wr == OS_NULL) return;

    ioc_release_stream(wr->stream);

    if (wr->buf_allocated) {
        os_free(wr->buf, wr->buf_sz);
    }

    os_free(wr, sizeof(iocPersistentWriter));
}


/**
****************************************************************************************************

  @brief Move the data.
  @anchor ioc_run_persistent_writer

  The ioc_run_persistent_writer() function moves data to the IO device.

  @param   wr Pointer to persistent writer object.
  @return  OSAL_SUCCESS if there is still stuff to write. OSAL_COMPLETED if all is done and
           ioc_release_persistent_writer() needs to be called. Other values indicate an error (still
           call ioc_release_persistent_writer().

****************************************************************************************************
*/
osalStatus ioc_run_persistent_writer(
    iocPersistentWriter *wr)
{
    osalStatus s;

    s = ioc_run_stream(wr->stream, IOC_CALL_SYNC);
    if (OSAL_IS_ERROR(s))
    {
        osal_error(OSAL_WARNING, iocom_mod, s, "upload to IO device failed");
    }
    return s;
}


/**
****************************************************************************************************

  @brief X
  @anchor X

  If a device without certificate chain has been connected, the connection has IOC_NO_CERT_CHAIN
  flag set. This function checks for those flags and initiates the certificate transfer.

  This function may be upgrader in future to automatically upload flash program to IO device
  if newer version has been copied to server.

  @param   wr Pointer to persistent writer object.
  @return  None.

****************************************************************************************************
*/
void ioc_upload_cert_chain_or_flash_prog(
    iocBServer *m)
{
    iocConnection *con;
    iocTargetBuffer *tbuf;
    iocMemoryBlock *mblk;
    osalStatus s;

    /* If we have persistent writer, keep on writing.
     */
    if (m->persistent_writer)
    {
        s = ioc_run_persistent_writer(m->persistent_writer);
        if (s)
        {
            ioc_release_persistent_writer(m->persistent_writer);
            m->persistent_writer = OS_NULL;
        }
        return;
    }

    /* If we are not triggered to scan for updates, we have nothing to do.
     */
    if (!m->check_cert_chain_etc) return;

    /* Synchronize.
     */
    ioc_lock(m->root);

    /* If we have connection which has no serfiticate chain (or maybe in future needs a flash software update)
     */
    for (con = m->root->con.first;
         con;
         con = con->link.next)
    {
        if (con->flags & IOC_NO_CERT_CHAIN)
        {
            for (tbuf = con->tbuf.first; tbuf; tbuf = tbuf->clink.next)
            {
                mblk = tbuf->mlink.mblk;

                if (!os_strcmp(mblk->mblk_name, "info"))
                {
                    m->persistent_writer = ioc_start_persistent_writer(OS_PBNR_CLIENT_CERT_CHAIN,
                        OS_NULL, "myhome-bundle.crt", mblk);

                    break;
                }
            }

            con->flags &= ~IOC_NO_CERT_CHAIN;
            break;
        }
    }

    /* End synchronization.
     */
    ioc_unlock(m->root);

    /* If we dodn't find connection to process.
     */
    if (con == OS_NULL)
    {
        m->check_cert_chain_etc = OS_FALSE;
    }
}
