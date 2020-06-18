/**

  @file    ioc_parameters.h
  @brief   Persistent and volatile IO device parameters
  @author  Pekka Lehtikoski
  @version 1.0
  @date    18.6.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"

#if IOC_DEVICE_PARAMETER_SUPPORT

struct iocSignal;

typedef struct iocParameterStorageInitParams
{
    /* Persistent block number. */
    os_int block_nr;

    /* Pointer to persistent data structure.
     */
    void *persistent_prm;

    /* Peristent data structure size in bytes.
     */
    os_memsz persistent_prm_sz;

    /* Pointer to volatile data structure.
     */
    void *volatile_prm;

    /* Volatile data structure size in bytes.
     */
    os_memsz volatile_prm_sz;
}
iocParameterStorageInitParams;


typedef struct iocParameterStorage
{
    iocParameterStorageInitParams prm;

}
iocParameterStorage;



/* Initialize persistent storage and store pointers to persistant and volatile structures.
 */
void ioc_initialize_parameters(
    iocParameterStorage *ps,
    iocParameterStorageInitParams *prm);

/* Load persistant parameters from storage (flash, EEPROM, SSD, etc).
 */
osalStatus ioc_load_parameters(
    iocParameterStorage *ps);

/* Save persistant parameters from storage.
 */
osalStatus ioc_save_parameters(
    iocParameterStorage *ps);

/* Set parameter value by signal (used from communication callback)
 */
osalStatus ioc_set_parameter_by_signal(
    const struct iocSignal *sig);


#endif
