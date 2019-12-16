/**

  @file    tito_test_application.h
  @brief   Controller application running for one IO device network.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    6.11.2019

  Copyright 2012 Pekka Lehtikoski. This file is part of the eobjects project and shall only be used, 
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept 
  it fully.

****************************************************************************************************
*/

/**
****************************************************************************************************

  Application instance running one IO network.

****************************************************************************************************
*/
class TitoTestApplication : public TitoApplication
{
public:
    /* Constructor.
	 */
    TitoTestApplication();

	/* Virtual destructor.
 	 */
    virtual ~TitoTestApplication();

    virtual void start(const os_char *network_name, os_uint device_nr);
    virtual void stop();
    virtual void run();

    TitoGinaIoDevice m_gina1;
    TitoGinaIoDevice m_gina2;
    gina_t *m_gina1_def;
    gina_t *m_gina2_def;

    TitoTestSequence1 m_test_seq1;
};
