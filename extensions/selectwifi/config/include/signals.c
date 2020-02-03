/* This file is gerated by signals-to-c.py script, do not modify. */
const struct selectwifi_t selectwifi = 
{
  {
    {"exp", &ioboard_exp, 2, SELECTWIFI_EXP_MBLK_SZ, (iocSignal*)&selectwifi.exp.net_1},
    {0, 16, OS_STR, &ioboard_exp, OS_NULL}, /* net_1 */
    {17, 16, OS_STR, &ioboard_exp, OS_NULL} /* net_2 */
  },

  {
    {"imp", &ioboard_imp, 4, SELECTWIFI_IMP_MBLK_SZ, (iocSignal*)&selectwifi.imp.set_net_1},
    {0, 16, OS_STR, &ioboard_imp, OS_NULL}, /* set_net_1 */
    {17, 16, OS_STR, &ioboard_imp, OS_NULL}, /* set_password_1 */
    {34, 16, OS_STR, &ioboard_imp, OS_NULL}, /* set_net_2 */
    {51, 16, OS_STR, &ioboard_imp, OS_NULL} /* set_password_2 */
  }
};

static const iocMblkSignalHdr *selectwifi_mblk_list[] =
{
  &selectwifi.exp.hdr,
  &selectwifi.imp.hdr
};

const iocDeviceHdr selectwifi_hdr = {(iocMblkSignalHdr**)selectwifi_mblk_list, sizeof(selectwifi_mblk_list)/sizeof(iocMblkSignalHdr*)};
