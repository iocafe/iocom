/**

  @file    ioc_nickgen.c
  @brief   Nick name generator.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_NICKGEN_SUPPORT

/* Nick name is simply two random 2/3 three letter words put together.
 */
OS_CONST os_char *ioc_nick1[] = {"hey", "ant", "jam", "bot", "ink", "mic", "dog",
    "red", "god", "oak", "air", "fir", "two", "top", "awe", "blu", "woo", "hoo",
    "eye", "ear", "rat", "dot", "meg", "dir", "cat", "how", "pin", "pot",
    "hat", "but", "gut", "gun", "nut", "tap", "bra", "leg", "jaw", "kit", "pit",
    "hot", "jar", "egg", "sun", "run", "pun", "mad", "min", "max", "ice", "hug",
    "hog", "ham", "meg", "get", "git", "put", "set", "you", "bug", "rug", "jim",
    "gin", "bob", "ace", "xi", "ox", "old", "one", "two", "yin", "ape", "bet",
    "daw", "dow", "cop", "elk", "eel", "fun", "cut", "hex", "guy", "pea", "zoo",
    "sad", "it", "we", "me", "ax", OS_NULL};


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

    i = (os_int)osal_rand(0, 100000);
    return nick_part[i%n];
}


/**
****************************************************************************************************

  @brief Generate a nick name
  @anchor ioc_generate_nickname

  @param   buf Pointer to buffer where store the nick name. Recommended size IOC_NAME_SZ bytes.
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
    n = ioc_get_nick_part(ioc_nick1);
    os_strncat(buf, n, buf_sz);
    os_strncat(buf, "-", buf_sz);
    os_strncat(buf, OSAL_BIN_NAME, buf_sz);
}

#endif
