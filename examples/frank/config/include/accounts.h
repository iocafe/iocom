/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct accounts_t
{
  iocDeviceHdr hdr;
  iocMblkSignalHdr *mblk_list[4];

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal nro_devices;
    iocSignal test;
  }
  exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal restart;
  }
  imp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal tod_state;
    iocSignal tod_tail;
    iocSignal frd_state;
    iocSignal frd_buf;
    iocSignal frd_head;
  }
  conf_exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal tod_cmd;
    iocSignal tod_select;
    iocSignal tod_buf;
    iocSignal tod_head;
    iocSignal frd_cmd;
    iocSignal frd_select;
    iocSignal frd_tail;
  }
  conf_imp;
}
accounts_t;

#define ACCOUNTS_EXP_MBLK_SZ 32
#define ACCOUNTS_IMP_MBLK_SZ 32
#define ACCOUNTS_CONF_EXP_MBLK_SZ 272
#define ACCOUNTS_CONF_IMP_MBLK_SZ 276

void accounts_init_signal_struct(accounts_t *s);
#define ACCOUNTS_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define ACCOUNTS_CONF_IMP_TOD_BUF_ARRAY_SZ 257

OSAL_C_HEADER_ENDS
