/* This file is gerated by signals-to-c.py script, do not modify. */
void gina_init_signal_struct(gina_t *s, gina_init_prm_t *prm)
{
  os_memclear(s, sizeof(gina_t));
  s->up.hdr.handle = prm->up;
  s->up.hdr.n_signals = 12;
  s->up.hdr.mblk_sz = GINA_UP_MBLK_SZ;
  s->up.hdr.first_signal = &s->up.dip_switch_3;

 /* dip_switch_3 */
  s->up.dip_switch_3.addr = 30;
  s->up.dip_switch_3.n = 1;
  s->up.dip_switch_3.flags = OS_BOOLEAN;
  s->up.dip_switch_3.handle = prm->up;

 /* dip_switch_4 */
  s->up.dip_switch_4.addr = 31;
  s->up.dip_switch_4.n = 1;
  s->up.dip_switch_4.flags = OS_BOOLEAN;
  s->up.dip_switch_4.handle = prm->up;

 /* touch_sensor */
  s->up.touch_sensor.addr = 32;
  s->up.touch_sensor.n = 1;
  s->up.touch_sensor.flags = OS_BOOLEAN;
  s->up.touch_sensor.handle = prm->up;

 /* potentiometer */
  s->up.potentiometer.addr = 33;
  s->up.potentiometer.n = 1;
  s->up.potentiometer.flags = OS_USHORT;
  s->up.potentiometer.handle = prm->up;

 /* A */
  s->up.A.addr = 36;
  s->up.A.n = 1;
  s->up.A.flags = OS_BOOLEAN;
  s->up.A.handle = prm->up;

 /* B */
  s->up.B.addr = 37;
  s->up.B.n = 1;
  s->up.B.flags = OS_BOOLEAN;
  s->up.B.handle = prm->up;

 /* C */
  s->up.C.addr = 38;
  s->up.C.n = 1;
  s->up.C.flags = OS_BOOLEAN;
  s->up.C.handle = prm->up;

 /* D */
  s->up.D.addr = 39;
  s->up.D.n = 1;
  s->up.D.flags = OS_BOOLEAN;
  s->up.D.handle = prm->up;

 /* E */
  s->up.E.addr = 40;
  s->up.E.n = 1;
  s->up.E.flags = OS_BOOLEAN;
  s->up.E.handle = prm->up;

 /* F */
  s->up.F.addr = 41;
  s->up.F.n = 1;
  s->up.F.flags = OS_BOOLEAN;
  s->up.F.handle = prm->up;

 /* G */
  s->up.G.addr = 42;
  s->up.G.n = 1;
  s->up.G.flags = OS_BOOLEAN;
  s->up.G.handle = prm->up;

 /* H */
  s->up.H.addr = 43;
  s->up.H.n = 1;
  s->up.H.flags = OS_BOOLEAN;
  s->up.H.handle = prm->up;
  s->mblk_list[0] = &s->up.hdr;

  s->down.hdr.handle = prm->down;
  s->down.hdr.n_signals = 4;
  s->down.hdr.mblk_sz = GINA_DOWN_MBLK_SZ;
  s->down.hdr.first_signal = &s->down.seven_segment;

 /* seven_segment */
  s->down.seven_segment.addr = 0;
  s->down.seven_segment.n = 8;
  s->down.seven_segment.flags = OS_BOOLEAN;
  s->down.seven_segment.handle = prm->down;

 /* servo */
  s->down.servo.addr = 9;
  s->down.servo.n = 1;
  s->down.servo.flags = OS_SHORT;
  s->down.servo.handle = prm->down;

 /* dimmer_led */
  s->down.dimmer_led.addr = 12;
  s->down.dimmer_led.n = 1;
  s->down.dimmer_led.flags = OS_SHORT;
  s->down.dimmer_led.handle = prm->down;

 /* led_builtin */
  s->down.led_builtin.addr = 15;
  s->down.led_builtin.n = 1;
  s->down.led_builtin.flags = OS_BOOLEAN;
  s->down.led_builtin.handle = prm->down;
  s->mblk_list[1] = &s->down.hdr;

  s->hdr.n_mblk_hdrs = 2;
  s->hdr.mblk_hdr = s->mblk_list;
}
