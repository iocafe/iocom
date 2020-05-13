/* This file is gerated by pins_to_c.py script, do not modify. */
#include "pins.h"

/* Parameters for inputs */
static os_ushort pins_inputs_gazerbeam_prm[]= {PIN_RV, PIN_RV, PIN_INTERRUPT_ENABLED, 1};
PINS_INTCONF_STRUCT(pin_gazerbeam_intconf)

/* Parameters for outputs */
static os_ushort pins_outputs_led_morse_prm[]= {PIN_RV, PIN_RV};
static os_ushort pins_outputs_led_builtin_prm[]= {PIN_RV, PIN_RV};

/* Parameters for analog_inputs */
static os_ushort pins_analog_inputs_ambient_prm[]= {PIN_RV, PIN_RV, PIN_MAX, 4095};
static os_ushort pins_analog_inputs_unused_pin_prm[]= {PIN_RV, PIN_RV, PIN_MAX, 4095};

/* Parameters for pwm */
static os_ushort pins_pwm_headlight_prm[]= {PIN_RV, PIN_RV, PIN_MAX, 4095, PIN_RESOLUTION, 12, PIN_FREQENCY, 5000, PIN_INIT, 0};

/* Parameters for cameras */
static os_ushort pins_cameras_camera_prm[]= {PIN_RV, PIN_RV};

/* Parameters for uart */
static os_ushort pins_uart_uart2_prm[]= {PIN_RV, PIN_RV, PIN_RX, 16, PIN_TX, 2, PIN_SPEED, 1152};

/* CANDY IO configuration structure */
OS_FLASH_MEM pins_t pins =
{
  {{1, &pins.inputs.gazerbeam}, /* inputs */
    {PIN_INPUT, 0, 12, pins_inputs_gazerbeam_prm, sizeof(pins_inputs_gazerbeam_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_PTR(pin_gazerbeam_intconf)} /* gazerbeam */
  },

  {{2, &pins.outputs.led_morse}, /* outputs */
    {PIN_OUTPUT, 0, 15, pins_outputs_led_morse_prm, sizeof(pins_outputs_led_morse_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL}, /* led_morse */
    {PIN_OUTPUT, 0, 33, pins_outputs_led_builtin_prm, sizeof(pins_outputs_led_builtin_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* led_builtin */
  },

  {{2, &pins.analog_inputs.ambient}, /* analog_inputs */
    {PIN_ANALOG_INPUT, 0, 13, pins_analog_inputs_ambient_prm, sizeof(pins_analog_inputs_ambient_prm)/sizeof(os_ushort), OS_NULL, &candy.exp.ambient PINS_INTCONF_NULL}, /* ambient */
    {PIN_ANALOG_INPUT, 0, 14, pins_analog_inputs_unused_pin_prm, sizeof(pins_analog_inputs_unused_pin_prm)/sizeof(os_ushort), OS_NULL, &candy.exp.unused_pin PINS_INTCONF_NULL} /* unused_pin */
  },

  {{1, &pins.pwm.headlight}, /* pwm */
    {PIN_PWM, 1, 4, pins_pwm_headlight_prm, sizeof(pins_pwm_headlight_prm)/sizeof(os_ushort), OS_NULL, &candy.imp.headlight PINS_INTCONF_NULL} /* headlight */
  },

  {{1, &pins.cameras.camera}, /* cameras */
    {PIN_CAMERA, 0, 0, pins_cameras_camera_prm, sizeof(pins_cameras_camera_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* camera */
  },

  {{1, &pins.uart.uart2}, /* uart */
    {PIN_UART, 0, 0, pins_uart_uart2_prm, sizeof(pins_uart_uart2_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* uart2 */
  },
};

/* List of pin type groups */
static OS_FLASH_MEM PinGroupHdr * OS_FLASH_MEM pins_group_list[] =
{
  &pins.inputs.hdr,
  &pins.outputs.hdr,
  &pins.analog_inputs.hdr,
  &pins.pwm.hdr,
  &pins.cameras.hdr,
  &pins.uart.hdr
};

/* CANDY IO configuration top header structure */
OS_FLASH_MEM IoPinsHdr pins_hdr = {pins_group_list, sizeof(pins_group_list)/sizeof(PinGroupHdr*)};
