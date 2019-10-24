/**

  @file    iopy_root.h
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

typedef struct {
  PyObject_HEAD // no semicolon
  int number;
} Root;

extern PyTypeObject RootType;


#if 0
/* Initialize communication root object.
 */
void ioc_initialize_root(
    iocRoot *root);

/* Release communication root object.
 */
void ioc_release_root(
    iocRoot *root);

/* Set callback function for iocRoot object.
 */
void ioc_set_root_callback(
    iocRoot *root,
    ioc_root_callback func,
    void *context);
#endif


