/* This file is gerated by signals_to_c.py script, do not modify. */
OS_FLASH_MEM struct gina_t gina = 
{
  {
    {"exp", &ioboard_exp, 9, GINA_EXP_MBLK_SZ, (iocSignal*)&gina.exp.frame_rate},
    {0, 1, OS_FLOAT, &ioboard_exp, OS_NULL}, /* frame_rate */
    {5, 5, OS_FLOAT, &ioboard_exp, OS_NULL}, /* testfloat */
    {26, 10, OS_STR, &ioboard_exp, OS_NULL}, /* teststr */
    {37, 1, OS_BOOLEAN, &ioboard_exp, OS_NULL}, /* testbool */
    {38, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.in_x}, /* in_x */
    {39, 1, OS_FLOAT|IOC_PIN_PTR, &ioboard_exp, &pins.analog_inputs.potentiometer}, /* potentiometer */
    {44, 1, OS_CHAR, &ioboard_exp, OS_NULL}, /* rec_state */
    {46, 1, OS_INT, &ioboard_exp, OS_NULL}, /* rec_head */
    {51, 5000, OS_UCHAR, &ioboard_exp, OS_NULL} /* rec_buf */
  },

  {
    {"imp", &ioboard_imp, 7, GINA_IMP_MBLK_SZ, (iocSignal*)&gina.imp.strtodevice},
    {0, 16, OS_STR, &ioboard_imp, OS_NULL}, /* strtodevice */
    {17, 8, OS_BOOLEAN, &ioboard_imp, OS_NULL}, /* seven_segment */
    {19, 1, OS_USHORT|IOC_PIN_PTR, &ioboard_imp, &pins.pwm.dimmer_led}, /* dimmer_led */
    {22, 1, OS_BOOLEAN, &ioboard_imp, OS_NULL}, /* myoutput */
    {23, 1, OS_CHAR, &ioboard_imp, OS_NULL}, /* rec_cmd */
    {25, 1, OS_UCHAR, &ioboard_imp, OS_NULL}, /* rec_select */
    {27, 1, OS_INT, &ioboard_imp, OS_NULL} /* rec_tail */
  },

  {
    {"conf_exp", &ioboard_conf_exp, 5, GINA_CONF_EXP_MBLK_SZ, (iocSignal*)&gina.conf_exp.tod_state},
    {0, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* tod_state */
    {2, 1, OS_INT, &ioboard_conf_exp, OS_NULL}, /* tod_tail */
    {7, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* frd_state */
    {9, 1, OS_INT, &ioboard_conf_exp, OS_NULL}, /* frd_head */
    {14, 257, OS_UCHAR, &ioboard_conf_exp, OS_NULL} /* frd_buf */
  },

  {
    {"conf_imp", &ioboard_conf_imp, 7, GINA_CONF_IMP_MBLK_SZ, (iocSignal*)&gina.conf_imp.tod_cmd},
    {0, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_cmd */
    {2, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_select */
    {4, 1, OS_INT, &ioboard_conf_imp, OS_NULL}, /* tod_head */
    {9, 257, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* tod_buf */
    {267, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* frd_cmd */
    {269, 1, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* frd_select */
    {271, 1, OS_INT, &ioboard_conf_imp, OS_NULL} /* frd_tail */
  },

  /* Signals for camera 'ccd' */
  {&gina.imp.rec_cmd,
   &gina.imp.rec_select,
   &gina.exp.rec_buf,
   &gina.exp.rec_head,
   &gina.imp.rec_tail,
   &gina.exp.rec_state,
   OS_FALSE},

  /* Signals for camera 'camera' */
  {&gina.imp.rec_cmd,
   &gina.imp.rec_select,
   &gina.exp.rec_buf,
   &gina.exp.rec_head,
   &gina.imp.rec_tail,
   &gina.exp.rec_state,
   OS_FALSE}
};

static OS_FLASH_MEM iocMblkSignalHdr * OS_FLASH_MEM gina_mblk_list[] =
{
  &gina.exp.hdr,
  &gina.imp.hdr,
  &gina.conf_exp.hdr,
  &gina.conf_imp.hdr
};

OS_FLASH_MEM iocDeviceHdr gina_hdr = {(iocMblkSignalHdr**)gina_mblk_list, sizeof(gina_mblk_list)/sizeof(iocMblkSignalHdr*)};
