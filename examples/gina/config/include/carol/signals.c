/* This file is gerated by signals-to-c.py script, do not modify. */
const struct gina_t gina = 
{
  {
    {"exp", &ioboard_exp, 14, GINA_EXP_MBLK_SZ, (iocSignal*)&gina.exp.dip_switch_3},
    {40, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.dip_switch_3}, /* dip_switch_3 */
    {41, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.dip_switch_4}, /* dip_switch_4 */
    {42, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.touch_sensor}, /* touch_sensor */
    {43, 1, OS_USHORT|IOC_PIN_PTR, &ioboard_exp, &pins.inputs.gazerbeam}, /* gazerbeam */
    {0, 5, OS_FLOAT, &ioboard_exp, OS_NULL}, /* testfloat */
    {21, 10, OS_STR, &ioboard_exp, OS_NULL}, /* teststr */
    {32, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.A}, /* A */
    {33, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.B}, /* B */
    {34, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.C}, /* C */
    {35, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.D}, /* D */
    {36, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.E}, /* E */
    {37, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.F}, /* F */
    {38, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.G}, /* G */
    {39, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_exp, &pins.outputs.H} /* H */
  },

  {
    {"imp", &ioboard_imp, 5, GINA_IMP_MBLK_SZ, (iocSignal*)&gina.imp.strtodevice},
    {0, 16, OS_STR, &ioboard_imp, OS_NULL}, /* strtodevice */
    {17, 8, OS_BOOLEAN, &ioboard_imp, OS_NULL}, /* seven_segment */
    {19, 1, OS_USHORT|IOC_PIN_PTR, &ioboard_imp, &pins.pwm.servo}, /* servo */
    {22, 1, OS_USHORT|IOC_PIN_PTR, &ioboard_imp, &pins.pwm.dimmer_led}, /* dimmer_led */
    {25, 1, OS_BOOLEAN, &ioboard_imp, OS_NULL} /* led_builtin_x */
  },

  {
    {"conf_exp", &ioboard_conf_exp, 5, GINA_CONF_EXP_MBLK_SZ, (iocSignal*)&gina.conf_exp.tod_state},
    {0, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* tod_state */
    {2, 1, OS_INT, &ioboard_conf_exp, OS_NULL}, /* tod_tail */
    {7, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* frd_state */
    {9, 257, OS_UCHAR, &ioboard_conf_exp, OS_NULL}, /* frd_buf */
    {267, 1, OS_INT, &ioboard_conf_exp, OS_NULL} /* frd_head */
  },

  {
    {"conf_imp", &ioboard_conf_imp, 7, GINA_CONF_IMP_MBLK_SZ, (iocSignal*)&gina.conf_imp.tod_cmd},
    {0, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_cmd */
    {2, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_select */
    {4, 257, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* tod_buf */
    {262, 1, OS_INT, &ioboard_conf_imp, OS_NULL}, /* tod_head */
    {267, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* frd_cmd */
    {269, 1, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* frd_select */
    {271, 1, OS_INT, &ioboard_conf_imp, OS_NULL} /* frd_tail */
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
