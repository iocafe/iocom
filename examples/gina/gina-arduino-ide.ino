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
    /* ESP-IDF 3.X/MELIFE test board : We cannot run too fast, WiFi will lock up?
     */
    osal_sleep(10);
#endif
}

/* Include test code
 */
// #include "/coderoot/iocom/examples/ioboard_test/code/ioboard_example.c"
