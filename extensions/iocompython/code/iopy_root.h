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

typedef struct
{
    PyObject_HEAD

    /** IOCOM root object pointer.
     */
    iocRoot *root;

    /** Network name.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /** Device name, max 15 upper case characters from 'A' - 'Z'. This
        identifies IO device type, like "TEMPCTRL".
     */
    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. This number is often written in
        context as device name, like "TEMPCTRL1".
     */
    int device_nr;

    int number;
}
Root;

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


