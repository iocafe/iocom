#if 0
// Credit Ivan Voras

#include "gina.h"
#include "driver/gpio.h"

#include <soc/sens_reg.h>
#include <soc/sens_struct.h>

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



/* Local version of reading analog output, adopted from ESP OS (by Ivan Voras)
Digging through IDF Source Code
Sampling data from an ADC is usually a simple task, so the next strategy is to see how the IDF does it, and replicate it in our code directly, without calling the provided API. The adc1_get_raw() function is implemented in the rtc_module.c file of the IDF, and of the eight or so things it does, only one is actually sampling the ADC, which is done by a call to adc_convert(). Luckily, adc_convert() is a simple function which samples the ADC by manipulating peripheral hardware registers via a global structure named SENS.

Adapting this code so it works in our program (and to mimic the behavior of adc1_get_raw()) is easy. It looks like this:

Finally, since adc1_get_raw() performs some configuration steps before sampling the ADC, it should be called directly, just after the ADC is set up. That way the relevant configuration can be performed before the timer is started.

The downside of this approach is that it doesn’t play nice with other IDF functions. As soon as some other peripheral, driver, or a random piece of code is called which resets the ADC configuration, our custom function will no longer work correctly. At least WiFi, PWM, I2C, and SPI do not influence the ADC configuration. In case something does influence it, a call to adc1_get_raw() will configure ADC appropriately again.

 */
int IRAM_ATTR local_adc1_read(int channel) {
    uint16_t adc_value;
    SENS.sar_meas_start1.sar1_en_pad = (1 << channel); // only one channel is selected
    while (SENS.sar_slave_addr1.meas_status != 0);
    SENS.sar_meas_start1.meas1_start_sar = 0;
    SENS.sar_meas_start1.meas1_start_sar = 1;
    while (SENS.sar_meas_start1.meas1_done_sar == 0);
    adc_value = SENS.sar_meas_start1.meas1_data_sar;
    return adc_value;
}


/* Interrupt handler, kicks high priority task to run.
 */
#define ADC_SAMPLES_COUNT 1000
int16_t abuf[ADC_SAMPLES_COUNT];
int16_t abufPos = 0;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);

  abuf[abufPos++] = local_adc1_read(ADC1_CHANNEL_0);

  if (abufPos >= ADC_SAMPLES_COUNT) {
    abufPos = 0;

    // Notify adcTask that the buffer is full.
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(adcTaskHandle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}



portMUX_TYPE DRAM_ATTR timerMux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t complexHandlerTask;
hw_timer_t * adcTimer = NULL; // our timer

void complexHandler(void *param) {
  while (true) {
    // Sleep until the ISR gives us something to do, or for 1 second
    uint32_t tcount = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(1000));
    if (check_for_work) {
      // Do something complex and CPU-intensive
    }
  }
}

void IRAM_ATTR onTimer() {
  // A mutex protects the handler from reentry (which shouldn't happen, but just in case)
  portENTER_CRITICAL_ISR(&timerMux);

  // Do something, e.g. read a pin.

  if (some_condition) {
    // Notify complexHandlerTask that the buffer is full.
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(complexHandlerTask, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  xTaskCreate(complexHandler, "Handler Task", 8192, NULL, 1, &complexHandlerTask);
  adcTimer = timerBegin(3, 80, true); // 80 MHz / 80 = 1 MHz hardware clock for easy figuring
  timerAttachInterrupt(adcTimer, &onTimer, true); // Attaches the handler function to the timer
  timerAlarmWrite(adcTimer, 45, true); // Interrupts when counter == 45, i.e. 22.222 times a second
  timerAlarmEnable(adcTimer);
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



#endif
