/* This file is gerated by pins-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

/* GINA IO configuration structure */
typedef struct
{
  struct
  {
    PinGroupHdr hdr;
    Pin gazerbeam;
    Pin in_x;
  }
  inputs;

  struct
  {
    PinGroupHdr hdr;
    Pin led_builtin;
    Pin tft_light;
  }
  outputs;

  struct
  {
    PinGroupHdr hdr;
    Pin ccd_signal;
    Pin potentiometer;
  }
  analog_inputs;

  struct
  {
    PinGroupHdr hdr;
    Pin ccd_clock;
    Pin dimmer_led;
  }
  pwm;

  struct
  {
    PinGroupHdr hdr;
    Pin tft_spi;
  }
  spi;

  struct
  {
    PinGroupHdr hdr;
    Pin ccd_data;
  }
  timers;

  struct
  {
    PinGroupHdr hdr;
    Pin ccd;
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

/* GINA IO configuration top header structure */
extern OS_FLASH_MEM_H IoPinsHdr pins_hdr;

/* Global GINA IO configuration structure */
extern OS_FLASH_MEM_H pins_t pins;

/* Name defines for pins and application pin groups (use ifdef to check if HW has pin) */
#define PINS_INPUTS_GAZERBEAM "gazerbeam"
#define PINS_INPUTS_IN_X "in_x"
#define PINS_OUTPUTS_LED_BUILTIN "led_builtin"
#define PINS_OUTPUTS_TFT_LIGHT "tft_light"
#define PINS_ANALOG_INPUTS_CCD_SIGNAL "ccd_signal"
#define PINS_ANALOG_INPUTS_POTENTIOMETER "potentiometer"
#define PINS_PWM_CCD_CLOCK "ccd_clock"
#define PINS_PWM_DIMMER_LED "dimmer_led"
#define PINS_SPI_TFT_SPI "tft_spi"
#define PINS_TIMERS_CCD_DATA "ccd_data"
#define PINS_CAMERAS_CCD "ccd"
#define PINS_UART_UART2 "uart2"

OSAL_C_HEADER_ENDS
