/**

  @file    controller_root.h
  @brief   Root class for Tito application.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    30.4.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/



/**
****************************************************************************************************

  Tito main object.

****************************************************************************************************
*/
class ControllerRoot
{
public:
    /* Constructor.
     */
    ControllerRoot();

    /* Virtual destructor.
     */
    virtual ~ControllerRoot();

    static const os_int MAX_APPS = 20;
    os_int m_nro_apps;
    class AppInstance *m_app[MAX_APPS];

    osalStatus loop();
};
