/* This file is gerated by signals-to-c.py script, do not modify. */
const struct gina_t gina = 
{
  {
    {"exp", &ioboard_exp, 13, GINA_EXP_MBLK_SZ, (iocSignal*)&gina.exp.dip_switch_3},
    {30, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.dip_switch_3}, /* dip_switch_3 */
    {31, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.dip_switch_4}, /* dip_switch_4 */
    {32, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.touch_sensor}, /* touch_sensor */
    {33, 1, OS_USHORT|IOC_PIN_PTR, &ioboard_exp, &pins.analog_inputs.potentiometer}, /* potentiometer */
    {0, 5, OS_FLOAT, &ioboard_exp, OS_NULL}, /* testfloat */
    {21, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.A}, /* A */
    {22, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.B}, /* B */
    {23, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.C}, /* C */
    {24, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.D}, /* D */
    {25, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.E}, /* E */
    {26, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.F}, /* F */
    {27, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.G}, /* G */
    {28, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.H} /* H */
  },

  {
    {"imp", &ioboard_imp, 4, GINA_IMP_MBLK_SZ, (iocSignal*)&gina.imp.seven_segment},
    {0, 8, OS_BOOLEAN, &ioboard_imp, OS_NULL}, /* seven_segment */
    {2, 1, OS_SHORT|IOC_PIN_PTR, &ioboard_imp, &pins.pwm.servo}, /* servo */
    {5, 1, OS_SHORT|IOC_PIN_PTR, &ioboard_imp, &pins.pwm.dimmer_led}, /* dimmer_led */
    {8, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_imp, &pins.outputs.led_builtin} /* led_builtin */
  },

  {
    {"conf_exp", &ioboard_conf_exp, 5, GINA_CONF_EXP_MBLK_SZ, (iocSignal*)&gina.conf_exp.tod_state},
    {0, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* tod_state */
    {2, 1, OS_INT, &ioboard_conf_exp, OS_NULL}, /* tod_tail */
    {6, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* frd_state */
    {8, 257, OS_UCHAR, &ioboard_conf_exp, OS_NULL}, /* frd_buf */
    {266, 1, OS_INT, &ioboard_conf_exp, OS_NULL} /* frd_head */
  },

  {
    {"conf_imp", &ioboard_conf_imp, 7, GINA_CONF_IMP_MBLK_SZ, (iocSignal*)&gina.conf_imp.tod_cmd},
    {0, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_cmd */
    {2, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_select */
    {4, 513, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* tod_buf */
    {518, 1, OS_INT, &ioboard_conf_imp, OS_NULL}, /* tod_head */
    {522, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* frd_cmd */
    {524, 1, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* frd_select */
    {526, 1, OS_INT, &ioboard_conf_imp, OS_NULL} /* frd_tail */
  }
};

static const iocMblkSignalHdr *gina_mblk_list[] =
{
  &gina.exp.hdr,
  &gina.imp.hdr,
  &gina.conf_exp.hdr,
  &gina.conf_imp.hdr
};

const iocDeviceHdr gina_hdr = {(iocMblkSignalHdr**)gina_mblk_list, sizeof(gina_mblk_list)/sizeof(iocMblkSignalHdr*)};
