/**

  @file    ioc_bserver.h
  @brief   Structures and functions to implement basic server.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    12.1.2020

  The basic server helpers functions and structures here wrap together bunch of IOCOM structures
  and API calls which are needed by typical basic server, much like ioc_ioboard does for IO boards.
  This layer is optional and written only for convinience.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

struct iocBServerNetwork;

typedef struct iocBServerParams
{
    const os_char *device_name;
    os_int device_nr;
    const os_char *network_name;

    iocMblkSignalHdr *signals_exp_hdr;
    iocMblkSignalHdr *signals_imp_hdr;
    iocMblkSignalHdr *signals_conf_exp_hdr;
    iocMblkSignalHdr *signals_conf_imp_hdr;
    const os_char *signal_config;
    os_memsz signal_config_sz;
    const os_char *network_defaults;
    os_memsz network_defaults_sz;
}
iocBServerParams;

typedef struct iocBServerMain
{
    /* Pointer to IOCOM root structure.
     */
    iocRoot *root;

    /* Identification of this IO network node.
     */
    os_char device_name[IOC_NAME_SZ];
    os_int device_nr;
    os_char network_name[IOC_NETWORK_NAME_SZ];

    /* Memory block handles for the server.
     */
    iocHandle exp, imp, conf_exp, conf_imp, info;

    /* Control stream to configure the IO node.
     */
    iocStreamerParams ctrl_stream_params;
    iocControlStreamState ctrl_stream;

    /* Data for published networks.
     */
    struct iocBServerNetwork *networks;
    os_int nro_networks;
}
iocBServerMain;


void ioc_initialize_bserver(
    iocBServerMain *m,
    iocRoot *root,
    iocBServerParams *prm);

void ioc_release_bserver(
    iocBServerMain *m);

osalStatus ioc_publish_bserver_networks(
    iocBServerMain *m,
    const os_char *publish);

void ioc_run_bserver_main(
    iocBServerMain *m);

#define IOC_SETUP_BSERVER_CTRL_STREAM_MACRO(bmain, sig) \
    bmain.ctrl_stream_params.is_device = OS_TRUE; \
    bmain.ctrl_stream_params.frd.cmd = &sig.conf_imp.frd_cmd; \
    bmain.ctrl_stream_params.frd.select = &sig.conf_imp.frd_select; \
    bmain.ctrl_stream_params.frd.buf = &sig.conf_exp.frd_buf; \
    bmain.ctrl_stream_params.frd.head = &sig.conf_exp.frd_head; \
    bmain.ctrl_stream_params.frd.tail = &sig.conf_imp.frd_tail; \
    bmain.ctrl_stream_params.frd.state = &sig.conf_exp.frd_state; \
    bmain.ctrl_stream_params.frd.to_device = OS_FALSE; \
    bmain.ctrl_stream_params.tod.cmd = &sig.conf_imp.tod_cmd; \
    bmain.ctrl_stream_params.tod.select = &sig.conf_imp.tod_select; \
    bmain.ctrl_stream_params.tod.buf = &sig.conf_imp.tod_buf; \
    bmain.ctrl_stream_params.tod.head = &sig.conf_imp.tod_head; \
    bmain.ctrl_stream_params.tod.tail = &sig.conf_exp.tod_tail; \
    bmain.ctrl_stream_params.tod.state = &sig.conf_exp.tod_state; \
    bmain.ctrl_stream_params.tod.to_device = OS_TRUE; \
    ioc_init_control_stream(&bmain.ctrl_stream, &bmain.ctrl_stream_params); \

