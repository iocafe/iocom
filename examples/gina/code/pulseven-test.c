/*
 * Example: Generate 1 MHz clock signal with ESP32
   note 24.3.2020/pekka
   ESP32: LEDC peripheral can be used to generate clock signals between 40 MHz (half of APB clock) and approximately 0.001 Hz.
   Please check the LEDC chapter in Technical Reference Manual. Example code below.
*/
#include "driver/ledc.h"
#include "driver/periph_ctrl.h"


void set_1MHz_clock_on_GPIO2(void)
{
    periph_module_enable(PERIPH_LEDC_MODULE);

    // Set up timer
    ledc_timer_config_t ledc_timer = {
       .duty_resolution = LEDC_TIMER_1_BIT,     // We need clock, not PWM so 1 bit is enough.
       .freq_hz = 1000000,                      // Clock frequency, 1 MHz
       .speed_mode = LEDC_HIGH_SPEED_MODE,
       .timer_num = LEDC_TIMER_0,
        // .clk_cfg = LEDC_USE_APB_CLK          // I think this is needed for neweer espressif software, if problem, try uncommenting this line
    };
    ledc_timer_config(&ledc_timer);

    // Set up GPIO PIN
    ledc_channel_config_t channel_config = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 1,
        .gpio_num   = 2,                        // GPIO pin
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0
    };
    ledc_channel_config(&channel_config);
}

