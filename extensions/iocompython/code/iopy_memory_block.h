/**

  @file    iopy_memory_block.h
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


typedef struct
{
  PyObject_HEAD

  iocHandle mblk_handle;

  /* Flag indicating that IOCOM new memory block was created when the python
     memory block was set up.
   */
  os_boolean mblk_created;

  int number;
}
MemoryBlock;

extern PyTypeObject MemoryBlockType;


