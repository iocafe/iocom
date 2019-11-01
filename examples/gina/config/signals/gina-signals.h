/* This file is gerated by signals-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS
typedef struct signal_EXPORT_t
{
  iocSignalStructHeader hdr;
  iocSignal DIP_SWITCH_3;
  iocSignal DIP_SWITCH_4;
  iocSignal TOUCH_SENSOR;
  iocSignal POTENTIOMETER;
}
signal_EXPORT_t;

extern signal_EXPORT_t signal_EXPORT;
#define SIGNAL_EXPORT_MBLK_SZ 36

typedef struct signal_IMPORT_t
{
  iocSignalStructHeader hdr;
  iocSignal SERVO;
  iocSignal DIMMER_LED;
  iocSignal LED_BUILTIN;
  iocSignal SEVEN_SEGMENT;
}
signal_IMPORT_t;

extern signal_IMPORT_t signal_IMPORT;
#define SIGNAL_IMPORT_MBLK_SZ 32

OSAL_C_HEADER_ENDS
