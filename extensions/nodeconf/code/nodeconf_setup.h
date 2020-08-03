/**

  @file    nodeconf_setup.h
  @brief   Data structures, defines and functions for accessing network node configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef NODECONF_SETUP_H_
#define NODECONF_SETUP_H_
#include "nodeconf.h"

struct iocNodeConf;

/* Flags for ioc_load_node_config():
   IOC_LOAD_PBNR_NODE_CONF = Optionally load node configuration overrides from
   separate memory block.
 */
#define IOC_LOAD_PBNR_NODE_CONF 1

/* Load network node configuration.
 */
void ioc_load_node_config(
    iocNodeConf *node,
    const os_char *default_config,
    os_memsz default_config_sz,
    os_int flags);

/* Release any memory allocated for node configuration.
 */
void ioc_release_node_config(
    iocNodeConf *node);

#endif
