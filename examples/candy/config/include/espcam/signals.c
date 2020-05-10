/* This file is gerated by signals-to-c.py script, do not modify. */
OS_FLASH_MEM struct candy_t candy = 
{
  {
    {"exp", &ioboard_exp, 5, CANDY_EXP_MBLK_SZ, (iocSignal*)&candy.exp.ambient},
    {0, 1, OS_UINT|IOC_PIN_PTR, &ioboard_exp, &pins.analog_inputs.ambient}, /* ambient */
    {5, 1, OS_UINT|IOC_PIN_PTR, &ioboard_exp, &pins.analog_inputs.unused_pin}, /* unused_pin */
    {10, 1, OS_CHAR, &ioboard_exp, OS_NULL}, /* rec_state */
    {12, 1, OS_INT, &ioboard_exp, OS_NULL}, /* rec_head */
    {17, 5000, OS_UCHAR, &ioboard_exp, OS_NULL} /* rec_buf */
  },

  {
    {"imp", &ioboard_imp, 5, CANDY_IMP_MBLK_SZ, (iocSignal*)&candy.imp.on},
    {0, 1, OS_BOOLEAN, &ioboard_imp, OS_NULL}, /* on */
    {1, 1, OS_UINT|IOC_PIN_PTR, &ioboard_imp, &pins.pwm.illumination}, /* illumination */
    {6, 1, OS_CHAR, &ioboard_imp, OS_NULL}, /* rec_cmd */
    {8, 1, OS_UCHAR, &ioboard_imp, OS_NULL}, /* rec_select */
    {10, 1, OS_INT, &ioboard_imp, OS_NULL} /* rec_tail */
  },

  {
    {"conf_exp", &ioboard_conf_exp, 5, CANDY_CONF_EXP_MBLK_SZ, (iocSignal*)&candy.conf_exp.tod_state},
    {0, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* tod_state */
    {2, 1, OS_INT, &ioboard_conf_exp, OS_NULL}, /* tod_tail */
    {7, 1, OS_CHAR, &ioboard_conf_exp, OS_NULL}, /* frd_state */
    {9, 1, OS_INT, &ioboard_conf_exp, OS_NULL}, /* frd_head */
    {14, 257, OS_UCHAR, &ioboard_conf_exp, OS_NULL} /* frd_buf */
  },

  {
    {"conf_imp", &ioboard_conf_imp, 7, CANDY_CONF_IMP_MBLK_SZ, (iocSignal*)&candy.conf_imp.tod_cmd},
    {0, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_cmd */
    {2, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* tod_select */
    {4, 1, OS_INT, &ioboard_conf_imp, OS_NULL}, /* tod_head */
    {9, 257, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* tod_buf */
    {267, 1, OS_CHAR, &ioboard_conf_imp, OS_NULL}, /* frd_cmd */
    {269, 1, OS_UCHAR, &ioboard_conf_imp, OS_NULL}, /* frd_select */
    {271, 1, OS_INT, &ioboard_conf_imp, OS_NULL} /* frd_tail */
  },

  /* Signals for camera 'camera' */
  {&candy.imp.rec_cmd,
   &candy.imp.rec_select,
   &candy.exp.rec_buf,
   &candy.exp.rec_head,
   &candy.imp.rec_tail,
   &candy.exp.rec_state,
   OS_FALSE}
};

static OS_FLASH_MEM iocMblkSignalHdr * OS_FLASH_MEM candy_mblk_list[] =
{
  &candy.exp.hdr,
  &candy.imp.hdr,
  &candy.conf_exp.hdr,
  &candy.conf_imp.hdr
};

OS_FLASH_MEM iocDeviceHdr candy_hdr = {(iocMblkSignalHdr**)candy_mblk_list, sizeof(candy_mblk_list)/sizeof(iocMblkSignalHdr*)};
