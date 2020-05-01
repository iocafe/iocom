/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct tito_t
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
tito_t;

#define TITO_EXP_MBLK_SZ 32
#define TITO_IMP_MBLK_SZ 32
#define TITO_CONF_EXP_MBLK_SZ 272
#define TITO_CONF_IMP_MBLK_SZ 276

void tito_init_signal_struct(tito_t *s);

/* Array length defines. */
#define TITO_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define TITO_CONF_IMP_TOD_BUF_ARRAY_SZ 257

/* Defines to check in code with #ifdef to know if signal is configured in JSON. */
#define TITO_EXP_NRO_DEVICES
#define TITO_EXP_TEST
#define TITO_IMP_RESTART
#define TITO_CONF_EXP_TOD_STATE
#define TITO_CONF_EXP_TOD_TAIL
#define TITO_CONF_EXP_FRD_STATE
#define TITO_CONF_EXP_FRD_BUF
#define TITO_CONF_EXP_FRD_HEAD
#define TITO_CONF_IMP_TOD_CMD
#define TITO_CONF_IMP_TOD_SELECT
#define TITO_CONF_IMP_TOD_BUF
#define TITO_CONF_IMP_TOD_HEAD
#define TITO_CONF_IMP_FRD_CMD
#define TITO_CONF_IMP_FRD_SELECT
#define TITO_CONF_IMP_FRD_TAIL

OSAL_C_HEADER_ENDS
