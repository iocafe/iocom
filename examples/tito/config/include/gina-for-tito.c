/* This file is gerated by signals-to-c.py script, do not modify. */
void gina_init_signal_struct(gina_t *s)
{
  os_memclear(s, sizeof(gina_t));
  s->exp.hdr.mblk_name = "exp";
  s->exp.hdr.n_signals = 12;
  s->exp.hdr.mblk_sz = GINA_EXP_MBLK_SZ;
  s->exp.hdr.first_signal = &s->exp.dip_switch_3;

 /* dip_switch_3 */
  s->exp.dip_switch_3.addr = 30;
  s->exp.dip_switch_3.n = 1;
  s->exp.dip_switch_3.flags = OS_BOOLEAN;

 /* dip_switch_4 */
  s->exp.dip_switch_4.addr = 31;
  s->exp.dip_switch_4.n = 1;
  s->exp.dip_switch_4.flags = OS_BOOLEAN;

 /* touch_sensor */
  s->exp.touch_sensor.addr = 32;
  s->exp.touch_sensor.n = 1;
  s->exp.touch_sensor.flags = OS_BOOLEAN;

 /* potentiometer */
  s->exp.potentiometer.addr = 33;
  s->exp.potentiometer.n = 1;
  s->exp.potentiometer.flags = OS_USHORT;

 /* A */
  s->exp.A.addr = 36;
  s->exp.A.n = 1;
  s->exp.A.flags = OS_BOOLEAN;

 /* B */
  s->exp.B.addr = 37;
  s->exp.B.n = 1;
  s->exp.B.flags = OS_BOOLEAN;

 /* C */
  s->exp.C.addr = 38;
  s->exp.C.n = 1;
  s->exp.C.flags = OS_BOOLEAN;

 /* D */
  s->exp.D.addr = 39;
  s->exp.D.n = 1;
  s->exp.D.flags = OS_BOOLEAN;

 /* E */
  s->exp.E.addr = 40;
  s->exp.E.n = 1;
  s->exp.E.flags = OS_BOOLEAN;

 /* F */
  s->exp.F.addr = 41;
  s->exp.F.n = 1;
  s->exp.F.flags = OS_BOOLEAN;

 /* G */
  s->exp.G.addr = 42;
  s->exp.G.n = 1;
  s->exp.G.flags = OS_BOOLEAN;

 /* H */
  s->exp.H.addr = 43;
  s->exp.H.n = 1;
  s->exp.H.flags = OS_BOOLEAN;
  s->mblk_list[0] = &s->exp.hdr;

  s->imp.hdr.mblk_name = "imp";
  s->imp.hdr.n_signals = 4;
  s->imp.hdr.mblk_sz = GINA_IMP_MBLK_SZ;
  s->imp.hdr.first_signal = &s->imp.seven_segment;

 /* seven_segment */
  s->imp.seven_segment.addr = 0;
  s->imp.seven_segment.n = 8;
  s->imp.seven_segment.flags = OS_BOOLEAN;

 /* servo */
  s->imp.servo.addr = 2;
  s->imp.servo.n = 1;
  s->imp.servo.flags = OS_SHORT;

 /* dimmer_led */
  s->imp.dimmer_led.addr = 5;
  s->imp.dimmer_led.n = 1;
  s->imp.dimmer_led.flags = OS_SHORT;

 /* led_builtin */
  s->imp.led_builtin.addr = 8;
  s->imp.led_builtin.n = 1;
  s->imp.led_builtin.flags = OS_BOOLEAN;
  s->mblk_list[1] = &s->imp.hdr;

  s->hdr.n_mblk_hdrs = 2;
  s->hdr.mblk_hdr = s->mblk_list;
}
