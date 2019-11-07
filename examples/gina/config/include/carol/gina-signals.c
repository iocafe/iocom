/* This file is gerated by signals-to-c.py script, do not modify. */
const struct gina_t gina = 
{
  {
    {&ioboard_UP, 12, GINA_UP_MBLK_SZ, &gina.up.dip_switch_3},
    {30, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.inputs.dip_switch_3}, /* dip_switch_3 */
    {31, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.inputs.dip_switch_4}, /* dip_switch_4 */
    {32, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.inputs.touch_sensor}, /* touch_sensor */
    {33, 1, OS_USHORT|IOC_PIN_PTR, &ioboard_UP, &pins.analog_inputs.potentiometer}, /* potentiometer */
    {36, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.A}, /* A */
    {37, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.B}, /* B */
    {38, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.C}, /* C */
    {39, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.D}, /* D */
    {40, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.E}, /* E */
    {41, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.F}, /* F */
    {42, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.G}, /* G */
    {43, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_UP, &pins.outputs.H} /* H */
  },

  {
    {&ioboard_DOWN, 4, GINA_DOWN_MBLK_SZ, &gina.down.seven_segment},
    {0, 8, OS_BOOLEAN, &ioboard_DOWN, OS_NULL}, /* seven_segment */
    {2, 1, OS_SHORT|IOC_PIN_PTR, &ioboard_DOWN, &pins.pwm.servo}, /* servo */
    {5, 1, OS_SHORT|IOC_PIN_PTR, &ioboard_DOWN, &pins.pwm.dimmer_led}, /* dimmer_led */
    {8, 1, OS_BOOLEAN|IOC_PIN_PTR, &ioboard_DOWN, &pins.outputs.led_builtin} /* led_builtin */
  }
};

static const iocMblkSignalHdr *gina_mblk_list[] =
{
  &gina.up.hdr,
  &gina.down.hdr
};

const iocDeviceHdr gina_hdr = {gina_mblk_list, sizeof(gina_mblk_list)/sizeof(iocMblkSignalHdr*)};
