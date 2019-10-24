/**

  @file    iopy_memory_block.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


typedef struct
{
  PyObject_HEAD

  iocMemoryBlock *mblk;

  int number;
}
MemoryBlock;

extern PyTypeObject MemoryBlockType;

#if 0
/* Initialize memory block object.
 */
iocMemoryBlock *ioc_initialize_memory_block(
    iocMemoryBlock *mblk,
    iocRoot *root,
    iocMemoryBlockParams *prm);

/* Release memory block object.
 */
void ioc_release_memory_block(
    iocMemoryBlock *mblk);

/* Modify memory block flag (IOC_AUTO_RECEIVE or IOC_AUTO_SEND)
 */
os_boolean ioc_set_flag(
    iocMemoryBlock *mblk,
    int flag,
    os_boolean set);

/* Get memory block parameter value.
 */
os_int ioc_get_memory_block_param(
    iocMemoryBlock *mblk,
    iocMemoryBlockParamIx param_ix,
    os_char *buf,
    os_memsz buf_sz);

/* Write data to memory block.
 */
void ioc_write(
    iocMemoryBlock *mblk,
    int addr,
    const os_uchar *buf,
    int n);

/* Read data from memory block.
 */
void ioc_read(
    iocMemoryBlock *mblk,
    int addr,
    os_uchar *buf,
    int n);

/* Send data synchronously.
 */
void ioc_send(
    iocMemoryBlock *mblk);

/* Receive data synchronously.
 */
void ioc_receive(
    iocMemoryBlock *mblk);

/* Add callback function.
 */
void ioc_add_callback(
    iocMemoryBlock *mblk,
    ioc_callback func,
    void *context);
#endif
