/**

  @file    iotopology_access.h
  @brief   Data structures, defines and functions for managing network topology and security.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    20.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/


/** X
*/
void iotopology_load_node_configuration(
    iocNode *node,
    oe_char8 *filename_dummy);

void iotopology_save_node_configuration(
    iocNode *node);
