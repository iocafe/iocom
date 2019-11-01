/* This file is gerated by signals-to-c.py script, do not modify. */
#include "iocom.h"
signal_EXPORT_t signal_EXPORT
= {{&ioboard_EXPORT, 4, &signal_EXPORT.DIP_SWITCH_3},
  {30, 1, OS_BOOLEAN, 0, &ioboard_EXPORT, &io_DIP_SWITCH_3}, /* DIP_SWITCH_3 */
  {31, 1, OS_BOOLEAN, 0, &ioboard_EXPORT, &io_DIP_SWITCH_4}, /* DIP_SWITCH_4 */
  {32, 1, OS_BOOLEAN, 0, &ioboard_EXPORT, &io_TOUCH_SENSOR}, /* TOUCH_SENSOR */
  {33, 1, OS_USHORT, 0, &ioboard_EXPORT, &io_POTENTIOMETER} /* POTENTIOMETER */
};

signal_IMPORT_t signal_IMPORT
= {{&ioboard_IMPORT, 4, &signal_IMPORT.SERVO},
  {0, 1, OS_SHORT, 0, &ioboard_IMPORT, &io_SERVO}, /* SERVO */
  {3, 1, OS_SHORT, 0, &ioboard_IMPORT, &io_DIMMER_LED}, /* DIMMER_LED */
  {6, 1, OS_BOOLEAN, 0, &ioboard_IMPORT, &io_LED_BUILTIN}, /* LED_BUILTIN */
  {7, 8, OS_BOOLEAN, 0, &ioboard_IMPORT} /* SEVEN_SEGMENT */
};

