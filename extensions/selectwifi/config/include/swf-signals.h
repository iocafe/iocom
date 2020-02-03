/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct selectwifi_t
{
  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal net_1;
    iocSignal net_2;
    iocSignal wifi_connected_1;
    iocSignal wifi_connected_2;
  }
  exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal set_net_1;
    iocSignal set_password_1;
    iocSignal set_net_2;
    iocSignal set_password_2;
  }
  imp;
}
selectwifi_t;

#define SELECTWIFI_EXP_MBLK_SZ 36
#define SELECTWIFI_IMP_MBLK_SZ 68

extern const selectwifi_t selectwifi;
extern const iocDeviceHdr selectwifi_hdr;

#define SELECTWIFI_EXP_NET_1_ARRAY_SZ 16
#define SELECTWIFI_EXP_NET_2_ARRAY_SZ 16
#define SELECTWIFI_IMP_SET_NET_1_ARRAY_SZ 16
#define SELECTWIFI_IMP_SET_PASSWORD_1_ARRAY_SZ 16
#define SELECTWIFI_IMP_SET_NET_2_ARRAY_SZ 16
#define SELECTWIFI_IMP_SET_PASSWORD_2_ARRAY_SZ 16

#ifndef IOBOARD_DEVICE_NAME
#define IOBOARD_DEVICE_NAME "selectwifi"
#endif

OSAL_C_HEADER_ENDS
