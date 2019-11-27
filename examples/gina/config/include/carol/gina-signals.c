/* This file is gerated by signals-to-c.py script, do not modify. */
const struct gina_t gina = 
{
  {
    {"exp", &ioboard_export, 13, GINA_EXP_MBLK_SZ, (iocSignal*)&gina.exp.dip_switch_3},
    {30, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.inputs.dip_switch_3}, /* dip_switch_3 */
    {31, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.inputs.dip_switch_4}, /* dip_switch_4 */
    {32, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.inputs.touch_sensor}, /* touch_sensor */
    {33, 1, OS_USHORT|IOC_PIN_PTR, &ioboard_export, &pins.analog_inputs.potentiometer}, /* potentiometer */
    {0, 5, OS_FLOAT, &ioboard_export, OS_NULL}, /* testfloat */
    {21, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.A}, /* A */
    {22, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.B}, /* B */
    {23, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.C}, /* C */
    {24, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.D}, /* D */
    {25, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.E}, /* E */
    {26, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.F}, /* F */
    {27, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.G}, /* G */
    {28, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_export, &pins.outputs.H} /* H */
  },

  {
    {"imp", &ioboard_import, 4, GINA_IMP_MBLK_SZ, (iocSignal*)&gina.imp.seven_segment},
    {0, 8, OS_BOOLEAN, &ioboard_import, OS_NULL}, /* seven_segment */
    {2, 1, OS_SHORT|IOC_PIN_PTR, &ioboard_import, &pins.pwm.servo}, /* servo */
    {5, 1, OS_SHORT|IOC_PIN_PTR, &ioboard_import, &pins.pwm.dimmer_led}, /* dimmer_led */
    {8, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_import, &pins.outputs.led_builtin} /* led_builtin */
  }
};

static const iocMblkSignalHdr *gina_mblk_list[] =
{
  &gina.exp.hdr,
  &gina.imp.hdr
};

const iocDeviceHdr gina_hdr = {(iocMblkSignalHdr**)gina_mblk_list, sizeof(gina_mblk_list)/sizeof(iocMblkSignalHdr*)};
