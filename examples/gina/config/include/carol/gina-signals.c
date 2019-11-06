/* This file is gerated by signals-to-c.py script, do not modify. */
void gina_init_signal_struct(gina_t *s, gina_init_prm_t *prm)
{
  os_memclear(s, sizeof(gina_t));
  s->up.hdr.handle = up;
  s->up.hdr.n_signals = 12;
  s->up.hdr.mblk_sz = GINA_UP_MBLK_SZ;
  s->up.hdr.first_signal = &s->up.dip_switch_3;

 /* dip_switch_3 */
  s->up.dip_switch_3.addr = 30;
  s->up.dip_switch_3.n = 1;
  s->up.dip_switch_3.type = OS_BOOLEAN;
  s->up.dip_switch_3.handle = up;

 /* dip_switch_4 */
  s->up.dip_switch_4.addr = 31;
  s->up.dip_switch_4.n = 1;
  s->up.dip_switch_4.type = OS_BOOLEAN;
  s->up.dip_switch_4.handle = up;

 /* touch_sensor */
  s->up.touch_sensor.addr = 32;
  s->up.touch_sensor.n = 1;
  s->up.touch_sensor.type = OS_BOOLEAN;
  s->up.touch_sensor.handle = up;

 /* potentiometer */
  s->up.potentiometer.addr = 33;
  s->up.potentiometer.n = 1;
  s->up.potentiometer.type = OS_USHORT;
  s->up.potentiometer.handle = up;

 /* A */
  s->up.A.addr = 36;
  s->up.A.n = 1;
  s->up.A.type = OS_BOOLEAN;
  s->up.A.handle = up;

 /* B */
  s->up.B.addr = 37;
  s->up.B.n = 1;
  s->up.B.type = OS_BOOLEAN;
  s->up.B.handle = up;

 /* C */
  s->up.C.addr = 38;
  s->up.C.n = 1;
  s->up.C.type = OS_BOOLEAN;
  s->up.C.handle = up;

 /* D */
  s->up.D.addr = 39;
  s->up.D.n = 1;
  s->up.D.type = OS_BOOLEAN;
  s->up.D.handle = up;

 /* E */
  s->up.E.addr = 40;
  s->up.E.n = 1;
  s->up.E.type = OS_BOOLEAN;
  s->up.E.handle = up;

 /* F */
  s->up.F.addr = 41;
  s->up.F.n = 1;
  s->up.F.type = OS_BOOLEAN;
  s->up.F.handle = up;

 /* G */
  s->up.G.addr = 42;
  s->up.G.n = 1;
  s->up.G.type = OS_BOOLEAN;
  s->up.G.handle = up;

 /* H */
  s->up.H.addr = 43;
  s->up.H.n = 1;
  s->up.H.type = OS_BOOLEAN;
  s->up.H.handle = up;
  s->mblk_list[0] = &s->up.hdr;

  s->down.hdr.handle = down;
  s->down.hdr.n_signals = 4;
  s->down.hdr.mblk_sz = GINA_DOWN_MBLK_SZ;
  s->down.hdr.first_signal = &s->down.seven_segment;

 /* seven_segment */
  s->down.seven_segment.addr = 0;
  s->down.seven_segment.n = 8;
  s->down.seven_segment.type = OS_BOOLEAN;
  s->down.seven_segment.handle = down;

 /* servo */
  s->down.servo.addr = 9;
  s->down.servo.n = 1;
  s->down.servo.type = OS_SHORT;
  s->down.servo.handle = down;

 /* dimmer_led */
  s->down.dimmer_led.addr = 12;
  s->down.dimmer_led.n = 1;
  s->down.dimmer_led.type = OS_SHORT;
  s->down.dimmer_led.handle = down;

 /* led_builtin */
  s->down.led_builtin.addr = 15;
  s->down.led_builtin.n = 1;
  s->down.led_builtin.type = OS_BOOLEAN;
  s->down.led_builtin.handle = down;
  s->mblk_list[1] = &s->down.hdr;

  s->hdr.n_mblk_hdrs = 2;
  s->hdr.mblk_hdr = s->mblk_list;
}
