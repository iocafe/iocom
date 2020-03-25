#include "gina.h"
#include "driver/gpio.h"

#define GPIO_OUTPUT_IO_0    18
#define GPIO_OUTPUT_PIN_SEL  (1ULL << GPIO_OUTPUT_IO_0)


// CONFIG_ARDUINO_RUNNING_CORE
// CONFIG_AUTOSTART_ARDUINO
// Notice skconfig.h in same folder as main.cpp #include "sdkconfig.h"


void ullethread(
    void *prm,
    osalEvent done)
{
    gpio_config_t io_conf;
    os_int cnt = 0;
    os_timer tnow, tlimit, period = 5;

    osal_event_set(done);

    os_memclear(&io_conf, sizeof(io_conf));
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;

    gpio_config(&io_conf);


    tlimit = esp_timer_get_time() + 1;
    while (1)
    {
        do {
            tnow = esp_timer_get_time();
        } while (tnow < tlimit);
        tlimit = tnow + period;

        gpio_set_level(GPIO_OUTPUT_IO_0, ++cnt % 2);
    }
}


void ulletest(void)
{
    osalThreadOptParams opt;
    osal_debug_error("HERE XX");

    os_memclear(&opt, sizeof(opt));
    opt.pin_to_core = OS_TRUE;
    opt.pin_to_core_nr = 1;
    opt.priority = OSAL_THREAD_PRIORITY_TIME_CRITICAL;

    // osal_thread_create(ullethread, OS_NULL, &opt, OSAL_THREAD_DETACHED);
}




#if 0
/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


/* Blink Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "sdkconfig.h"
#include <Arduino.h>

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO (gpio_num_t)CONFIG_BLINK_GPIO

#ifndef LED_BUILTIN
#define LED_BUILTIN 4
#endif

void blink_task(void *pvParameter)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

#if !CONFIG_AUTOSTART_ARDUINO
void arduinoTask(void *pvParameter) {
    pinMode(LED_BUILTIN, OUTPUT);
    while(1) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(1000);
    }
}

void app_main()
{
    // initialize arduino library before we start the tasks
    initArduino();

    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(&arduinoTask, "arduino_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}
#else
void setup() {
    Serial.begin(115200);
    xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.println("Hello!");
    delay(1000);
}
#endif 

*/

#endif