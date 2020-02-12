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
  @oaram   file_name Specifies file name or persistent block number.

  @return  Pointer to persistent writer object, or OSAL_NULL if the function failed.
           The persistant writer object returned by this function must be released by
           ioc_release_persistent_writer() call.

****************************************************************************************************
*/
iocPersistentWriter *ioc_start_persistent_writer(
    osPersistentBlockNr default_block_nr,
    const os_char *dir,
    const os_char *file_name)
{
    iocPersistentWriter *wr;
    osalStatus s;
    os_char *buf;
    os_memsz n_read;

    /* If communication is busy writing something else?
     */
/*     stream = ioc_open_stream(
        iocroot, select, frd_buf_name, tod_buf_name, exp_mblk_path, imp_mblk_path,
        OS_NULL, 0, OS_NULL, os_strstr(flags, "device",
        OSAL_STRING_SEARCH_ITEM_NAME) ?  IOC_IS_DEVICE : IOC_IS_CONTROLLER); */


    /* Get data from persistent block or from file.
     */
    s = osal_get_persistent_block_or_file(default_block_nr, dir, file_name, &buf, &n_read);
    if (OSAL_IS_ERROR(s)) {
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
        return OS_NULL;
    }
    os_memclear(wr, sizeof(iocPersistentWriter));
    wr->buf_allocated = (os_boolean) (s == OSAL_MEMORY_ALLOCATED);
    wr->buf = buf;
    wr->buf_sz = n_read;

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

    return OSAL_COMPLETED;
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

return;

//    if (!root->check_cert_chain_or_prog_updates && m->wr == OS_NULL) return;


    /* If we have persistent writer, keep on writing.
     */
    if (m->persistent_writer)
    {

        return;
    }

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
            /* * iocPersistentWriter *ioc_start_persistent_writer(
                osPersistentBlockNr default_block_nr,
                    const os_char *dir,
                    const os_char *file_name) */

            con->flags &= ~IOC_NO_CERT_CHAIN;
        }
    }

    /* End synchronization.
     */
    ioc_unlock(m->root);
}




#if 0
****************************************************************************************************
  Global blocking function, set configuration.
****************************************************************************************************
*/
PyObject *iocom_stream_setconf(
    PyObject *self,
    PyObject *args,
    PyObject *kwds)
{
    const char
        *frd_buf_name = "frd_buf",
        *tod_buf_name = "tod_buf",
        *device_path = OS_NULL,
        *flags = "";

    int
        select = OS_PBNR_CONFIG;

    os_char
        exp_mblk_path[64],
        imp_mblk_path[64];

    osalStatus s;
    iocStream *stream;
    Root *root;
    iocRoot *iocroot;
    PyObject *pydata = NULL;
    char *buffer;
    Py_ssize_t length;
    int count = -1;
    int pos = 0;

    static char *kwlist[] = {
        "path",
        "data",
        "pos",
        "n",
        "select",
        "flags",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO|iiis",
         kwlist, &device_path, &pydata, &pos, &count, &select, &flags))
    {
        PyErr_SetString(iocomError, "Device path mydevice.mynetwork and byte data to send are expected as arguments.");
        return NULL;
    }

    root = (Root*)self;
    iocroot = root->root;
    if (iocroot == OS_NULL)
    {
        PyErr_SetString(iocomError, "IOCOM root object has been deleted");
        return NULL;
    }

    os_strncpy(exp_mblk_path, "conf_exp.", sizeof(exp_mblk_path));
    os_strncat(exp_mblk_path, device_path, sizeof(exp_mblk_path));
    os_strncpy(imp_mblk_path, "conf_imp.", sizeof(imp_mblk_path));
    os_strncat(imp_mblk_path, device_path, sizeof(imp_mblk_path));

    stream = ioc_open_stream(
        iocroot, select, frd_buf_name, tod_buf_name, exp_mblk_path, imp_mblk_path,
        OS_NULL, 0, OS_NULL, os_strstr(flags, "device",
        OSAL_STRING_SEARCH_ITEM_NAME) ?  IOC_IS_DEVICE : IOC_IS_CONTROLLER);

    PyBytes_AsStringAndSize(pydata, &buffer, &length);
    if (count < 0) count = length;
    if (pos + count > length) count = length - pos;
    if (count < 0) count = 0;
    ioc_start_stream_write(stream, buffer + pos, count);

    while ((s = ioc_run_stream(stream, IOC_CALL_SYNC)) == OSAL_SUCCESS && osal_go())
    {
        Py_BEGIN_ALLOW_THREADS
        os_timeslice();
        Py_END_ALLOW_THREADS
    }

    ioc_release_stream(stream);
    return Py_BuildValue("s", s == OSAL_COMPLETED ? "completed" : "failed");
}
#endif
