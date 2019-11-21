/**

  @file    ioc_identifiers.c
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
#include "iocom.h"


/**
****************************************************************************************************

  @brief Split IO path to separate identifiers.
  @anchor ioc_iopath_to_identifiers

  The ioc_iopath_to_identifiers() function is used to split IO path to identifiers. Argument
  expect selects what we expect from IO path, do we expect a signal, memory block, device
  or network selection.

  @param   identifiers Structure to fill in with separate identifiers. If something is not
           specified by path, it is set to empty string or 0 in case of device number.
  @param   iopath Path to split
  @param   expect What to expect from IO path. One of IOC_EXPECT_SIGNAL, IOC_EXPECT_MEMORY_BLOCK,
           IOC_EXPECT_DEVICE or IOC_EXPECT_NETWORK.
  @return  None.

****************************************************************************************************
*/
void ioc_iopath_to_identifiers(
    iocIdentifiers *identifiers,
    const os_char *iopath,
    iocExpectIoPath expect)
{
    os_char *p, *e, *ee;
    os_boolean has_more;

    /* Clear the identifier structure.
     */
    os_memclear(identifiers, sizeof(iocIdentifiers));
    if (iopath == OS_NULL) return;

    if (expect == IOC_EXPECT_SIGNAL)
    {
        if (!ioc_get_part_of_iopath(&iopath, identifiers->signal_name, IOC_SIGNAL_NAME_SZ)) return;
        expect = IOC_EXPECT_MEMORY_BLOCK;
    }

    if (expect == IOC_EXPECT_MEMORY_BLOCK)
    {
        if (!ioc_get_part_of_iopath(&iopath, identifiers->mblk_name, IOC_NAME_SZ)) return;
        expect = IOC_EXPECT_DEVICE;
    }

    if (expect == IOC_EXPECT_DEVICE)
    {
        has_more = ioc_get_part_of_iopath(&iopath, identifiers->device_name, IOC_NAME_SZ);

        /* Get device number, if any
         */
        p = identifiers->device_name;
        e = ee = os_strchr(p, '\0');
        while (e > p)
        {
            if (!osal_char_isdigit(e[-1])) break;
            e--;
        }
        if (e != ee)
        {
            identifiers->device_nr = (os_short)osal_str_to_int(e, OS_NULL);
            *e = '\0';
        }

        if (!has_more) return;
    }

    os_strncpy(identifiers->network_name, iopath, IOC_NETWORK_NAME_SZ);
}


/**
****************************************************************************************************

  @brief Get part of IO path.
  @anchor ioc_iopath_to_identifiers

  The ioc_get_part_of_iopath() function stores first part of IO path into buffer, what is before
  the first dot '.'. If dot was not found, the whole IO path is stored. Number of bytes
  stored is limited to buffer size.

  @param   iopath Pointer to pointer to IO path. This pointer is moved by function to past
           stored part.
  @param   buf Buffer where to store the part.
  @param   buf_sz Buffer size in bytes to avoid overlows.
  @return  OS_TRUE if part included dot and has still more information to follow.

****************************************************************************************************
*/
os_boolean ioc_get_part_of_iopath(
    const os_char **iopath,
    os_char *buf,
    os_memsz buf_sz)
{
    const os_char *b, *p, *e;
    os_memsz n;

    b = *iopath;
    p = os_strchr((os_char*)b, '.');
    e = p ? p : strchr((os_char*)b, '\0');
    n = (e - b) + 1;
    if (n > buf_sz) n = buf_sz;
    os_strncpy(buf, b, n);
    if (!os_strcmp(buf, "*")) buf[0] = '\0';

    *iopath = e + 1;
    return p ? OS_TRUE : OS_FALSE;
}
