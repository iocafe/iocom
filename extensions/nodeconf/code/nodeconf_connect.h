/**

  @file    nodeconf_connect.h
  @brief   Create connections and end points by node configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    15.1.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#pragma once
#ifndef NODECONF_CONNECT_H_
#define NODECONF_CONNECT_H_
#include "nodeconf.h"

osalStatus ioc_connect_node(
    iocRoot *root,
    iocConnectionConfig *connconf,
    os_short additional_flags);

#endif
