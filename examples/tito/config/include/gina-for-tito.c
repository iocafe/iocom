/* This file is gerated by signals-to-c.py script, do not modify. */
struct gina_t gina = 
{
  {
    {&ioboard_UP, 12, GINA_UP_MBLK_SZ, &gina.up.dip_switch_3},
    {30, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* dip_switch_3 */
    {31, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* dip_switch_4 */
    {32, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* touch_sensor */
    {33, 1, OS_USHORT, 0, &ioboard_UP, OS_NULL}, /* potentiometer */
    {36, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* A */
    {37, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* B */
    {38, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* C */
    {39, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* D */
    {40, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* E */
    {41, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* F */
    {42, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL}, /* G */
    {43, 1, OS_BOOLEAN, 0, &ioboard_UP, OS_NULL} /* H */
  },

  {
    {&ioboard_DOWN, 4, GINA_DOWN_MBLK_SZ, &gina.down.seven_segment},
    {0, 8, OS_BOOLEAN, 0, &ioboard_DOWN, OS_NULL}, /* seven_segment */
    {9, 1, OS_SHORT, 0, &ioboard_DOWN, OS_NULL}, /* servo */
    {12, 1, OS_SHORT, 0, &ioboard_DOWN, OS_NULL}, /* dimmer_led */
    {15, 1, OS_BOOLEAN, 0, &ioboard_DOWN, OS_NULL} /* led_builtin */
  }
};

static const iocMblkSignalHdr *gina_mblk_list[] =
{
  &gina.up.hdr,
  &gina.down.hdr
};

const iocDeviceHdr gina_hdr = {gina_mblk_list, sizeof(gina_mblk_list)/sizeof(iocMblkSignalHdr*)};
