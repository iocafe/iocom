/**

  @file    application.h
  @brief   Buster application's main class.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    2.8.2020

  Copyright 2020 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/

/* Global signals. This allows mapping IO pins directly to signals from JSON, but we can have only
   one application instance.
 */
extern struct buster_t buster;

/**
****************************************************************************************************

  Buster main object.

****************************************************************************************************
*/
class Application : public AbstractApplication
{
public:
    /* Functions to start, stop and thread function to run the application.
     */
    void start(os_int argc, const os_char *argv[]);
    void stop();
    osalStatus run(os_timer *ti);

    virtual void communication_callback_1(
        struct iocHandle *handle,
        os_int start_addr,
        os_int end_addr,
        os_ushort flags);

    /* Structure holding signals for the Buster.
     */
    buster_t *m_signals;

    /* Device information (nc = network configuration, rm resource monitor). */
    dinfoNodeConfState m_dinfo_nc;
    dinfoResMonState m_dinfo_rm;

    /* Basic server (ioserver extension) structure.
     */
    iocBServer m_bmain;

    Minion m_minion1;
    minion_t *m_minion1_def;

    BlinkLedSequence m_test_seq1;

/* Camera state and camera output */
#if PINS_CAMERA
    // pinsCamera pins_camera;
    // iocBrickBuffer video_output;
    /* Camera control parameter has changed, camera on/off */
    os_boolean m_camera_on_or_off;
    // os_boolean camera_is_on;
#endif

};
