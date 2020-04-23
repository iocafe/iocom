/* This file is gerated by signals-to-c.py script, do not modify. */
const struct candy_t candy = 
{
  {
    {"exp", &ioboard_exp, 9, CANDY_EXP_MBLK_SZ, (iocSignal*)&candy.exp.frame_rate},
    {0, 1, OS_FLOAT, &ioboard_exp, OS_NULL}, /* frame_rate */
    {5, 5, OS_FLOAT, &ioboard_exp, OS_NULL}, /* testfloat */
    {26, 10, OS_STR, &ioboard_exp, OS_NULL}, /* teststr */
    {37, 1, OS_BOOLEAN, &ioboard_exp, OS_NULL}, /* testbool */
    {38, 1, OS_BOOLEAN, &ioboard_exp, OS_NULL}, /* in_x */
    {39, 1, OS_FLOAT, &ioboard_exp, OS_NULL}, /* potentiometer */
    {44, 1, OS_CHAR, &ioboard_exp, OS_NULL}, /* rec_state */
    {46, 2800, OS_UCHAR, &ioboard_exp, OS_NULL}, /* rec_buf */
    {2847, 1, OS_INT, &ioboard_exp, OS_NULL} /* rec_head */
  },

  {
    {"imp", &ioboard_imp, 7, CANDY_IMP_MBLK_SZ, (iocSignal*)&candy.imp.strtodevice},
    {0, 16, OS_STR, &ioboard_imp, OS_NULL}, /* strtodevice */
    {17, 8, OS_BOOLEAN, &ioboard_imp, OS_NULL}, /* seven_segment */
    {19, 1, OS_USHORT, &ioboard_imp, OS_NULL}, /* dimmer_led */
    {22, 1, OS_BOOLEAN, &ioboard_imp, OS_NULL}, /* myoutput */
    {23, 1, OS_CHAR, &ioboard_imp, OS_NULL}, /* rec_cmd */
    {25, 1, OS_UCHAR, &ioboard_imp, OS_NULL}, /* rec_select */
    {27, 1, OS_INT, &ioboard_imp, OS_NULL} /* rec_tail */
  },

  {
    {"conf_exp", &ioboard_conf_exp, 5, CANDY_CONF_EXP_MBLK_SZ, (iocSignal*)&candy.conf_exp.tod_state},
    {0, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* tod_state */
    {2, 1, OS_INT, &ioboard_conf_exp, OS_NULL}, /* tod_tail */
    {7, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* frd_state */
    {9, 257, OS_UCHAR, &ioboard_conf_exp, OS_NULL}, /* frd_buf */
    {267, 1, OS_INT, &ioboard_conf_exp, OS_NULL} /* frd_head */
  },

  {
    {"conf_imp", &ioboard_conf_imp, 7, CANDY_CONF_IMP_MBLK_SZ, (iocSignal*)&candy.conf_imp.tod_cmd},
    {0, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_cmd */
    {2, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_select */
    {4, 257, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* tod_buf */
    {262, 1, OS_INT, &ioboard_conf_imp, OS_NULL}, /* tod_head */
    {267, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* frd_cmd */
    {269, 1, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* frd_select */
    {271, 1, OS_INT, &ioboard_conf_imp, OS_NULL} /* frd_tail */
  },

  /* Signals for linecam 'ccd' */
  {&candy.imp.rec_cmd,
   &candy.imp.rec_select,
   &candy.exp.rec_buf,
   &candy.exp.rec_head,
   &candy.imp.rec_tail,
   &candy.exp.rec_state,
   OS_FALSE}
};

static const iocMblkSignalHdr *candy_mblk_list[] =
{
  &candy.exp.hdr,
  &candy.imp.hdr,
  &candy.conf_exp.hdr,
  &candy.conf_imp.hdr
};

const iocDeviceHdr candy_hdr = {(iocMblkSignalHdr**)candy_mblk_list, sizeof(candy_mblk_list)/sizeof(iocMblkSignalHdr*)};
