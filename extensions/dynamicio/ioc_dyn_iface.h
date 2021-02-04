/**

  @file    ioc_dyn_iface.h
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
#pragma once
#ifndef IOC_DYN_IFACE_H_
#define IOC_DYN_IFACE_H_
#include "iocomx.h"

#if IOC_DYNAMIC_MBLK_CODE
#if IOC_ABSTRACT_DYNAMIC_MBLK_SUPPORT

struct iocDynamicInterface;

typedef struct iocAbstractDynamicRoot
{
    const struct iocDynamicInterface *iface;
}
iocAbstractDynamicRoot;

typedef struct iocAbstractDynamicNetwork
{
}
iocAbstractDynamicNetwork;

typedef iocAbstractDynamicNetwork *abstract_add_dynamic_network(
    iocAbstractDynamicRoot *droot,
    const os_char *network_name);

typedef void abstract_remove_dynamic_network(
    iocAbstractDynamicRoot *droot,
    iocAbstractDynamicNetwork *dnetwork);

typedef osalStatus abstract_add_dynamic_info(
    iocAbstractDynamicRoot *droot,
    iocHandle *mblk_handle,
    os_boolean resize_mblks);

typedef void abstract_dynamic_mblk_is_deleted(
    iocAbstractDynamicRoot *droot,
    iocMemoryBlock *mblk);



/**
****************************************************************************************************
    Interface to dynamic functions.
****************************************************************************************************
*/

/** The dynamic root class structure.
 */
typedef struct iocDynamicInterface
{
    /* Add an IO device network to dynamic information.
     */
    abstract_add_dynamic_network *add_dynamic_network;

    /* Remove a dynamic network.
     */
    abstract_remove_dynamic_network *remove_dynamic_network;

    /* Add dynamic memory block/signal information.
     */
    abstract_add_dynamic_info *add_dynamic_info;

    /* Memory block is being deleted, remove any references to it from dynamic configuration.
     */
    abstract_dynamic_mblk_is_deleted *dynamic_mblk_is_deleted;
}
iocDynamicInterface;


/* Interface to default iocom implementation of dynamic IO network objects.
 */
extern OS_CONST_H iocDynamicInterface ioc_default_dynamic_iface;


iocAbstractDynamicNetwork *ioc_gen_add_dynamic_network(
    iocAbstractDynamicRoot *droot,
    const os_char *network_name);

void ioc_gen_remove_dynamic_network(
    iocAbstractDynamicRoot *droot,
    iocAbstractDynamicNetwork *dnetwork);

osalStatus ioc_gen_add_dynamic_info(
    iocAbstractDynamicRoot *droot,
    iocHandle *mblk_handle,
    os_boolean resize_mblks);

void ioc_gen_dynamic_mblk_is_deleted(
    iocAbstractDynamicRoot *droot,
    iocMemoryBlock *mblk);


#else
    /* IOC_ABSTRACT_DYNAMIC_MBLK_SUPPORT is zero -> Map ioc_gen_* functions and
       pointers directly to default implementations.
     */
    #define iocAbstractDynamicRoot iocDynamicRoot
    #define iocAbstractDynamicNetwork iocDynamicNetwork

    #define ioc_gen_add_dynamic_network ioc_add_dynamic_network
    #define ioc_gen_remove_dynamic_network ioc_remove_dynamic_network
    #define ioc_gen_add_dynamic_info ioc_add_dynamic_info
    #define ioc_gen_dynamic_mblk_is_deleted ioc_dynamic_mblk_is_deleted


#endif
#endif
#endif
