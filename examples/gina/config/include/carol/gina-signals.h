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

#define GINA_UP_MBLK_SZ 44
#define GINA_DOWN_MBLK_SZ 32

extern gina_t gina;
extern const iocDeviceHdr gina_hdr;

#define GINA_DOWN_SEVEN_SEGMENT_ARRAY_SZ 8

#define IOBOARD_DEVICE_NAME "gina"

OSAL_C_HEADER_ENDS
