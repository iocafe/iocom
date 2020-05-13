/* This file is gerated by signals_to_c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct claudia_t
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
claudia_t;

#define CLAUDIA_EXP_MBLK_SZ 32
#define CLAUDIA_IMP_MBLK_SZ 32
#define CLAUDIA_CONF_EXP_MBLK_SZ 272
#define CLAUDIA_CONF_IMP_MBLK_SZ 276

void claudia_init_signal_struct(claudia_t *s);

/* Array length defines. */
#define CLAUDIA_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define CLAUDIA_CONF_IMP_TOD_BUF_ARRAY_SZ 257

/* Defines to check in code with #ifdef to know if signal is configured in JSON. */
#define CLAUDIA_EXP_NRO_DEVICES
#define CLAUDIA_EXP_TEST
#define CLAUDIA_IMP_RESTART
#define CLAUDIA_CONF_EXP_TOD_STATE
#define CLAUDIA_CONF_EXP_TOD_TAIL
#define CLAUDIA_CONF_EXP_FRD_STATE
#define CLAUDIA_CONF_EXP_FRD_HEAD
#define CLAUDIA_CONF_EXP_FRD_BUF
#define CLAUDIA_CONF_IMP_TOD_CMD
#define CLAUDIA_CONF_IMP_TOD_SELECT
#define CLAUDIA_CONF_IMP_TOD_HEAD
#define CLAUDIA_CONF_IMP_TOD_BUF
#define CLAUDIA_CONF_IMP_FRD_CMD
#define CLAUDIA_CONF_IMP_FRD_SELECT
#define CLAUDIA_CONF_IMP_FRD_TAIL

OSAL_C_HEADER_ENDS
