/**

  @file    ioc_dyn_iface.c
  @brief   Interface to dynamic IO network objects.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Implementation of dynamic IO network presentation can depend on use, for example if we
  compile iocom with eobjects library, we want to use eobjects data abstraction. If we
  run with QT, we may want to use QT objects, etc.
  Simple default implementation of dynamic IO network objects is used by default, if
  alternate implementation is not provided.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocom.h"
#if IOC_DYNAMIC_MBLK_CODE
#if IOC_ABSTRACT_DYNAMIC_MBLK_SUPPORT



OS_CONST iocDynamicInterface ioc_default_dynamic_iface = {

    (abstract_add_dynamic_network*)ioc_add_dynamic_network,
    (abstract_remove_dynamic_network*)ioc_remove_dynamic_network

};



/**
****************************************************************************************************

  @brief Allocate and clear dynamic network object.
  @anchor ioc_gen_add_dynamic_network

  The ioc_initialize_dynamic_network() function just allocates clear memory for dynamic IO device
  network structure. This function is called by ioc_add_dynamic_network(), which is used to
  initiate storing information about new dynamic network.

  @return  Pointer to newly allocated dynamic network structure, or OS_NULL if memory allocation
           failed..

****************************************************************************************************
*/
iocAbstractDynamicNetwork *ioc_gen_add_dynamic_network(
    iocAbstractDynamicRoot *droot,
    const os_char *network_name)
{
    osal_debug_assert(droot != OS_NULL);
    osal_debug_assert(droot->iface != OS_NULL);
    osal_debug_assert(droot->iface->add_dynamic_network != OS_NULL);
    return droot->iface->add_dynamic_network(droot, network_name);
}

#endif
#endif
