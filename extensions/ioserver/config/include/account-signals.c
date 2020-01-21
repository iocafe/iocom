/* This file is gerated by signals-to-c.py script, do not modify. */
void account_signals_init_signal_struct(account_signals_t *s)
{
  os_memclear(s, sizeof(account_signals_t));
  s->exp.hdr.mblk_name = "exp";
  s->exp.hdr.n_signals = 35;
  s->exp.hdr.mblk_sz = ACCOUNT_SIGNALS_EXP_MBLK_SZ;
  s->exp.hdr.first_signal = &s->exp.new1_text;

 /* new1_text */
  s->exp.new1_text.addr = 0;
  s->exp.new1_text.n = 16;
  s->exp.new1_text.flags = OS_STR;

 /* new1_name */
  s->exp.new1_name.addr = 17;
  s->exp.new1_name.n = 28;
  s->exp.new1_name.flags = OS_STR;

 /* new1_password */
  s->exp.new1_password.addr = 46;
  s->exp.new1_password.n = 46;
  s->exp.new1_password.flags = OS_STR;

 /* new1_privileges */
  s->exp.new1_privileges.addr = 93;
  s->exp.new1_privileges.n = 16;
  s->exp.new1_privileges.flags = OS_STR;

 /* new1_ip */
  s->exp.new1_ip.addr = 110;
  s->exp.new1_ip.n = 64;
  s->exp.new1_ip.flags = OS_STR;

 /* new1_count */
  s->exp.new1_count.addr = 175;
  s->exp.new1_count.n = 1;
  s->exp.new1_count.flags = OS_INT;

 /* new1_timer */
  s->exp.new1_timer.addr = 180;
  s->exp.new1_timer.n = 1;
  s->exp.new1_timer.flags = OS_LONG;

 /* new2_text */
  s->exp.new2_text.addr = 189;
  s->exp.new2_text.n = 16;
  s->exp.new2_text.flags = OS_STR;

 /* new2_name */
  s->exp.new2_name.addr = 206;
  s->exp.new2_name.n = 28;
  s->exp.new2_name.flags = OS_STR;

 /* new2_password */
  s->exp.new2_password.addr = 235;
  s->exp.new2_password.n = 46;
  s->exp.new2_password.flags = OS_STR;

 /* new2_privileges */
  s->exp.new2_privileges.addr = 282;
  s->exp.new2_privileges.n = 16;
  s->exp.new2_privileges.flags = OS_STR;

 /* new2_ip */
  s->exp.new2_ip.addr = 299;
  s->exp.new2_ip.n = 64;
  s->exp.new2_ip.flags = OS_STR;

 /* new2_count */
  s->exp.new2_count.addr = 364;
  s->exp.new2_count.n = 1;
  s->exp.new2_count.flags = OS_INT;

 /* new2_timer */
  s->exp.new2_timer.addr = 369;
  s->exp.new2_timer.n = 1;
  s->exp.new2_timer.flags = OS_LONG;

 /* new3_text */
  s->exp.new3_text.addr = 378;
  s->exp.new3_text.n = 16;
  s->exp.new3_text.flags = OS_STR;

 /* new3_name */
  s->exp.new3_name.addr = 395;
  s->exp.new3_name.n = 28;
  s->exp.new3_name.flags = OS_STR;

 /* new3_password */
  s->exp.new3_password.addr = 424;
  s->exp.new3_password.n = 46;
  s->exp.new3_password.flags = OS_STR;

 /* new3_privileges */
  s->exp.new3_privileges.addr = 471;
  s->exp.new3_privileges.n = 16;
  s->exp.new3_privileges.flags = OS_STR;

 /* new3_ip */
  s->exp.new3_ip.addr = 488;
  s->exp.new3_ip.n = 64;
  s->exp.new3_ip.flags = OS_STR;

 /* new3_count */
  s->exp.new3_count.addr = 553;
  s->exp.new3_count.n = 1;
  s->exp.new3_count.flags = OS_INT;

 /* new3_timer */
  s->exp.new3_timer.addr = 558;
  s->exp.new3_timer.n = 1;
  s->exp.new3_timer.flags = OS_LONG;

 /* alarm1_text */
  s->exp.alarm1_text.addr = 567;
  s->exp.alarm1_text.n = 16;
  s->exp.alarm1_text.flags = OS_STR;

 /* alarm1_name */
  s->exp.alarm1_name.addr = 584;
  s->exp.alarm1_name.n = 28;
  s->exp.alarm1_name.flags = OS_STR;

 /* alarm1_password */
  s->exp.alarm1_password.addr = 613;
  s->exp.alarm1_password.n = 46;
  s->exp.alarm1_password.flags = OS_STR;

 /* alarm1_privileges */
  s->exp.alarm1_privileges.addr = 660;
  s->exp.alarm1_privileges.n = 16;
  s->exp.alarm1_privileges.flags = OS_STR;

 /* alarm1_ip */
  s->exp.alarm1_ip.addr = 677;
  s->exp.alarm1_ip.n = 64;
  s->exp.alarm1_ip.flags = OS_STR;

 /* alarm1_count */
  s->exp.alarm1_count.addr = 742;
  s->exp.alarm1_count.n = 1;
  s->exp.alarm1_count.flags = OS_INT;

 /* alarm1_timer */
  s->exp.alarm1_timer.addr = 747;
  s->exp.alarm1_timer.n = 1;
  s->exp.alarm1_timer.flags = OS_LONG;

 /* alarm2_text */
  s->exp.alarm2_text.addr = 756;
  s->exp.alarm2_text.n = 16;
  s->exp.alarm2_text.flags = OS_STR;

 /* alarm2_name */
  s->exp.alarm2_name.addr = 773;
  s->exp.alarm2_name.n = 28;
  s->exp.alarm2_name.flags = OS_STR;

 /* alarm2_password */
  s->exp.alarm2_password.addr = 802;
  s->exp.alarm2_password.n = 46;
  s->exp.alarm2_password.flags = OS_STR;

 /* alarm2_privileges */
  s->exp.alarm2_privileges.addr = 849;
  s->exp.alarm2_privileges.n = 16;
  s->exp.alarm2_privileges.flags = OS_STR;

 /* alarm2_ip */
  s->exp.alarm2_ip.addr = 866;
  s->exp.alarm2_ip.n = 64;
  s->exp.alarm2_ip.flags = OS_STR;

 /* alarm2_count */
  s->exp.alarm2_count.addr = 931;
  s->exp.alarm2_count.n = 1;
  s->exp.alarm2_count.flags = OS_INT;

 /* alarm2_timer */
  s->exp.alarm2_timer.addr = 936;
  s->exp.alarm2_timer.n = 1;
  s->exp.alarm2_timer.flags = OS_LONG;
  s->mblk_list[0] = &s->exp.hdr;

  s->conf_imp.hdr.mblk_name = "conf_imp";
  s->conf_imp.hdr.n_signals = 7;
  s->conf_imp.hdr.mblk_sz = ACCOUNT_SIGNALS_CONF_IMP_MBLK_SZ;
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
  s->mblk_list[1] = &s->conf_imp.hdr;

  s->conf_exp.hdr.mblk_name = "conf_exp";
  s->conf_exp.hdr.n_signals = 5;
  s->conf_exp.hdr.mblk_sz = ACCOUNT_SIGNALS_CONF_EXP_MBLK_SZ;
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

  s->hdr.n_mblk_hdrs = 3;
  s->hdr.mblk_hdr = s->mblk_list;
}
