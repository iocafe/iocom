/* This file is gerated by pins_to_c.py script, do not modify. */
OSAL_C_HEADER_BEGINS

/* TITO IO configuration structure */
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
    Pin uart2;
  }
  uart;
}
pins_t;

/* TITO IO configuration top header structure */
extern OS_FLASH_MEM_H IoPinsHdr pins_hdr;

/* Global TITO IO configuration structure */
extern OS_FLASH_MEM_H pins_t pins;

/* Name defines for pins and application pin groups (use ifdef to check if HW has pin) */
#define PINS_INPUTS_GAZERBEAM "gazerbeam"
#define PINS_OUTPUTS_LED_BUILTIN "led_builtin"
#define PINS_UART_UART2 "uart2"

OSAL_C_HEADER_ENDS
