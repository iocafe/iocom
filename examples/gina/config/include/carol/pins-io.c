/* This file is gerated by pins-to-c.py script, do not modify. */
#include "pins.h"

/* Parameters for inputs */
static os_ushort pins_inputs_gazerbeam_prm[]= {PIN_RV, PIN_RV, PIN_INTERRUPT_ENABLED, 1};
PINS_INTCONF_STRUCT(pin_gazerbeam_intconf)
static os_ushort pins_inputs_in_x_prm[]= {PIN_RV, PIN_RV};

/* Parameters for outputs */
static os_ushort pins_outputs_led_builtin_prm[]= {PIN_RV, PIN_RV};
static os_ushort pins_outputs_tft_light_prm[]= {PIN_RV, PIN_RV};

/* Parameters for analog_inputs */
static os_ushort pins_analog_inputs_ccd_signal_prm[]= {PIN_RV, PIN_RV, PIN_MAX, 4095};
static os_ushort pins_analog_inputs_potentiometer_prm[]= {PIN_RV, PIN_RV, PIN_SPEED, 3, PIN_MAX, 4095};

/* Parameters for pwm */
static os_ushort pins_pwm_ccd_clock_prm[]= {PIN_RV, PIN_RV, PIN_RESOLUTION, 1, PIN_HPOINT, 0, PIN_FREQENCY_KHZ, 800, PIN_INIT, 1, PIN_TIMER_SELECT, 0};
static os_ushort pins_pwm_dimmer_led_prm[]= {PIN_RV, PIN_RV, PIN_RESOLUTION, 12, PIN_FREQENCY, 5000, PIN_MAX, 4095, PIN_INIT, 0};

/* Parameters for spi */
static os_ushort pins_spi_tft_spi_prm[]= {PIN_RV, PIN_RV, PIN_MOSI, 23, PIN_SCLK, 18, PIN_DC, 2, PIN_MISO, 19};

/* Parameters for timers */
static os_ushort pins_timers_ccd_data_prm[]= {PIN_RV, PIN_RV, PIN_FREQENCY_KHZ, 100, PIN_TIMER_SELECT, 0, PIN_TIMER_GROUP_SELECT, 0, PIN_INTERRUPT_ENABLED, 1};
PINS_INTCONF_STRUCT(pin_ccd_data_intconf)

/* Parameters for cameras */
static os_ushort pins_cameras_ccd_prm[]= {PIN_RV, PIN_RV, PIN_B_BANK, 1, PIN_TIMER_SELECT, 1, PIN_C_BANK, 2, PIN_A, 36, PIN_D, 35, PIN_C, 32, PIN_B, 21};

/* Parameters for uart */
static os_ushort pins_uart_uart2_prm[]= {PIN_RV, PIN_RV, PIN_TX, 17, PIN_SPEED, 96, PIN_RX, 16};

/* GINA IO configuration structure */
OS_FLASH_MEM pins_t pins =
{
  {{2, &pins.inputs.gazerbeam}, /* inputs */
    {PIN_INPUT, 0, 39, pins_inputs_gazerbeam_prm, sizeof(pins_inputs_gazerbeam_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_PTR(pin_gazerbeam_intconf)}, /* gazerbeam */
    {PIN_INPUT, 0, 34, pins_inputs_in_x_prm, sizeof(pins_inputs_in_x_prm)/sizeof(os_ushort), OS_NULL, &gina.exp.in_x PINS_INTCONF_NULL} /* in_x */
  },

  {{2, &pins.outputs.led_builtin}, /* outputs */
    {PIN_OUTPUT, 0, 33, pins_outputs_led_builtin_prm, sizeof(pins_outputs_led_builtin_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL}, /* led_builtin */
    {PIN_OUTPUT, 0, 5, pins_outputs_tft_light_prm, sizeof(pins_outputs_tft_light_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* tft_light */
  },

  {{2, &pins.analog_inputs.ccd_signal}, /* analog_inputs */
    {PIN_ANALOG_INPUT, 0, 36, pins_analog_inputs_ccd_signal_prm, sizeof(pins_analog_inputs_ccd_signal_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL}, /* ccd_signal */
    {PIN_ANALOG_INPUT, 0, 26, pins_analog_inputs_potentiometer_prm, sizeof(pins_analog_inputs_potentiometer_prm)/sizeof(os_ushort), OS_NULL, &gina.exp.potentiometer PINS_INTCONF_NULL} /* potentiometer */
  },

  {{2, &pins.pwm.ccd_clock}, /* pwm */
    {PIN_PWM, 0, 22, pins_pwm_ccd_clock_prm, sizeof(pins_pwm_ccd_clock_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL}, /* ccd_clock */
    {PIN_PWM, 1, 27, pins_pwm_dimmer_led_prm, sizeof(pins_pwm_dimmer_led_prm)/sizeof(os_ushort), OS_NULL, &gina.imp.dimmer_led PINS_INTCONF_NULL} /* dimmer_led */
  },

  {{1, &pins.spi.tft_spi}, /* spi */
    {PIN_SPI, 0, 0, pins_spi_tft_spi_prm, sizeof(pins_spi_tft_spi_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* tft_spi */
  },

  {{1, &pins.timers.ccd_data}, /* timers */
    {PIN_TIMER, 0, 0, pins_timers_ccd_data_prm, sizeof(pins_timers_ccd_data_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_PTR(pin_ccd_data_intconf)} /* ccd_data */
  },

  {{1, &pins.cameras.ccd}, /* cameras */
    {PIN_CAMERA, 0, 0, pins_cameras_ccd_prm, sizeof(pins_cameras_ccd_prm)/sizeof(os_ushort), OS_NULL, OS_NULL PINS_INTCONF_NULL} /* ccd */
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
  &pins.analog_inputs.hdr,
  &pins.pwm.hdr,
  &pins.spi.hdr,
  &pins.timers.hdr,
  &pins.cameras.hdr,
  &pins.uart.hdr
};

/* GINA IO configuration top header structure */
OS_FLASH_MEM IoPinsHdr pins_hdr = {pins_group_list, sizeof(pins_group_list)/sizeof(PinGroupHdr*)};
