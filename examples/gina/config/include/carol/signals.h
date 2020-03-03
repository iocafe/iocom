/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

typedef struct gina_t
{
  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal dip_switch_3;
    iocSignal dip_switch_4;
    iocSignal touch_sensor;
    iocSignal potentiometer;
    iocSignal testfloat;
    iocSignal teststr;
    iocSignal A;
    iocSignal B;
    iocSignal C;
    iocSignal D;
    iocSignal E;
    iocSignal F;
    iocSignal G;
    iocSignal H;
  }
  exp;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal strtodevice;
    iocSignal seven_segment;
    iocSignal servo;
    iocSignal dimmer_led;
    iocSignal led_builtin_x;
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
gina_t;

#define GINA_EXP_MBLK_SZ 46
#define GINA_IMP_MBLK_SZ 32
#define GINA_CONF_EXP_MBLK_SZ 272
#define GINA_CONF_IMP_MBLK_SZ 276

extern const gina_t gina;
extern const iocDeviceHdr gina_hdr;

#define GINA_EXP_TESTFLOAT_ARRAY_SZ 5
#define GINA_EXP_TESTSTR_ARRAY_SZ 10
#define GINA_IMP_STRTODEVICE_ARRAY_SZ 16
#define GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ 8
#define GINA_CONF_EXP_FRD_BUF_ARRAY_SZ 257
#define GINA_CONF_IMP_TOD_BUF_ARRAY_SZ 257

#ifndef IOBOARD_DEVICE_NAME
#define IOBOARD_DEVICE_NAME "gina"
#endif

OSAL_C_HEADER_ENDS
