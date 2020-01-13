/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct account_signals_t
{
  iocDeviceHdr hdr;
  iocMblkSignalHdr *mblk_list[2];

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
account_signals_t;

#define ACCOUNT_SIGNALS_CONF_EXP_MBLK_SZ 272
#define ACCOUNT_SIGNALS_CONF_IMP_MBLK_SZ 276

void account_signals_init_signal_struct(account_signals_t *s);
#define ACCOUNT_SIGNALS_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define ACCOUNT_SIGNALS_CONF_IMP_TOD_BUF_ARRAY_SZ 257

OSAL_C_HEADER_ENDS
