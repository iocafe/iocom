#include <Arduino.h>
#include <FreeRTOS.h>
#include <eosalx.h>

/*
  4_ioboard_test.ino
  Include a IO board test app to build it within Arduino IDE.
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
    /* ESP-IDF 3.X/MELIFE test board : We cannot write too fast through WiFi, WiFi will lock up.
     */
    os_sleep(20);
#endif
}

/* Include code for the application
 */
#include "/coderoot/iocom/examples/ioboard_test/code/ioboard_example.c"
