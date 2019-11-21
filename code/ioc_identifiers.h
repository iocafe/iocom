/**

  @file    ioc_identifiers.h
  @brief   Functions related to identifiers (names).
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.11.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/** IO path fields. Empty string means unspefied.
 */
typedef struct iocIdentifiers
{
    /** Signal name name, max 31 characters. Empty string = not selected, for example
        we are expecting memory block or device selection.
     */
    os_char signal_name[IOC_SIGNAL_NAME_SZ];

    /** Memory block name, max 15 characters. Empty string = any memory block or
        we were expecting device selection.
     */
    os_char mblk_name[IOC_NAME_SZ];

    /** Device name, max 15 characters.
     */
    os_char device_name[IOC_NAME_SZ];

    /** If there are multiple devices of same type (same device name),
        this identifies the device. 0 = any device number.
     */
    os_short device_nr;

    /** Network name. Empty string = any network.
     */
    os_char network_name[IOC_NETWORK_NAME_SZ];
}
iocIdentifiers;


/** What IO path given to ioc_iopath_to_identifiers() is expected to select.
 */
typedef enum
{
    IOC_EXPECT_SIGNAL,
    IOC_EXPECT_MEMORY_BLOCK,
    IOC_EXPECT_DEVICE,
    IOC_EXPECT_NETWORK
}
iocExpectIoPath;


/* Split IO path to separate identifiers.
*/
void ioc_iopath_to_identifiers(
    iocIdentifiers *identifiers,
    const os_char *iopath,
    iocExpectIoPath expect);

/* Get part of IO path.
 */
os_boolean ioc_get_part_of_iopath(
    const os_char **iopath,
    os_char *buf,
    os_memsz buf_sz);
