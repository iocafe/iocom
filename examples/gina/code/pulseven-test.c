/*
 * ESP32: LEDC peripheral can be used to generate PWM signals between 40 MHz (half of APB clock) and approximately 0.001 Hz.
Please check the LEDC chapter in Technical Reference Manual.
*/

#include <stdio.h>
#include "esp_err.h"
#include "driver/ledc.h"
#include "driver/periph_ctrl.h"


void ulletest(void)
{
    periph_module_enable(PERIPH_LEDC_MODULE);

//    int bit_width = 1; // 1 - 20 bits
//    int bit_width = 20; // 1 - 20 bits
//    int divider = 256;  // Q10.8 fixed point number, 0x100 — 0x3FFFF
//    int duty_cycle = 1 << (bit_width - 1);

//    float freq_hz = ((uint64_t) LEDC_REF_CLK_HZ << 8) / (float) divider / (1 << bit_width);
//    printf("frequency: %f Hz\n", freq_hz);

    /* Prepare and set configuration of timers
    * that will be used by LED Controller
    */
   ledc_timer_config_t ledc_timer = {
       .duty_resolution = LEDC_TIMER_1_BIT, // resolution of PWM duty
       .freq_hz = 1000000,                      // frequency of PWM signal
       .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
       .timer_num = LEDC_TIMER_0,            // timer index
 //      .clk_cfg = LEDC_USE_APB_CLK              // Auto select the source clock XXXXXXXXX NEEDS TO BE ADDED LEDC_USE_REF_TICK?
   };
   // Set configuration of timer0 for high speed channels
   ledc_timer_config(&ledc_timer);

   // Prepare and set configuration of timer1 for low speed channels
   /* ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
   ledc_timer.timer_num = LEDC_TIMER_0;
   ledc_timer_config(&ledc_timer); */



   //  ledc_timer_set(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, divider, bit_width, LEDC_REF_TICK);
    // ledc_timer_rst(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0);
    // ledc_timer_resume(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0);

    /* ledc_channel_config_t channel_config = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = duty_cycle,
        .gpio_num   = GPIO_NUM_2,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0
    }; */
    ledc_channel_config_t channel_config = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 1,
        .gpio_num   = 2, // GPIO_NUM_2,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0
    };

    ledc_channel_config(&channel_config);


#define LEDC_TEST_DUTY         2
//    ledc_set_duty(channel_config.speed_mode, channel_config.channel, LEDC_TEST_DUTY);

    return;

}

