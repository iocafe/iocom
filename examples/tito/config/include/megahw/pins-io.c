/* This file is gerated by pins_to_c.py script, do not modify. */
#include "pins.h"

/* Parameters for inputs */
static os_ushort pins_inputs_gazerbeam_prm[]= {PIN_RV, PIN_RV, PIN_INTERRUPT_ENABLED, 1};
PINS_INTCONF_STRUCT(pin_gazerbeam_intconf)

/* Parameters for outputs */
static os_ushort pins_outputs_led_builtin_prm[]= {PIN_RV, PIN_RV};

/* Parameters for uart */
static os_ushort pins_uart_uart2_prm[]= {PIN_RV, PIN_RV, PIN_SPEED, 96, PIN_TX, 17, PIN_RX, 16};

/* TITO IO configuration structure */
OS_FLASH_MEM pins_t pins =
{
  {{1, &pins.inputs.gazerbeam}, /* inputs */
    {PIN_INPUT, 0, 39, pins_inputs_gazerbeam_prm, sizeof(pins_inputs_gazerbeam_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_PTR(pin_gazerbeam_intconf)} /* gazerbeam */
  },

  {{1, &pins.outputs.led_builtin}, /* outputs */
    {PIN_OUTPUT, 0, 33, pins_outputs_led_builtin_prm, sizeof(pins_outputs_led_builtin_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* led_builtin */
  },

  {{1, &pins.uart.uart2}, /* uart */
    {PIN_UART, 0, 2, pins_uart_uart2_prm, sizeof(pins_uart_uart2_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* uart2 */
  },
};

/* List of pin type groups */
static OS_FLASH_MEM PinGroupHdr * OS_FLASH_MEM pins_group_list[] =
{
  &pins.inputs.hdr,
  &pins.outputs.hdr,
  &pins.uart.hdr
};

/* TITO IO configuration top header structure */
OS_FLASH_MEM IoPinsHdr pins_hdr = {pins_group_list, sizeof(pins_group_list)/sizeof(PinGroupHdr*)};
