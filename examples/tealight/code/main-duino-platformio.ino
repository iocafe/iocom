#include <Arduino.h>
#include <eosalx.h>
#include <FreeRTOS.h>

/*
  main-duino-platformio.ino
  Entry point when building with Visual Studio Code, PlatformIO and Arduino Framework.
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
    if (osal_loop(osal_application_context)) {
        osal_reboot(0);
    }

    /* ESP32: Save some resources for other tasks.
     */
#ifdef ESP_PLATFORM
    osal_sleep(3);
#endif
}
