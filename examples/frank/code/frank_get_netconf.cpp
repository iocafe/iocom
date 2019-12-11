/**

  @file    frank_get_netconf.cpp
  @brief   Get IO device's network configuration.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    9.12.2019

  Copyright 2012 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "frank.h"

static void frank_get_netconf_thread_func(void *prm, osalEvent done);


/**
****************************************************************************************************

  @brief Application constructor.

  @return  None.

****************************************************************************************************
*/
FrankGetNetConf::FrankGetNetConf()
{
    m_event = osal_event_create();
}


/**
****************************************************************************************************

  @brief Application destructor.

  Join worker thread to this thread and clean up.

  @return  None.

****************************************************************************************************
*/
FrankGetNetConf::~FrankGetNetConf()
{
    stop();
    osal_event_delete(m_event);
}


void FrankGetNetConf::run()
{
    iocStream *stream;
    os_char *buf;
    os_memsz buf_sz;
    osalStatus s;

    stream = ioc_open_stream(&frank_root, OS_PBNR_IO_DEVICE_CONFIG,
        "frd_buf", "tod_buf", "conf_exp", "conf_imp",
        m_device_name, m_device_nr, m_network_name, 0);

    ioc_start_stream_read(stream);

    while ((s = ioc_run_stream(stream, IOC_CALL_SYNC)) == OSAL_SUCCESS)
    {
        os_timeslice();
        if (m_stop_thread) break;
    }

    if (s == OSAL_STATUS_COMPLETED)
    {
        buf = ioc_get_stream_data(stream, &buf_sz);
        // osal_trace_int("ioc_get_stream_data returned ", buf_sz);
        osal_trace_str("ioc_get_stream_data returned ", buf);
    }

    ioc_release_stream(stream);
}


void FrankGetNetConf::run_write()
{
    iocStream *stream;
    osalStatus s;

    stream = ioc_open_stream(&frank_root, OS_PBNR_IO_DEVICE_CONFIG,
        "frd_buf", "tod_buf", "conf_exp", "conf_imp",
        m_device_name, m_device_nr, m_network_name, 0);

    ioc_start_stream_write(stream, "NAKSU DATAA", 10);

    while ((s = ioc_run_stream(stream, IOC_CALL_SYNC)) == OSAL_SUCCESS)
    {
        os_timeslice();
        if (m_stop_thread) break;
    }

    if (s == OSAL_STATUS_COMPLETED)
    {
        osal_trace("data written");
    }

    ioc_release_stream(stream);
}


void FrankGetNetConf::start(const os_char *device_name, os_short device_nr, const os_char *network_name)
{
    os_strncpy(m_device_name, device_name, IOC_NAME_SZ);
    m_device_nr = device_nr;
    os_strncpy(m_network_name, network_name, IOC_NETWORK_NAME_SZ);

    m_stop_thread = OS_FALSE;
    m_thread = osal_thread_create(frank_get_netconf_thread_func, this,
        OS_NULL, OSAL_THREAD_ATTACHED);
    m_started = OS_TRUE;
}

void FrankGetNetConf::stop()
{
    if (!m_started) return;

    m_stop_thread = OS_TRUE;
    osal_event_set(m_event);
    osal_thread_join(m_thread);
    m_started = OS_FALSE;
}

static void frank_get_netconf_thread_func(void *prm, osalEvent done)
{
    FrankGetNetConf *app = (FrankGetNetConf*)prm;
    osal_event_set(done);
    app->run();
    // app->run_write();
}
