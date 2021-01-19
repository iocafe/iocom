/**

  @file    ioc_nickgen.c
  @brief   Nick name generator.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    10.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_NICKGEN_SUPPORT

/* Parts of nick name
 */
OS_FLASH_MEM os_char *ioc_nick1[] = {"low", "ho", "go", "meg", "max", "go", "gin", "bin", "dow", "bit", "do", OS_NULL};
OS_FLASH_MEM os_char *ioc_nick2[] = {"hawk", "ant", "yellow", "bone", "ink", "mighty", "dog", "red", "god", "oak", "air", "two", OS_NULL};
OS_FLASH_MEM os_char *ioc_nick3[] = {"eye", "ear", "hand", "belly", "feet", "brain", "nose", "but", "gut", "nut", OS_NULL};


/**
****************************************************************************************************

  @brief Helper function to generate nick name
  @anchor ioc_get_nick_part

  @param   nick_part List of choices to select one.
  @return  Pointer to choice selected at random.

****************************************************************************************************
*/
static const os_char *ioc_get_nick_part(
    const os_char **nick_part)
{
    const os_char **name;
    os_int n, i;

    n = 0;
    for (name = nick_part; *name; name++) n++;

    i = (os_int)osal_rand(0, n-1);
    return nick_part[i];
}


/**
****************************************************************************************************

  @brief Generate a nick name
  @anchor ioc_generate_nickname

  @param   buf Pointer to buffer where store the nick name.
  @param   buf_sz Buffer size in bytes.

****************************************************************************************************
*/
void ioc_generate_nickname(
    os_char *buf,
    os_memsz buf_sz)
{
    const os_char *n;

    n = ioc_get_nick_part(ioc_nick1);
    os_strncpy(buf, n, buf_sz);
    os_strncat(buf, "-", buf_sz);
    n = ioc_get_nick_part(ioc_nick2);
    os_strncat(buf, n, buf_sz);
    n = ioc_get_nick_part(ioc_nick3);
    os_strncat(buf, n, buf_sz);
}

#endif
