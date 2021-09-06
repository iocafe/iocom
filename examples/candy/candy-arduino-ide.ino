#include <Arduino.h>
#include <FreeRTOS.h>
#include <eosalx.h>

/*
  candy-arduino-ide.ino
  Entry point when building with Arduino IDE.
 */


/* The setup routine runs once when you press reset.
 */
void setup()
{
   /* Initialize the eosal library.
    */
    osal_initialize(OSAL_INIT_DEFAULT);
    osal_main(0, 0);
}

/* The loop routine runs over and over again forever.
 */
void loop()
{
    /* Forward loop call to osal_loop(). Reboot if osal_loop returns "no success".
     */
    if (osal_loop(osal_application_context)) osal_reboot(0);

#ifdef ESP_PLATFORM
    /* ESP32: Save some resources for other tasks.
     */
    osal_sleep(20);
#endif
}
