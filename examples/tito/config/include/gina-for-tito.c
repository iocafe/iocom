/* This file is gerated by signals-to-c.py script, do not modify. */
void gina_init_signal_struct(gina_t *s)
{
  os_memclear(s, sizeof(gina_t));
  s->exp.hdr.mblk_name = "exp";
  s->exp.hdr.n_signals = 13;
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

 /* testfloat */
  s->exp.testfloat.addr = 0;
  s->exp.testfloat.n = 5;
  s->exp.testfloat.flags = OS_FLOAT;

 /* A */
  s->exp.A.addr = 21;
  s->exp.A.n = 1;
  s->exp.A.flags = OS_BOOLEAN;

 /* B */
  s->exp.B.addr = 22;
  s->exp.B.n = 1;
  s->exp.B.flags = OS_BOOLEAN;

 /* C */
  s->exp.C.addr = 23;
  s->exp.C.n = 1;
  s->exp.C.flags = OS_BOOLEAN;

 /* D */
  s->exp.D.addr = 24;
  s->exp.D.n = 1;
  s->exp.D.flags = OS_BOOLEAN;

 /* E */
  s->exp.E.addr = 25;
  s->exp.E.n = 1;
  s->exp.E.flags = OS_BOOLEAN;

 /* F */
  s->exp.F.addr = 26;
  s->exp.F.n = 1;
  s->exp.F.flags = OS_BOOLEAN;

 /* G */
  s->exp.G.addr = 27;
  s->exp.G.n = 1;
  s->exp.G.flags = OS_BOOLEAN;

 /* H */
  s->exp.H.addr = 28;
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

  s->conf_exp.hdr.mblk_name = "conf_exp";
  s->conf_exp.hdr.n_signals = 5;
  s->conf_exp.hdr.mblk_sz = GINA_CONF_EXP_MBLK_SZ;
  s->conf_exp.hdr.first_signal = &s->conf_exp.tod_state;

 /* tod_state */
  s->conf_exp.tod_state.addr = 0;
  s->conf_exp.tod_state.n = 1;
  s->conf_exp.tod_state.flags = OS_CHAR;

 /* tod_tail */
  s->conf_exp.tod_tail.addr = 2;
  s->conf_exp.tod_tail.n = 1;
  s->conf_exp.tod_tail.flags = OS_INT;

 /* frd_state */
  s->conf_exp.frd_state.addr = 7;
  s->conf_exp.frd_state.n = 1;
  s->conf_exp.frd_state.flags = OS_CHAR;

 /* frd_buf */
  s->conf_exp.frd_buf.addr = 9;
  s->conf_exp.frd_buf.n = 257;
  s->conf_exp.frd_buf.flags = OS_UCHAR;

 /* frd_head */
  s->conf_exp.frd_head.addr = 267;
  s->conf_exp.frd_head.n = 1;
  s->conf_exp.frd_head.flags = OS_INT;
  s->mblk_list[2] = &s->conf_exp.hdr;

  s->conf_imp.hdr.mblk_name = "conf_imp";
  s->conf_imp.hdr.n_signals = 7;
  s->conf_imp.hdr.mblk_sz = GINA_CONF_IMP_MBLK_SZ;
  s->conf_imp.hdr.first_signal = &s->conf_imp.tod_cmd;

 /* tod_cmd */
  s->conf_imp.tod_cmd.addr = 0;
  s->conf_imp.tod_cmd.n = 1;
  s->conf_imp.tod_cmd.flags = OS_CHAR;

 /* tod_select */
  s->conf_imp.tod_select.addr = 2;
  s->conf_imp.tod_select.n = 1;
  s->conf_imp.tod_select.flags = OS_CHAR;

 /* tod_buf */
  s->conf_imp.tod_buf.addr = 4;
  s->conf_imp.tod_buf.n = 257;
  s->conf_imp.tod_buf.flags = OS_UCHAR;

 /* tod_head */
  s->conf_imp.tod_head.addr = 262;
  s->conf_imp.tod_head.n = 1;
  s->conf_imp.tod_head.flags = OS_INT;

 /* frd_cmd */
  s->conf_imp.frd_cmd.addr = 267;
  s->conf_imp.frd_cmd.n = 1;
  s->conf_imp.frd_cmd.flags = OS_CHAR;

 /* frd_select */
  s->conf_imp.frd_select.addr = 269;
  s->conf_imp.frd_select.n = 1;
  s->conf_imp.frd_select.flags = OS_UCHAR;

 /* frd_tail */
  s->conf_imp.frd_tail.addr = 271;
  s->conf_imp.frd_tail.n = 1;
  s->conf_imp.frd_tail.flags = OS_INT;
  s->mblk_list[3] = &s->conf_imp.hdr;

  s->hdr.n_mblk_hdrs = 4;
  s->hdr.mblk_hdr = s->mblk_list;
}
