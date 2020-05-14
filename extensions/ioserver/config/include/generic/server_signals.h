/* This file is gerated by signals_to_c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct account_signals_t
{
  iocDeviceHdr hdr;
  iocMblkSignalHdr *mblk_list[3];

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal new1_text;
    iocSignal new1_name;
    iocSignal new1_password;
    iocSignal new1_privileges;
    iocSignal new1_ip;
    iocSignal new1_count;
    iocSignal new2_text;
    iocSignal new2_name;
    iocSignal new2_password;
    iocSignal new2_privileges;
    iocSignal new2_ip;
    iocSignal new2_count;
    iocSignal new3_text;
    iocSignal new3_name;
    iocSignal new3_password;
    iocSignal new3_privileges;
    iocSignal new3_ip;
    iocSignal new3_count;
  }
  exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal tod_state;
    iocSignal tod_tail;
    iocSignal frd_state;
    iocSignal frd_head;
    iocSignal frd_buf;
  }
  conf_exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal tod_cmd;
    iocSignal tod_select;
    iocSignal tod_head;
    iocSignal tod_buf;
    iocSignal frd_cmd;
    iocSignal frd_select;
    iocSignal frd_tail;
  }
  conf_imp;
}
account_signals_t;

#define ACCOUNT_SIGNALS_EXP_MBLK_SZ 540
#define ACCOUNT_SIGNALS_CONF_EXP_MBLK_SZ 272
#define ACCOUNT_SIGNALS_CONF_IMP_MBLK_SZ 276

void account_signals_init_signal_struct(account_signals_t *s);

/* Array length defines. */
#define ACCOUNT_SIGNALS_EXP_NEW1_TEXT_ARRAY_SZ 16
#define ACCOUNT_SIGNALS_EXP_NEW1_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_NEW1_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_NEW1_PRIVILEGES_ARRAY_SZ 16
#define ACCOUNT_SIGNALS_EXP_NEW1_IP_ARRAY_SZ 64
#define ACCOUNT_SIGNALS_EXP_NEW2_TEXT_ARRAY_SZ 16
#define ACCOUNT_SIGNALS_EXP_NEW2_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_NEW2_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_NEW2_PRIVILEGES_ARRAY_SZ 16
#define ACCOUNT_SIGNALS_EXP_NEW2_IP_ARRAY_SZ 64
#define ACCOUNT_SIGNALS_EXP_NEW3_TEXT_ARRAY_SZ 16
#define ACCOUNT_SIGNALS_EXP_NEW3_NAME_ARRAY_SZ 28
#define ACCOUNT_SIGNALS_EXP_NEW3_PASSWORD_ARRAY_SZ 46
#define ACCOUNT_SIGNALS_EXP_NEW3_PRIVILEGES_ARRAY_SZ 16
#define ACCOUNT_SIGNALS_EXP_NEW3_IP_ARRAY_SZ 64
#define ACCOUNT_SIGNALS_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define ACCOUNT_SIGNALS_CONF_IMP_TOD_BUF_ARRAY_SZ 257

/* Defines to check in code with #ifdef to know if signal is configured in JSON. */
#define ACCOUNT_SIGNALS_EXP_NEW1_TEXT
#define ACCOUNT_SIGNALS_EXP_NEW1_NAME
#define ACCOUNT_SIGNALS_EXP_NEW1_PASSWORD
#define ACCOUNT_SIGNALS_EXP_NEW1_PRIVILEGES
#define ACCOUNT_SIGNALS_EXP_NEW1_IP
#define ACCOUNT_SIGNALS_EXP_NEW1_COUNT
#define ACCOUNT_SIGNALS_EXP_NEW2_TEXT
#define ACCOUNT_SIGNALS_EXP_NEW2_NAME
#define ACCOUNT_SIGNALS_EXP_NEW2_PASSWORD
#define ACCOUNT_SIGNALS_EXP_NEW2_PRIVILEGES
#define ACCOUNT_SIGNALS_EXP_NEW2_IP
#define ACCOUNT_SIGNALS_EXP_NEW2_COUNT
#define ACCOUNT_SIGNALS_EXP_NEW3_TEXT
#define ACCOUNT_SIGNALS_EXP_NEW3_NAME
#define ACCOUNT_SIGNALS_EXP_NEW3_PASSWORD
#define ACCOUNT_SIGNALS_EXP_NEW3_PRIVILEGES
#define ACCOUNT_SIGNALS_EXP_NEW3_IP
#define ACCOUNT_SIGNALS_EXP_NEW3_COUNT
#define ACCOUNT_SIGNALS_CONF_EXP_TOD_STATE
#define ACCOUNT_SIGNALS_CONF_EXP_TOD_TAIL
#define ACCOUNT_SIGNALS_CONF_EXP_FRD_STATE
#define ACCOUNT_SIGNALS_CONF_EXP_FRD_HEAD
#define ACCOUNT_SIGNALS_CONF_EXP_FRD_BUF
#define ACCOUNT_SIGNALS_CONF_IMP_TOD_CMD
#define ACCOUNT_SIGNALS_CONF_IMP_TOD_SELECT
#define ACCOUNT_SIGNALS_CONF_IMP_TOD_HEAD
#define ACCOUNT_SIGNALS_CONF_IMP_TOD_BUF
#define ACCOUNT_SIGNALS_CONF_IMP_FRD_CMD
#define ACCOUNT_SIGNALS_CONF_IMP_FRD_SELECT
#define ACCOUNT_SIGNALS_CONF_IMP_FRD_TAIL

OSAL_C_HEADER_ENDS
