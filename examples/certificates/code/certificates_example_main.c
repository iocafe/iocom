/**

  @file    iocom/examples/certificates/certificates_example_main.c
  @brief   Example and unit tests for certificate utilities.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    26.4.2021

  Copyright 2020 Pekka Lehtikoski. This file is part of the eosal and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "certificates_example_main.h"

/* If needed for the operating system, EOSAL_C_MAIN macro generates the actual C main() function.
 */
EOSAL_C_MAIN

/**
****************************************************************************************************

  @brief Process entry point.

  The osal_main() function is OS independent entry point.

  @param   argc Number of command line arguments.
  @param   argv Array of string pointers, one for each command line argument. UTF8 encoded.

  @return  None.

****************************************************************************************************
*/
osalStatus osal_main(
    os_int argc,
    os_char *argv[])
{
    osPersistentParams persistentprm;
    osalSecurityConfig security_prm;
    os_memclear(&security_prm, sizeof(security_prm));

    /* Initialize persistent storage
     */
    os_memclear(&persistentprm, sizeof(persistentprm));
    persistentprm.subdirectory = "exampledata";
    os_persistent_initialze(&persistentprm);

    /* Initialize TLS library.
     */
    osal_tls_initialize(OS_NULL, 0, OS_NULL, 0, &security_prm);

    /* Call real example functionality.
     */
    my_generate_root_key();
    my_generate_root_certificate();
    my_generate_server_key();
    my_generate_certificate_request();

    return OSAL_SUCCESS;
}


/*  Empty function implementation needed to build for microcontroller.
 */
osalStatus osal_loop(
    void *app_context)
{
    OSAL_UNUSED(app_context);
    return OSAL_SUCCESS;
}

/*  Empty function implementation needed to build for microcontroller.
 */
void osal_main_cleanup(
    void *app_context)
{
    OSAL_UNUSED(app_context);
}
