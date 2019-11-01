/* This file is gerated by signals-to-c.py script, do not modify. */
#include "iocom.h"
iocSignal com_DIP_SWITCH_3 = {30, 1, OS_BOOLEAN, 0,  OS_NULL};
iocSignal com_DIP_SWITCH_4 = {31, 1, OS_BOOLEAN, 0,  &com_DIP_SWITCH_3};
iocSignal com_TOUCH_SENSOR = {32, 1, OS_BOOLEAN, 0,  &com_DIP_SWITCH_4};
iocSignal com_POTENTIOMETER = {33, 1, OS_USHORT, 0,  &com_TOUCH_SENSOR};
iocSignal *com_EXPORT_signals = &com_POTENTIOMETER;
iocSignal com_SERVO = {0, 1, OS_SHORT, 0,  OS_NULL};
iocSignal com_DIMMER_LED = {3, 1, OS_SHORT, 0,  &com_SERVO};
iocSignal com_LED_BUILTIN = {6, 1, OS_BOOLEAN, 0,  &com_DIMMER_LED};
iocSignal com_SEVEN_SEGMENT = {7, 8, OS_BOOLEAN, 0,  &com_LED_BUILTIN};
iocSignal *com_IMPORT_signals = &com_SEVEN_SEGMENT;
