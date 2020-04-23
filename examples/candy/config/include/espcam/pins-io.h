/* This file is gerated by pins-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

/* CANDY IO configuration structure */
typedef struct
{
  struct
  {
    PinGroupHdr hdr;
    Pin gazerbeam;
  }
  inputs;

  struct
  {
    PinGroupHdr hdr;
    Pin led_builtin;
  }
  outputs;

  struct
  {
    PinGroupHdr hdr;
    Pin ambient;
    Pin unused_pin;
  }
  analog_inputs;

  struct
  {
    PinGroupHdr hdr;
    Pin illumination;
  }
  pwm;

  struct
  {
    PinGroupHdr hdr;
    Pin camera;
  }
  cameras;

  struct
  {
    PinGroupHdr hdr;
    Pin uart2;
  }
  uart;
}
pins_t;

/* CANDY IO configuration top header structure */
extern const IoPinsHdr pins_hdr;

/* Global CANDY IO configuration structure */
extern const pins_t pins;

/* Name defines for pins and application pin groups (use ifdef to check if HW has pin) */
#define PINS_INPUTS_GAZERBEAM "gazerbeam"
#define PINS_OUTPUTS_LED_BUILTIN "led_builtin"
#define PINS_ANALOG_INPUTS_AMBIENT "ambient"
#define PINS_ANALOG_INPUTS_UNUSED_PIN "unused_pin"
#define PINS_PWM_ILLUMINATION "illumination"
#define PINS_CAMERAS_CAMERA "camera"
#define PINS_UART_UART2 "uart2"

OSAL_C_HEADER_ENDS
