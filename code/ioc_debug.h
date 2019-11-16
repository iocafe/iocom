/**

  @file    ioc_debug.h
  @brief   Debugging macros.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.7.2018

  Macros for making debug code more readable. If OSAL_DEBUG define is zero, empty macros will 
  be defined and do not generate any code.

  Debug identifier must be first item in the object structure. It is used to verify
  that a function argument is pointer to correct initialized object.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


#if OSAL_DEBUG == 0
  #define IOC_DEBUG_ID
  #define IOC_SET_DEBUG_ID(o,c)

#else
  #define IOC_DEBUG_ID os_char debug_id;
  #define IOC_SET_DEBUG_ID(o,c) (o)->debug_id = c;

#endif
