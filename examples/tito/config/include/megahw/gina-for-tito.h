/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct gina_t
{
  iocDeviceHdr hdr;
  iocMblkSignalHdr *mblk_list[4];

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal frame_rate;
    iocSignal testfloat;
    iocSignal teststr;
    iocSignal testbool;
    iocSignal in_x;
    iocSignal potentiometer;
    iocSignal rec_state;
    iocSignal rec_head;
    iocSignal rec_buf;
  }
  exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal strtodevice;
    iocSignal seven_segment;
    iocSignal dimmer_led;
    iocSignal myoutput;
    iocSignal rec_cmd;
    iocSignal rec_select;
    iocSignal rec_tail;
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

  iocStreamerSignals ccd;
}
gina_t;

#define GINA_EXP_MBLK_SZ 5052
#define GINA_IMP_MBLK_SZ 32
#define GINA_CONF_EXP_MBLK_SZ 272
#define GINA_CONF_IMP_MBLK_SZ 276

void gina_init_signal_struct(gina_t *s);

/* Array length defines. */
#define GINA_EXP_TESTFLOAT_ARRAY_SZ 5
#define GINA_EXP_TESTSTR_ARRAY_SZ 10
#define GINA_EXP_REC_BUF_ARRAY_SZ 5000
#define GINA_IMP_STRTODEVICE_ARRAY_SZ 16
#define GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ 8
#define GINA_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define GINA_CONF_IMP_TOD_BUF_ARRAY_SZ 257

/* Defines to check in code with #ifdef to know if signal is configured in JSON. */
#define GINA_EXP_FRAME_RATE
#define GINA_EXP_TESTFLOAT
#define GINA_EXP_TESTSTR
#define GINA_EXP_TESTBOOL
#define GINA_EXP_IN_X
#define GINA_EXP_POTENTIOMETER
#define GINA_EXP_REC_STATE
#define GINA_EXP_REC_HEAD
#define GINA_EXP_REC_BUF
#define GINA_IMP_STRTODEVICE
#define GINA_IMP_SEVEN_SEGMENT
#define GINA_IMP_DIMMER_LED
#define GINA_IMP_MYOUTPUT
#define GINA_IMP_REC_CMD
#define GINA_IMP_REC_SELECT
#define GINA_IMP_REC_TAIL
#define GINA_CONF_EXP_TOD_STATE
#define GINA_CONF_EXP_TOD_TAIL
#define GINA_CONF_EXP_FRD_STATE
#define GINA_CONF_EXP_FRD_HEAD
#define GINA_CONF_EXP_FRD_BUF
#define GINA_CONF_IMP_TOD_CMD
#define GINA_CONF_IMP_TOD_SELECT
#define GINA_CONF_IMP_TOD_HEAD
#define GINA_CONF_IMP_TOD_BUF
#define GINA_CONF_IMP_FRD_CMD
#define GINA_CONF_IMP_FRD_SELECT
#define GINA_CONF_IMP_FRD_TAIL

OSAL_C_HEADER_ENDS
