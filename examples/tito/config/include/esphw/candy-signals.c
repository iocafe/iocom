/* This file is gerated by signals_to_c.py script, do not modify. */
void candy_init_signal_struct(candy_t *s)
{
  os_memclear(s, sizeof(candy_t));
  s->exp.hdr.mblk_name = "exp";
  s->exp.hdr.n_signals = 6;
  s->exp.hdr.mblk_sz = CANDY_EXP_MBLK_SZ;
  s->exp.hdr.first_signal = &s->exp.ambient;

 /* ambient */
  s->exp.ambient.addr = 0;
  s->exp.ambient.n = 1;
  s->exp.ambient.flags = OS_UINT;

 /* unused_pin */
  s->exp.unused_pin.addr = 5;
  s->exp.unused_pin.n = 1;
  s->exp.unused_pin.flags = OS_UINT;

 /* prm_resolution */
  s->exp.prm_resolution.addr = 10;
  s->exp.prm_resolution.n = 10;
  s->exp.prm_resolution.flags = OS_STR;

 /* rec_state */
  s->exp.rec_state.addr = 21;
  s->exp.rec_state.n = 1;
  s->exp.rec_state.flags = OS_CHAR;

 /* rec_head */
  s->exp.rec_head.addr = 23;
  s->exp.rec_head.n = 1;
  s->exp.rec_head.flags = OS_INT;

 /* rec_buf */
  s->exp.rec_buf.addr = 28;
  s->exp.rec_buf.n = 8192;
  s->exp.rec_buf.flags = OS_UCHAR;
  s->mblk_list[0] = &s->exp.hdr;

  s->imp.hdr.mblk_name = "imp";
  s->imp.hdr.n_signals = 6;
  s->imp.hdr.mblk_sz = CANDY_IMP_MBLK_SZ;
  s->imp.hdr.first_signal = &s->imp.on;

 /* on */
  s->imp.on.addr = 0;
  s->imp.on.n = 1;
  s->imp.on.flags = OS_BOOLEAN;

 /* headlight */
  s->imp.headlight.addr = 1;
  s->imp.headlight.n = 1;
  s->imp.headlight.flags = OS_UINT;

 /* set_resolution */
  s->imp.set_resolution.addr = 6;
  s->imp.set_resolution.n = 10;
  s->imp.set_resolution.flags = OS_STR;

 /* rec_cmd */
  s->imp.rec_cmd.addr = 17;
  s->imp.rec_cmd.n = 1;
  s->imp.rec_cmd.flags = OS_CHAR;

 /* rec_select */
  s->imp.rec_select.addr = 19;
  s->imp.rec_select.n = 1;
  s->imp.rec_select.flags = OS_UCHAR;

 /* rec_tail */
  s->imp.rec_tail.addr = 21;
  s->imp.rec_tail.n = 1;
  s->imp.rec_tail.flags = OS_INT;
  s->mblk_list[1] = &s->imp.hdr;

  s->conf_exp.hdr.mblk_name = "conf_exp";
  s->conf_exp.hdr.n_signals = 5;
  s->conf_exp.hdr.mblk_sz = CANDY_CONF_EXP_MBLK_SZ;
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

 /* frd_head */
  s->conf_exp.frd_head.addr = 9;
  s->conf_exp.frd_head.n = 1;
  s->conf_exp.frd_head.flags = OS_INT;

 /* frd_buf */
  s->conf_exp.frd_buf.addr = 14;
  s->conf_exp.frd_buf.n = 257;
  s->conf_exp.frd_buf.flags = OS_UCHAR;
  s->mblk_list[2] = &s->conf_exp.hdr;

  s->conf_imp.hdr.mblk_name = "conf_imp";
  s->conf_imp.hdr.n_signals = 7;
  s->conf_imp.hdr.mblk_sz = CANDY_CONF_IMP_MBLK_SZ;
  s->conf_imp.hdr.first_signal = &s->conf_imp.tod_cmd;

 /* tod_cmd */
  s->conf_imp.tod_cmd.addr = 0;
  s->conf_imp.tod_cmd.n = 1;
  s->conf_imp.tod_cmd.flags = OS_CHAR;

 /* tod_select */
  s->conf_imp.tod_select.addr = 2;
  s->conf_imp.tod_select.n = 1;
  s->conf_imp.tod_select.flags = OS_CHAR;

 /* tod_head */
  s->conf_imp.tod_head.addr = 4;
  s->conf_imp.tod_head.n = 1;
  s->conf_imp.tod_head.flags = OS_INT;

 /* tod_buf */
  s->conf_imp.tod_buf.addr = 9;
  s->conf_imp.tod_buf.n = 257;
  s->conf_imp.tod_buf.flags = OS_UCHAR;

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

  /* camera 'camera' */
  s->camera.cmd =  &s->imp.rec_cmd;
  s->camera.select =  &s->imp.rec_select;
  s->camera.buf =  &s->exp.rec_buf;
  s->camera.head =  &s->exp.rec_head;
  s->camera.tail =  &s->imp.rec_tail;
  s->camera.state =  &s->exp.rec_state;

  s->hdr.n_mblk_hdrs = 4;
  s->hdr.mblk_hdr = s->mblk_list;
}
