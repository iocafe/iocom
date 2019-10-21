/**

  @file    iocdomain_main.h
  @brief   IO domain controller library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    19.9.2019

  The domain controller library listens for connections from IO devices and other IO comain
  controllers. Once an IO device connects to domain, memory maps for the device are created.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/**
****************************************************************************************************
    Parameters for The iodomain_start() function.
****************************************************************************************************
*/
typedef struct
{

    int dulle;
} 
iodomainParams;


/**
****************************************************************************************************
    IO domain class structure.
****************************************************************************************************
*/
typedef struct
{
    iocRoot root;
}
iodomainClass;


/* Initialize the IO domain data structure.
 */
void iodomain_initialize(
    iodomainClass *iodomain);

/* Finished with IO domain. Clean up.
*/
void iodomain_shutdown(
    iodomainClass *iodomain);

/* Set up and start IO domain.
*/
void iodomain_start(
    iodomainClass *iodomain,
    iodomainParams *prm);
