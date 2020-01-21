/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct account_signals_t
{
  iocDeviceHdr hdr;
  iocMblkSignalHdr *mblk_list[3];

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal new1_name;
    iocSignal new1_password;
    iocSignal new1_count;
    iocSignal new2_name;
    iocSignal new2_password;
    iocSignal new2_count;
    iocSignal new3_name;
    iocSignal new3_password;
    iocSignal new3_count;
    iocSignal alarm1_name;
    iocSignal alarm1_password;
    iocSignal alarm1_count;
    iocSignal alarm1_text;
    iocSignal alarm2_name;
    iocSignal alarm2_password;
    iocSignal alarm2_count;
    iocSignal alarm2_text;
  }
  exp;

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
}
account_signals_t;

#define ACCOUNT_SIGNALS_EXP_MBLK_SZ 467
#define ACCOUNT_SIGNALS_CONF_IMP_MBLK_SZ 276
#define ACCOUNT_SIGNALS_CONF_EXP_MBLK_SZ 272

void account_signals_init_signal_struct(account_signals_t *s);
#define ACCOUNT_SIGNALS_EXP_NEW1_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_NEW1_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_NEW2_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_NEW2_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_NEW3_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_NEW3_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_ALARM1_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_ALARM1_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_ALARM1_TEXT_ARRAY_SZ 30
#define ACCOUNT_SIGNALS_EXP_ALARM2_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_ALARM2_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_ALARM2_TEXT_ARRAY_SZ 30
#define ACCOUNT_SIGNALS_CONF_IMP_TOD_BUF_ARRAY_SZ 257
#define ACCOUNT_SIGNALS_CONF_EXP_FRD_BUF_ARRAY_SZ 257

OSAL_C_HEADER_ENDS
