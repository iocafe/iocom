/**

  @file    nodeconf_persistent.h
  @brief   Data structures, defines and functions for managing network node configuration and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


/** X
*/
void ioc_load_node_config(
    iocNodeConf *node,
    const os_char *default_config);

void ioc_save_node_config(
    iocNodeConf *node);

/* void ioc_release_node_config(
    iocNodeConf *node);
 */
