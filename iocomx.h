/**

  @file    iocomx.h
  @brief   Main iocom header with extensions.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    29.7.2018

  This iocom main header file with extensions. If further iocom base and extension headers.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/
#ifndef IOCOMX_INCLUDED
#define IOCOMX_INCLUDED

/* Include iocom base.
 */
#include "iocom.h"

/* If C++ compilation, all functions, etc. from this point on in included headers are
   plain C and must be left undecorated.
 */
OSAL_C_HEADER_BEGINS

/* Include extension headers.
 */
/* #include "iocom/extensions/ioc_ssl/ioc_ssl.h" */

/* If C++ compilation, end the undecorated code.
 */
OSAL_C_HEADER_ENDS

#endif
