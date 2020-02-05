/* This file is gerated by signals-to-c.py script, do not modify. */
const struct selectwifi_t selectwifi = 
{
  {
    {"exp", &swf.exp, 4, SELECTWIFI_EXP_MBLK_SZ, (iocSignal*)&selectwifi.exp.net_1},
    {0, 16, OS_STR, &swf.exp, OS_NULL}, /* net_1 */
    {17, 16, OS_STR, &swf.exp, OS_NULL}, /* net_2 */
    {34, 1, OS_BOOLEAN, &swf.exp, OS_NULL}, /* wifi_connected_1 */
    {35, 1, OS_BOOLEAN, &swf.exp, OS_NULL} /* wifi_connected_2 */
  },

  {
    {"imp", &swf.imp, 5, SELECTWIFI_IMP_MBLK_SZ, (iocSignal*)&selectwifi.imp.set_net_1},
    {0, 16, OS_STR, &swf.imp, OS_NULL}, /* set_net_1 */
    {17, 16, OS_STR, &swf.imp, OS_NULL}, /* set_password_1 */
    {34, 16, OS_STR, &swf.imp, OS_NULL}, /* set_net_2 */
    {51, 16, OS_STR, &swf.imp, OS_NULL}, /* set_password_2 */
    {68, 1, OS_BOOLEAN, &swf.imp, OS_NULL} /* save */
  }
};

static const iocMblkSignalHdr *selectwifi_mblk_list[] =
{
  &selectwifi.exp.hdr,
  &selectwifi.imp.hdr
};

const iocDeviceHdr selectwifi_hdr = {(iocMblkSignalHdr**)selectwifi_mblk_list, sizeof(selectwifi_mblk_list)/sizeof(iocMblkSignalHdr*)};
