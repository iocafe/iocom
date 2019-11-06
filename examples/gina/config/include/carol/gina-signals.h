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
  up;

  struct 
  {
    iocMblkSignalHdr hdr;
    iocSignal seven_segment;
    iocSignal servo;
    iocSignal dimmer_led;
    iocSignal led_builtin;
  }
  down;
}
gina_t;

extern const iocDeviceHdr gina_hdr;

void gina_init_signal_struct(gina_t *s, gina_init_prm_t *prm);

#ifndef IOBOARD_DEVICE_NAME
#define IOBOARD_DEVICE_NAME "gina"
#endif

OSAL_C_HEADER_ENDS
