/**

  @file    ioc_parameters.c
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

/**
****************************************************************************************************

  @brief Initialize and set up parameter storage structure.
  @anchor ioc_initialize_parameters

  The ioc_initialize_parameters() function clears memory allocated for parameter storage structure
  and for persistent/volatile buffer data structures and stores pointers to buffer stuctures
  within parameter storage.

  This function must be called before parameter storage is used.

  @param   ps Pointer to parameter storage structure to initialize.
  @param   prm Parameters for this function.
  @return  None.

****************************************************************************************************
*/
void ioc_initialize_parameters(
    iocParameterStorage *ps,
    iocParameterStorageInitParams *prm)
{
    os_memclear(ps, sizeof(iocParameterStorage));
    os_memcpy(&ps->prm, prm, sizeof(iocParameterStorageInitParams));
    if (prm->persistent_prm) {
        os_memclear(prm->persistent_prm, prm->persistent_prm_sz);
    }
    if (prm->volatile_prm) {
        os_memclear(prm->volatile_prm, prm->volatile_prm_sz);
    }
}


/* Load persistant parameters from storage (flash, EEPROM, SSD, etc).
 */
osalStatus ioc_load_parameters(
    iocParameterStorage *ps)
{
    return OSAL_SUCCESS;
}

/* Save persistant parameters from storage.
 */
osalStatus ioc_save_parameters(
    iocParameterStorage *ps)
{
    return OSAL_SUCCESS;
}


/* Set parameter value by signal (used from communication callback)
 * @return  OSAL_COMPLETED indicates change, OSAL_NOTHING_TO_DO = no change.
 */
osalStatus ioc_set_parameter_by_signal(
    const struct iocSignal *sig)
{
    os_long x;
    os_char state_bits, *src;
    osalTypeId type;
    os_memsz type_sz;
    os_double d;
    os_float f;
    osalStatus s = OSAL_NOTHING_TO_DO;

    osal_debug_assert(sig->flags & IOC_PFLAG_IS_PRM);

    type = (sig->flags & OSAL_TYPEID_MASK);
    type_sz = osal_type_size(type);
    if (sig->n <= 1) {
        switch (type)
        {
            case OS_FLOAT:
                f = (os_float)ioc_get_double_ext(sig, &state_bits, IOC_SIGNAL_DEFAULT);
                if (state_bits & OSAL_STATE_CONNECTED)
                {
                    if (f != *(os_float*)sig->ptr) {
                        *(os_float*)sig->ptr = f;
                        s = OSAL_COMPLETED;
                    }
                }
                break;

            case OS_DOUBLE:
                d = ioc_get_double_ext(sig, &state_bits, IOC_SIGNAL_DEFAULT);
                if (state_bits & OSAL_STATE_CONNECTED)
                {
                    if (d != *(os_double*)sig->ptr) {
                        *(os_double*)sig->ptr = d;
                        s = OSAL_COMPLETED;
                    }
                }
                break;

            default:
                x = ioc_get_ext(sig, &state_bits, IOC_SIGNAL_DEFAULT);
                if (state_bits & OSAL_STATE_CONNECTED)
                {
#if OSAL_SMALL_ENDIAN
                    src = (os_char*)&x;
#else
                    src = (os_char*)&x + sizeof(os_long) - type_sz;
#endif
                    if (os_memcmp(sig->ptr, src, type_sz))
                    {
                        os_memcpy((os_char*)sig->ptr, src, type_sz);
                        s = OSAL_COMPLETED;
                    }
                }
                break;
        }
    }
    else {
        switch (type)
        {
            case OS_STR:
                break;

            default:
                break;
        }
    }


    return s;
}


#endif
