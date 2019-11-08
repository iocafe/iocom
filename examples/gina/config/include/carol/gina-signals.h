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
    iocSignal seven_segment;
    iocSignal servo;
    iocSignal dimmer_led;
    iocSignal led_builtin;
  }
  imp;
}
gina_t;

#define GINA_EXP_MBLK_SZ 44
#define GINA_IMP_MBLK_SZ 32

extern const gina_t gina;
extern const iocDeviceHdr gina_hdr;

#define GINA_IMP_SEVEN_SEGMENT_ARRAY_SZ 8

#ifndef IOBOARD_DEVICE_NAME
#define IOBOARD_DEVICE_NAME "gina"
#endif

OSAL_C_HEADER_ENDS
