/* This file is gerated by pins-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

/* GINA IO configuration structure */
typedef struct
{
  struct
  {
    PinGroupHdr hdr;
    Pin dip_switch_3;
    Pin dip_switch_4;
    Pin touch_sensor;
  }
  inputs;

  struct
  {
    PinGroupHdr hdr;
    Pin led_builtin;
    Pin A;
    Pin B;
    Pin C;
    Pin D;
    Pin E;
    Pin F;
    Pin G;
    Pin H;
  }
  outputs;

  struct
  {
    PinGroupHdr hdr;
    Pin potentiometer;
  }
  analog_inputs;

  struct
  {
    PinGroupHdr hdr;
    Pin servo;
    Pin dimmer_led;
  }
  pwm;
}
pins_t;

/* GINA IO configuration top header structure */
extern const IoPinsHdr pins_hdr;

/* Global GINA IO configuration structure */
extern const pins_t pins;

/* Application's pin groups (linked list heads) */
extern const Pin *pins_segment7_group;

/* Name defines for pins and application pin groups (use ifdef to check if HW has pin) */
#define PINS_INPUTS_DIP_SWITCH_3 "dip_switch_3"
#define PINS_INPUTS_DIP_SWITCH_4 "dip_switch_4"
#define PINS_INPUTS_TOUCH_SENSOR "touch_sensor"
#define PINS_OUTPUTS_LED_BUILTIN "led_builtin"
#define PINS_OUTPUTS_A "A"
#define PINS_SEGMENT7_GROUP "segment7"
#define PINS_OUTPUTS_B "B"
#define PINS_OUTPUTS_C "C"
#define PINS_OUTPUTS_D "D"
#define PINS_OUTPUTS_E "E"
#define PINS_OUTPUTS_F "F"
#define PINS_OUTPUTS_G "G"
#define PINS_OUTPUTS_H "H"
#define PINS_ANALOG_INPUTS_POTENTIOMETER "potentiometer"
#define PINS_PWM_SERVO "servo"
#define PINS_PWM_DIMMER_LED "dimmer_led"

OSAL_C_HEADER_ENDS
