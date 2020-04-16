/* This file is gerated by signals-to-c.py script, do not modify. */
void gina_init_signal_struct(gina_t *s)
{
  os_memclear(s, sizeof(gina_t));
  s->exp.hdr.mblk_name = "exp";
  s->exp.hdr.n_signals = 9;
  s->exp.hdr.mblk_sz = GINA_EXP_MBLK_SZ;
  s->exp.hdr.first_signal = &s->exp.frame_rate;

 /* frame_rate */
  s->exp.frame_rate.addr = 0;
  s->exp.frame_rate.n = 1;
  s->exp.frame_rate.flags = OS_FLOAT;

 /* testfloat */
  s->exp.testfloat.addr = 5;
  s->exp.testfloat.n = 5;
  s->exp.testfloat.flags = OS_FLOAT;

 /* teststr */
  s->exp.teststr.addr = 26;
  s->exp.teststr.n = 10;
  s->exp.teststr.flags = OS_STR;

 /* testbool */
  s->exp.testbool.addr = 37;
  s->exp.testbool.n = 1;
  s->exp.testbool.flags = OS_BOOLEAN;

 /* in_x */
  s->exp.in_x.addr = 38;
  s->exp.in_x.n = 1;
  s->exp.in_x.flags = OS_BOOLEAN;

 /* potentiometer */
  s->exp.potentiometer.addr = 39;
  s->exp.potentiometer.n = 1;
  s->exp.potentiometer.flags = OS_FLOAT;

 /* rec_state */
  s->exp.rec_state.addr = 44;
  s->exp.rec_state.n = 1;
  s->exp.rec_state.flags = OS_CHAR;

 /* rec_buf */
  s->exp.rec_buf.addr = 46;
  s->exp.rec_buf.n = 6000;
  s->exp.rec_buf.flags = OS_UCHAR;

 /* rec_head */
  s->exp.rec_head.addr = 6047;
  s->exp.rec_head.n = 1;
  s->exp.rec_head.flags = OS_INT;
  s->mblk_list[0] = &s->exp.hdr;

  s->imp.hdr.mblk_name = "imp";
  s->imp.hdr.n_signals = 7;
  s->imp.hdr.mblk_sz = GINA_IMP_MBLK_SZ;
  s->imp.hdr.first_signal = &s->imp.strtodevice;

 /* strtodevice */
  s->imp.strtodevice.addr = 0;
  s->imp.strtodevice.n = 16;
  s->imp.strtodevice.flags = OS_STR;

 /* seven_segment */
  s->imp.seven_segment.addr = 17;
  s->imp.seven_segment.n = 8;
  s->imp.seven_segment.flags = OS_BOOLEAN;

 /* dimmer_led */
  s->imp.dimmer_led.addr = 19;
  s->imp.dimmer_led.n = 1;
  s->imp.dimmer_led.flags = OS_USHORT;

 /* myoutput */
  s->imp.myoutput.addr = 22;
  s->imp.myoutput.n = 1;
  s->imp.myoutput.flags = OS_BOOLEAN;

 /* rec_cmd */
  s->imp.rec_cmd.addr = 23;
  s->imp.rec_cmd.n = 1;
  s->imp.rec_cmd.flags = OS_CHAR;

 /* rec_select */
  s->imp.rec_select.addr = 25;
  s->imp.rec_select.n = 1;
  s->imp.rec_select.flags = OS_UCHAR;

 /* rec_tail */
  s->imp.rec_tail.addr = 27;
  s->imp.rec_tail.n = 1;
  s->imp.rec_tail.flags = OS_INT;
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

  /* linecam 'ccd' */
  s->ccd.cmd =  &s->imp.rec_cmd;
  s->ccd.select =  &s->imp.rec_select;
  s->ccd.buf =  &s->exp.rec_buf;
  s->ccd.head =  &s->exp.rec_head;
  s->ccd.tail =  &s->imp.rec_tail;
  s->ccd.state =  &s->exp.rec_state;

  s->hdr.n_mblk_hdrs = 4;
  s->hdr.mblk_hdr = s->mblk_list;
}
