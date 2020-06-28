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

iocParameterStorage ioc_prm_storage;

/**
****************************************************************************************************

  @brief Set parameter value by signal (used from communication callback)
  @anchor ioc_set_parameter_by_signal

  The ioc_set_parameter_by_signal()

  @param   sig Changed signal.
  @param   pin_sig Pointer to set to point value signal if this is to be forwarded to pin.
  @return  OSAL_COMPLETED indicates change, OSAL_NOTHING_TO_DO = no change. OS_NULL if not needed.

****************************************************************************************************
*/
osalStatus ioc_set_parameter_by_signal(
    const struct iocSignal *sig,
    const struct iocSignal **pin_sig)
{
    os_char *buf1ptr, *buf2ptr, *tmpbuf = OS_NULL;
    os_char buf1[64], buf2[62];
    iocSignal *dsig;
    osalTypeId type;
    os_memsz type_sz, sz, n;
    osalStatus s = OSAL_NOTHING_TO_DO;

    osal_debug_assert(sig->flags & IOC_PFLAG_IS_PRM);

    if (pin_sig) {
        *pin_sig = OS_NULL;
    }
    dsig = (iocSignal*)sig->ptr;
    if (dsig == OS_NULL) return OSAL_STATUS_FAILED;

    type = (sig->flags & OSAL_TYPEID_MASK);
    type_sz = osal_type_size(type);
    n = sig->n;
    if (n < 1) n = 1;
    if (type == OS_BOOLEAN) {
        sz = 1;
        if (n > 1) sz += (n + 7)/8;
    }
    else
    {
        sz = n * type_sz + 1;
    }

    buf1ptr = buf1;
    buf2ptr = buf2;
    if (sz > sizeof(buf1)) {
        tmpbuf = os_malloc(2*sz, OS_NULL);
        if (tmpbuf== OS_NULL)
        {
            s = OSAL_STATUS_MEMORY_ALLOCATION_FAILED;
            goto getout;
        }
        buf1ptr = tmpbuf;
        buf2ptr = buf1ptr + sz;
    }
    ioc_read(sig->handle, sig->addr, buf1ptr, (os_int)sz, 0);
    if (buf1ptr[0] & OSAL_STATE_CONNECTED)
    {
        ioc_read(dsig->handle, dsig->addr, buf2ptr, (os_int)sz, 0);
        if (os_memcmp(buf1ptr, buf2ptr, sz))
        {
            ioc_write(dsig->handle, dsig->addr, buf1ptr, (os_int)sz, 0);
            s = OSAL_COMPLETED;

            if ((dsig->flags & IOC_PIN_PTR) && dsig->ptr && pin_sig) {
                *pin_sig =  dsig;
            }
        }
    }

    if (s == OSAL_COMPLETED && (sig->flags & IOC_PFLAG_IS_PERSISTENT)) {
        if (!ioc_prm_storage.changed)
        {
            os_get_timer(&ioc_prm_storage.ti);
            ioc_prm_storage.changed = OS_TRUE;
        }
    }

getout:
    os_free(tmpbuf, sz);
    return s;
}

#endif
