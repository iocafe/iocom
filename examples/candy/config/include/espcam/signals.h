/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct candy_t
{
  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal ambient;
    iocSignal unused_pin;
    iocSignal rec_state;
    iocSignal rec_buf;
    iocSignal rec_head;
  }
  exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal on;
    iocSignal illumination;
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

  iocStreamerSignals camera;
}
candy_t;

#define CANDY_EXP_MBLK_SZ 10018
#define CANDY_IMP_MBLK_SZ 32
#define CANDY_CONF_EXP_MBLK_SZ 272
#define CANDY_CONF_IMP_MBLK_SZ 276

extern OS_FLASH_MEM_H candy_t candy;
extern OS_FLASH_MEM_H iocDeviceHdr candy_hdr;


/* Array length defines. */
#define CANDY_EXP_REC_BUF_ARRAY_SZ 10000
#define CANDY_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define CANDY_CONF_IMP_TOD_BUF_ARRAY_SZ 257

/* Defines to check in code with #ifdef to know if signal is configured in JSON. */
#define CANDY_EXP_AMBIENT
#define CANDY_EXP_UNUSED_PIN
#define CANDY_EXP_REC_STATE
#define CANDY_EXP_REC_BUF
#define CANDY_EXP_REC_HEAD
#define CANDY_IMP_ON
#define CANDY_IMP_ILLUMINATION
#define CANDY_IMP_REC_CMD
#define CANDY_IMP_REC_SELECT
#define CANDY_IMP_REC_TAIL
#define CANDY_CONF_EXP_TOD_STATE
#define CANDY_CONF_EXP_TOD_TAIL
#define CANDY_CONF_EXP_FRD_STATE
#define CANDY_CONF_EXP_FRD_BUF
#define CANDY_CONF_EXP_FRD_HEAD
#define CANDY_CONF_IMP_TOD_CMD
#define CANDY_CONF_IMP_TOD_SELECT
#define CANDY_CONF_IMP_TOD_BUF
#define CANDY_CONF_IMP_TOD_HEAD
#define CANDY_CONF_IMP_FRD_CMD
#define CANDY_CONF_IMP_FRD_SELECT
#define CANDY_CONF_IMP_FRD_TAIL

#ifndef IOBOARD_DEVICE_NAME
#define IOBOARD_DEVICE_NAME "candy"
#endif

OSAL_C_HEADER_ENDS
