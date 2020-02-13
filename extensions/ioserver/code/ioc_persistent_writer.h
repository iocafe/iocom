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

/* Persistent writer object structure.
 */
typedef struct iocPersistentWriter
{
    /* Memory for buffer has been allocated by os_malloc
     */
    os_boolean buf_allocated;

    /* Data buffer to write. Either direct pointer to flash, or buffer allocated by os_malloc().
     */
    os_char *buf;
    os_memsz buf_sz;

    /* Writing stream
     */
    iocStream *stream;
}
iocPersistentWriter;


/* Get data to and start writing.
 */
iocPersistentWriter *ioc_start_persistent_writer(
    osPersistentBlockNr default_block_nr,
    const os_char *dir,
    const os_char *file_name,
    iocMemoryBlock *mblk);

/* Release persistent writer object.
 */
void ioc_release_persistent_writer(
    iocPersistentWriter *wr);

/* Move the data.
 */
osalStatus ioc_run_persistent_writer(
    iocPersistentWriter *wr);

/* Check if certificate chain or flasg program is needed for some io device
 */
void ioc_upload_cert_chain_or_flash_prog(
    iocBServer *m);
